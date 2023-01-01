namespace Hub.AI;

using System;
using Microsoft.Data.Analysis;
using Hub.Data;
using Microsoft.ML.Data;
using Microsoft.ML;
using Microsoft.ML.Trainers.FastTree;

public class ThermostatAI
{
    static readonly TimeSpan SetpointDuration = TimeSpan.FromHours(2.0);

    const AITrainingAlgorithm DefaultAlgorithm = AITrainingAlgorithm.FastForest;

    /// <summary>
    /// Gets the setpoint for the thermostat in Celsius.
    /// </summary>
    public async Task<double> GetSetpointAsync(HomeDatabase db, DeviceId id)
    {
        // First, check if the house is unoccupied.
        if (await db.GetOccupancyAsync() is OccupancyEvent occupancy &&
            !occupancy.Occupied)
        {
            // If the house is unoccupied, use the default setpoint.
            return occupancy.UnoccupiedCelsius;
        }
        var now = DateTime.UtcNow;
        // Next, check if the user has set a setpoint for this device.
        if (await db.GetThermostatUserSetpointAsync(id) is DataLogEvent userSetpoint &&
            userSetpoint.Timestamp > now - SetpointDuration)
        {
            // If the user set a setpoint, use that.
            return userSetpoint.Value;
        }
        var prediction = await PredictAsync(db, id);
        // Console.WriteLine($"Predicted temperature: {prediction.TargetCelsius}");
        return prediction.TargetCelsius;
    }

    async Task<ThermostatPrediction> PredictAsync(HomeDatabase db, DeviceId id)
    {
        var predictions = await PredictManyAsync(new[] { DateTime.UtcNow }, DefaultAlgorithm, db, id);
        return predictions[0];
    }

    public async Task<ThermostatPrediction[]> PredictManyAsync(DateTime[] times, AITrainingAlgorithm algorithm, HomeDatabase db, DeviceId id)
    {
        // Console.WriteLine("Training model...");

        MLContext mlContext = new MLContext(seed: 0);

        var states = await DeviceStatesOverTime.LoadAsync(id, 60 * 24 * 365, includeAwayData: false, db: db);

        // Console.WriteLine($"Training on {states.States.Length} data points.");

        // Load the training data
        var dataQ =
            from s in states.States
            select new ThermostatData (s.Timestamp, (float)s.AISetTemperature);
        var data = dataQ.ToArray();
        if (data.Length == 0)
        {
            return new[] { new ThermostatPrediction { TargetCelsius = (float)67.0.FahrenheitToCelsius() } };
        }
        else if (data.Length == 1)
        {
            return new[] { new ThermostatPrediction { TargetCelsius = data[0].TargetCelsius } };
        }
        // foreach (var d in data) {
        //     d.Dump();
        // }

        // Train
        var columns = new[] {
            DataFrameColumn.Create("CosDayOfWeek", data.Select(d => d.CosDayOfWeek)),
            DataFrameColumn.Create("SinDayOfWeek", data.Select(d => d.SinDayOfWeek)),
            DataFrameColumn.Create("CosDayOfYear", data.Select(d => d.CosDayOfYear)),
            DataFrameColumn.Create("SinDayOfYear", data.Select(d => d.SinDayOfYear)),
            DataFrameColumn.Create("CosHour", data.Select(d => d.CosHour)),
            DataFrameColumn.Create("SinHour", data.Select(d => d.SinHour)),
            DataFrameColumn.Create("TargetCelsius", data.Select(d => d.TargetCelsius)),
        };
        var dataView = new DataFrame(columns);
        
        var pipelineHead =
            mlContext.Transforms.CopyColumns(outputColumnName: "Label", inputColumnName:"TargetCelsius")
            // .Append(mlContext.Transforms.Categorical.OneHotEncoding(outputColumnName: "DayOfWeekEncoded", inputColumnName:"DayOfWeek"))
            .Append(mlContext.Transforms.Concatenate("Features",
                "CosDayOfWeek", "SinDayOfWeek",
                "CosDayOfYear", "SinDayOfYear",
                "CosHour", "SinHour"));
        ITransformer model = algorithm switch
        {
            AITrainingAlgorithm.LbfgsPoisson =>
                pipelineHead.Append (mlContext.Regression.Trainers.LbfgsPoissonRegression()).Fit(dataView),
            AITrainingAlgorithm.Gam =>
                pipelineHead.Append (mlContext.Regression.Trainers.Gam()).Fit(dataView),
            AITrainingAlgorithm.Sdca =>
                pipelineHead.Append (mlContext.Regression.Trainers.Sdca()).Fit(dataView),
            AITrainingAlgorithm.FastTreeTweedie =>
                pipelineHead.Append (mlContext.Regression.Trainers.FastTreeTweedie()).Fit(dataView),
            AITrainingAlgorithm.FastTree =>
                pipelineHead.Append (mlContext.Regression.Trainers.FastTree()).Fit(dataView),
            AITrainingAlgorithm.FastForest =>
                pipelineHead.Append (mlContext.Regression.Trainers.FastForest()).Fit(dataView),
            AITrainingAlgorithm.OnlineGradientDescent =>
                pipelineHead.Append (mlContext.Regression.Trainers.OnlineGradientDescent()).Fit(dataView),
            _ =>
                throw new NotSupportedException(algorithm.ToString())
        };

        // Predict
        var predictionFunction = mlContext.Model.CreatePredictionEngine<ThermostatData, ThermostatPrediction>(model);
        var predictions = new ThermostatPrediction[times.Length];
        for (int i = 0; i < times.Length; i++)
        {
            var sample = new ThermostatData(times[i], 0);
            var prediction = predictionFunction.Predict(sample);
            predictions[i] = prediction;
        }
        return predictions;
    }
}

public enum AITrainingAlgorithm
{
    LbfgsPoisson,
    Gam,
    Sdca,
    FastTreeTweedie,
    FastTree,
    FastForest,
    OnlineGradientDescent,
}

public class ThermostatData
{
    [LoadColumn(0)]
    public float CosDayOfWeek;

    [LoadColumn(1)]
    public float SinDayOfWeek;

    [LoadColumn(2)]
    public float CosDayOfYear;

    [LoadColumn(3)]
    public float SinDayOfYear;

    [LoadColumn(4)]
    public float CosHour;

    [LoadColumn(5)]
    public float SinHour;

    [LoadColumn(6)]
    public float TargetCelsius;

    public ThermostatData()
    {
    }

    public ThermostatData(DateTime timestamp, float targetCelsius)
    {
        var dayOfWeek = (int)timestamp.DayOfWeek;
        var dayOfYear = timestamp.DayOfYear;
        var hour = timestamp.Hour;
        CosDayOfWeek = MathF.Cos(dayOfWeek * 2 * MathF.PI / 7);
        SinDayOfWeek = MathF.Sin(dayOfWeek * 2 * MathF.PI / 7);
        CosDayOfYear = MathF.Cos(dayOfYear * 2 * MathF.PI / 365);
        SinDayOfYear = MathF.Sin(dayOfYear * 2 * MathF.PI / 365);
        CosHour = MathF.Cos(hour * 2 * MathF.PI / 24);
        SinHour = MathF.Sin(hour * 2 * MathF.PI / 24);
        TargetCelsius = targetCelsius;
    }

    public void Dump() {
        var d = this;
        Console.WriteLine($"{d.CosDayOfWeek} {d.SinDayOfWeek} {d.CosDayOfYear} {d.SinDayOfYear} {d.CosHour} {d.SinHour} {d.TargetCelsius}");
    }
}

public class ThermostatPrediction
{
    [ColumnName("Score")]
    public float TargetCelsius;
}


namespace Hub.AI;

using System;
using Microsoft.Data.Analysis;
using Hub.Data;
using Microsoft.ML.Data;
using Microsoft.ML;
using Microsoft.ML.Trainers.FastTree;

public class ThermostatAI
{
    public const double DefaultTemperature = 23.0;

    static readonly TimeSpan SetpointDuration = TimeSpan.FromHours(1.0);

    /// <summary>
    /// Gets the setpoint for the thermostat in Celsius.
    /// </summary>
    public async Task<double> GetSetpointAsync(HomeDatabase db, DeviceId id)
    {
        var now = DateTime.UtcNow;
        // First, check if the user has set a setpoint for this device.
        if (await db.GetThermostatUserSetpointAsync(id) is DataLogEvent userSetpoint &&
            userSetpoint.Timestamp > now - SetpointDuration)
        {
            // If the user set a setpoint, use that.
            return userSetpoint.Value;
        }
        var prediction = await PredictAsync(db, id);
        Console.WriteLine($"Predicted temperature: {prediction.TargetCelsius}");
        return prediction.TargetCelsius;
    }

    async Task<ThermostatPrediction> PredictAsync(HomeDatabase db, DeviceId id)
    {
        // Console.WriteLine("Training model...");

        MLContext mlContext = new MLContext(seed: 0);

        var states = await DeviceStatesOverTime.LoadAsync(id, 60 * 24 * 365, db);

        // Console.WriteLine($"Training on {states.States.Length} data points.");

        var dataQ =
            from s in states.States
            select new ThermostatData (s.Timestamp, (float)s.AISetTemperature);
        var data = dataQ.ToArray();
        // foreach (var d in data) {
        //     d.Dump();
        // }
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

        var pipeline =
            mlContext.Transforms.CopyColumns(outputColumnName: "Label", inputColumnName:"TargetCelsius")
            // .Append(mlContext.Transforms.Categorical.OneHotEncoding(outputColumnName: "DayOfWeekEncoded", inputColumnName:"DayOfWeek"))
            .Append(mlContext.Transforms.Concatenate("Features",
                "CosDayOfWeek", "SinDayOfWeek",
                "CosDayOfYear", "SinDayOfYear",
                "CosHour", "SinHour"))
            .Append(mlContext.Regression.Trainers.LbfgsPoissonRegression());
        var model = pipeline.Fit(dataView);

        var predictionFunction = mlContext.Model.CreatePredictionEngine<ThermostatData, ThermostatPrediction>(model);

        var sample = new ThermostatData(DateTime.UtcNow, 0);
        // sample = data[0];
        // Console.WriteLine("Sample:");
        // sample.Dump();
        var prediction = predictionFunction.Predict(sample);

        return prediction;
    }
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


global using DeviceId = System.Guid;

using SQLite;

namespace Hub.Data;

public class Device
{
    public static readonly TimeSpan OnlineTimeout = TimeSpan.FromHours(1);

    [PrimaryKey]
    public DeviceId Id { get; set; }
    public string Name { get; set; } = "";
}

public class DeviceState
{
    public DateTime Timestamp { get; set; }
    public double ThermostatReading { get; set; }
    public double AISetTemperature { get; set; }
    // public double UserSetTemperature { get; set; }
    public bool HeaterOn { get; set; }

    public bool HasAllData => ThermostatReading > 0.1 && AISetTemperature > 0.1;// && UserSetTemperature > 0.1;
}


public class DeviceStatesOverTime
{
    public DeviceId Id { get; private set; }
    public DeviceState[] States { get; private set; } = Array.Empty<DeviceState>();
    public DateTime Start => States.Length > 0 ? States[0].Timestamp : DateTime.MinValue;
    public DateTime End => States.Length > 0 ? States[^1].Timestamp : DateTime.MinValue;

    public DeviceStatesOverTime(DeviceId id, DeviceState[] states)
    {
        Id = id;
        States = states.Where(s => s.HasAllData).ToArray();
    }

    public static async Task<DeviceStatesOverTime> LoadAsync(DeviceId id, int minutes, bool includeAwayData, HomeDatabase db)
    {
        var end = DateTime.UtcNow;
        var start = end - TimeSpan.FromMinutes(minutes);
        var events = await db.GetDeviceEventsAsync(id, start, end).ConfigureAwait(false);
        var occupancyEvents = await db.GetOccupancyEventsAsync(start, end).ConfigureAwait(false);
        var houseOccupied = true;
        var heaterOn = false;
        var temperature = 0.0;
        var userSetpoint = 0.0;
        var aiSetpoint = 0.0;
        var states = new List<DeviceState>();
        var allEvents =
            events
            .Cast<ILoggedEvent>()
            .Concat(occupancyEvents)
            .OrderBy(e => e.Timestamp)
            .ToArray();
        foreach (var loggedEvent in allEvents)
        {
            if (loggedEvent is OccupancyEvent occupancy)
            {
                houseOccupied = occupancy.Occupied;
            }
            else if (loggedEvent is DataLogEvent e) {
                if (e.EventType == LogEventType.ThermostatReading)
                {
                    temperature = e.Value;
                }
                else if (e.EventType == LogEventType.HeaterOn)
                {
                    heaterOn = e.Value > 0.5f;
                }
                else if (e.EventType == LogEventType.UserTemperatureSetting)
                {
                    userSetpoint = e.Value;
                }
                else if (e.EventType == LogEventType.AITemperatureSetting)
                {
                    aiSetpoint = e.Value;
                }
                var shouldAdd = (e.EventType == LogEventType.AITemperatureSetting) &&
                    (houseOccupied || includeAwayData);
                if (shouldAdd) {
                    states.Add(new DeviceState
                    {
                        Timestamp = e.Timestamp,
                        ThermostatReading = temperature,
                        AISetTemperature = aiSetpoint,
                        // UserSetTemperature = userSetpoint,
                        HeaterOn = heaterOn
                    });
                }
            }
        }
        return new DeviceStatesOverTime(id, states.ToArray());
    }
}

public class DeviceInfo {
    public Guid Id { get; set; } = Guid.Empty;
    public string Name { get; set; } = "";
    public DeviceStatus Status { get; set; } = DeviceStatus.Unknown;
    public double CurrentTemperature { get; set; } = 0;
    public double SetTemperature { get; set; } = 0;
    public double LastRequestTemperature { get; set; } = 0;
    public DateTime LastRequestTemperatureTimestamp { get; set; }
    public (double X, double Y)[] TempReadingsF { get; set; } = Array.Empty<(double X, double Y)>();
    public (double X, double Y)[] AISetTemperaturesF { get; set; } = Array.Empty<(double X, double Y)>();
    public (double X, double Y)[] UserTempReadingsF { get; set; } = Array.Empty<(double X, double Y)>();
    public DeviceState[] States { get; set; } = Array.Empty<DeviceState>();
    public DeviceInfo(Guid deviceId) {
        Id = deviceId;
    }
    public static async Task<DeviceInfo> LoadAsync(Guid deviceId, int displayMinutes, HomeDatabase db) {
        var lastSetTemp = await db.GetThermostatUserSetpointAsync(deviceId);
        var now = DateTime.UtcNow;
        var states = (await DeviceStatesOverTime.LoadAsync(deviceId, displayMinutes, includeAwayData: true, db: db)).States.Where(s => s.HasAllData).ToArray();
        var minTime = states.Length > 0 ? states[0].Timestamp : now;
        return new DeviceInfo(deviceId) {
            Status = await db.GetOnlineDisplayStatusAsync(deviceId, db),
            CurrentTemperature = states.Length > 0 ? states[^1].ThermostatReading : 0.0,
            SetTemperature = await (new Hub.AI.ThermostatAI().GetSetpointAsync(db, deviceId)),
            LastRequestTemperature = lastSetTemp?.Value ?? 0.0,
            LastRequestTemperatureTimestamp = lastSetTemp?.Timestamp ?? DateTime.MinValue,
            TempReadingsF = states.Select(s => (
                (s.Timestamp - minTime).TotalMinutes,
                s.ThermostatReading.CelsiusToFahrenheit()
            )).ToArray(),
            AISetTemperaturesF = states.Select(s => (
                (s.Timestamp - minTime).TotalMinutes,
                s.AISetTemperature.CelsiusToFahrenheit()
            )).ToArray(),
            States = states,
        };
    }
}

public enum DeviceStatus {
    Unknown,
    Online,
    Offline,
}


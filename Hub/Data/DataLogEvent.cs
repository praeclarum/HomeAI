using SQLite;

namespace Hub.Data;

public class DataLogEvent
{
    [PrimaryKey, AutoIncrement]
    public int Id { get; set; }
    /// <summary>
    /// The time that the event occurred in UTC.
    /// </summary>
    public DateTime Timestamp { get; set; }
    [Indexed]
    public DeviceId DeviceId { get; set; }
    public LogEventType EventType { get; set; }
    public double Value { get; set; }
}




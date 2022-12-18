global using DeviceId = System.Guid;

namespace HomeAI;

public class LogEventRequest
{
    /// <summary>
    /// The time that the event occurred in UTC.
    /// </summary>
    public DateTime Timestamp { get; set; }
    public DeviceId DeviceId { get; set; }
    public LogEventType EventType { get; set; }
    public double Value { get; set; }
}

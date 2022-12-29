using SQLite;

namespace Hub.Data;

public class OccupancyEvent
{
    [PrimaryKey, AutoIncrement]
    public int Id { get; set; }
    /// <summary>
    /// The time that the event occurred in UTC.
    /// </summary>
    [Indexed]
    public DateTime Timestamp { get; set; }
    public OccupancyEventType EventType { get; set; }
    public bool Occupied { get; set; }
    public double UnoccupiedCelsius { get; set; }
}

public enum OccupancyEventType {
    Manual,
    Automatic,
}


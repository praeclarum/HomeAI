global using DeviceId = System.Guid;

using SQLite;

namespace Hub.Data;

public class Device
{
    static readonly TimeSpan onlineTimeout = TimeSpan.FromMinutes(5);

    [PrimaryKey]
    public DeviceId Id { get; set; }
    public string Name { get; set; } = "";

    public async Task<string> GetOnlineDisplayStatusAsync(HomeDatabase db)
    {
        var lastEvent = await db.GetLastEventAsync(Id);
        if (lastEvent is null)
        {
            return "Not seen";
        }
        else
        {
            var timeSinceLastEvent = DateTime.UtcNow - lastEvent.Timestamp;
            if (timeSinceLastEvent < onlineTimeout)
            {
                return "Online";
            }
            else
            {
                return "Offline";
            }
        }
    }
}

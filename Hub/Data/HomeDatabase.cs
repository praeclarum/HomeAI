using SQLite;

namespace Hub.Data;

public class HomeDatabase
{
    private readonly SQLiteAsyncConnection _database;

    static readonly Lazy<SQLiteConnectionString> _connectionString =
        new Lazy<SQLiteConnectionString>(() =>
            new SQLiteConnectionString(Path.Combine(Environment.GetEnvironmentVariable("HOME") ?? "", "Home.db")));

    public static string DatabasePath => _connectionString.Value.DatabasePath;

    public HomeDatabase()
    {
        _database = new SQLiteAsyncConnection(_connectionString.Value);
    }

    public async Task CreateTablesAsync()
    {
        await _database.CreateTableAsync<DataLogEvent>();
        await _database.CreateTableAsync<Device>();
    }

    public Task<Device[]> GetDevicesAsync()
    {
        return _database.Table<Device>().OrderBy(x => x.Name).ToArrayAsync();
    }

    public Task AddDeviceAsync(Device device)
    {
        return _database.InsertAsync(device);
    }

    public Task<DataLogEvent> GetLastEventAsync(DeviceId id)
    {
        return _database.Table<DataLogEvent>()
            .Where(x => x.DeviceId == id)
            .OrderByDescending(x => x.Timestamp)
            .FirstOrDefaultAsync();
    }

    public Task AddEventAsync(DataLogEvent dataLogEvent)
    {
        return _database.InsertAsync(dataLogEvent);
    }

    public Task<DataLogEvent[]> GetEventsAsync(int pageSize)
    {
        return _database.Table<DataLogEvent>()
            .OrderByDescending(x => x.Timestamp)
            .Take(pageSize)
            .ToArrayAsync();
    }

    public async Task<DataLogEvent?> GetThermostatUserSetpointAsync(DeviceId id)
    {
        var e = await _database.Table<DataLogEvent>()
            .Where(x => x.DeviceId == id && x.EventType == LogEventType.UserTemperatureSetting)
            .OrderByDescending(x => x.Timestamp)
            .FirstOrDefaultAsync()
            .ConfigureAwait(false);
        return e;
    }

    public async Task<DataLogEvent?> GetThermostatReadingAsync(DeviceId id)
    {
        var e = await _database.Table<DataLogEvent>()
            .Where(x => x.DeviceId == id && x.EventType == LogEventType.ThermostatReading)
            .OrderByDescending(x => x.Timestamp)
            .FirstOrDefaultAsync()
            .ConfigureAwait(false);
        return e;
    }

    public async Task<DataLogEvent[]> GetThermostatReadingsAsync(DeviceId id, int max)
    {
        var e = await _database.Table<DataLogEvent>()
            .Where(x => x.DeviceId == id && x.EventType == LogEventType.ThermostatReading)
            .OrderByDescending(x => x.Timestamp)
            .Take(max)
            .ToArrayAsync()
            .ConfigureAwait(false);
        return e;
    }

    public Task<DataLogEvent[]> GetEventsAsync(DeviceId id, DateTime start, DateTime end)
    {
        return _database.Table<DataLogEvent>()
            .Where(x => x.DeviceId == id && x.Timestamp >= start && x.Timestamp <= end)
            .OrderBy(x => x.Timestamp)
            .ToArrayAsync();
    }

    public async Task<byte[]> GetBackupAsync()
    {
        var tempPath = Path.GetTempFileName();
        try {
            await _database.BackupAsync(tempPath).ConfigureAwait(false);
            return await File.ReadAllBytesAsync(tempPath).ConfigureAwait(false);
        }
        finally {
            File.Delete(tempPath);
        }
    }
}


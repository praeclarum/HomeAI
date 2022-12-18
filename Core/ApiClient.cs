namespace HomeAI;

using System.Text;
using System.Text.Json;

public class ApiClient
{
    private readonly HttpClient httpClient = new HttpClient();
    const string baseUrl = "http://localhost:5280/api/";

    public ApiClient()
    {
        httpClient.BaseAddress = new Uri(baseUrl);        
    }

    public Task PostLogEventAsync(LogEventRequest logEvent) {
        return PostAsJsonAsync("events", logEvent);
    }

    public async Task<double> GetThermostatSetpointAsync(Guid id) {
        var r = await GetAsJsonAsync<ThermostatSetpointResponse>($"thermostatesetpoint/{id}");
        return r.setpoint;
    }

    class ThermostatSetpointResponse {
        public double setpoint { get; set; }
    }

    async Task PostAsJsonAsync<T>(string path, T data)
    {
        var json = JsonSerializer.Serialize(data);
        var resp = await httpClient.PostAsync(path,
            new StringContent(json, Encoding.UTF8, "application/json"));
        resp.EnsureSuccessStatusCode();
    }

    async Task<T> GetAsJsonAsync<T>(string path)
    {
        var resp = await httpClient.GetAsync(path);
        resp.EnsureSuccessStatusCode();
        var json = await resp.Content.ReadAsStringAsync();
        return JsonSerializer.Deserialize<T>(json) ?? throw new Exception("Failed to deserialize response");
    }
}

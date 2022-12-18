using Hub.Data;
using Hub.AI;
using Microsoft.AspNetCore.Mvc;

namespace Hub.Controllers;

public class ApiController : ControllerBase
{
    [Route("api/devices"), HttpGet]
    public async Task<IActionResult> GetDevicesAsync()
    {
        var db = new Data.HomeDatabase();
        await db.CreateTablesAsync();
        var devices = await db.GetDevicesAsync();
        return Ok(devices);
    }

    [Route("api/events"), HttpGet]
    public async Task<IActionResult> GetEventsAsync()
    {
        var db = new Data.HomeDatabase();
        var events = await db.GetEventsAsync(100);
        return Ok(events);
    }

    [Route("api/events"), HttpPost]
    public async Task<IActionResult> PostEventAsync([FromBody] LogEventRequest dataLogEvent)
    {
        if (dataLogEvent.Key != Secrets.HubWriteKey) {
            return Unauthorized();
        }
        var db = new Data.HomeDatabase();
        var data = new Data.DataLogEvent {
            Timestamp = dataLogEvent.Timestamp,
            DeviceId = dataLogEvent.DeviceId,
            EventType = dataLogEvent.EventType,
            Value = dataLogEvent.Value,
        };
        await db.AddEventAsync(data);
        return Ok(new{id=data.Id});
    }

    [Route("api/thermostatesetpoint/{id}"), HttpGet]
    public async Task<IActionResult> GetThermostatSetpointAsync(Guid id)
    {
        var db = new HomeDatabase();
        var setpoint = await new ThermostatAI().GetSetpointAsync(db, id);
        return Ok(new {setpoint = setpoint});
    }
}

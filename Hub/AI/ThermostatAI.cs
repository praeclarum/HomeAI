namespace Hub.AI;

using System;

using Hub.Data;

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
        return DefaultTemperature;
    }
}


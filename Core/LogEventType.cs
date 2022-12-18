namespace HomeAI;

public enum LogEventType {
    /// <summary>
    /// The current temperature from the thermostat in Celsius.
    /// </summary>
    ThermostatReading = 0,
    /// <summary>
    /// The last temperature that the user set.
    /// </summary>
    UserTemperatureSetting = 1,
    /// <summary>
    /// The last temperature commanded by the AI.
    /// </summary>
    AITemperatureSetting = 2,
    /// <summary>
    /// Is the heater active?
    /// </summary>
    HeaterOn = 3,
}

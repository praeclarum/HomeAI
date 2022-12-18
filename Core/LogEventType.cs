namespace HomeAI;

public enum LogEventType {
    /// <summary>
    /// The current temperature from the thermostat in Celsius.
    /// </summary>
    ThermostatReading,
    /// <summary>
    /// The last temperature that the user set.
    /// </summary>
    UserTemperatureSetting,
    /// <summary>
    /// The last temperature commanded by the AI.
    /// </summary>
    AITemperatureSetting,
}

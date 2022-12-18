using HomeAI;

Guid deviceId = Guid.Parse(args[0]);

var api = new ApiClient();

double setpoint = 0.0;
for (;;) {
    {
        var newSetpoint = await api.GetThermostatSetpointAsync(deviceId);
        if (newSetpoint != setpoint) {
            var oldSetpoint = setpoint;
            setpoint = newSetpoint;
            await api.PostLogEventAsync(new LogEventRequest {
                DeviceId = deviceId,
                Timestamp = DateTime.UtcNow,
                EventType = LogEventType.AITemperatureSetting,
                Value = newSetpoint,
            });
            Console.WriteLine($"Setpoint changed from {oldSetpoint} to {newSetpoint}");
        }
    }
    Console.Write($"thermostat {deviceId} {setpoint}> ");
    var cmdLine = (Console.ReadLine()??"").Split(' ');
    var cmd = cmdLine[0];
    var cmdArgs = cmdLine[1..];
    switch (cmd)
    {
        case "exit":
        case "quit":
        case "e":
        case "q":
            return;
        case "read":
            if (cmdArgs.Length != 1) {
                Console.WriteLine("Usage: read <temperature in celsius>");
                break;
            }
            if (!double.TryParse(cmdArgs[0], out var readTemp)) {
                Console.WriteLine("Invalid temperature");
                break;
            }
            await api.PostLogEventAsync(new LogEventRequest {
                DeviceId = deviceId,
                Timestamp = DateTime.UtcNow,
                EventType = LogEventType.ThermostatReading,
                Value = readTemp,
            });
            Console.WriteLine($"Read temperature {readTemp} C");
            break;
        case "set":
            if (cmdArgs.Length != 1) {
                Console.WriteLine("Usage: set <temperature in celsius>");
                break;
            }
            if (!double.TryParse(cmdArgs[0], out var setTemp)) {
                Console.WriteLine("Invalid temperature");
                break;
            }
            await api.PostLogEventAsync(new LogEventRequest {
                DeviceId = deviceId,
                Timestamp = DateTime.UtcNow,
                EventType = LogEventType.UserTemperatureSetting,
                Value = setTemp,
            });
            Console.WriteLine($"Setting temperature to {setTemp} C");

            break;
    }
}







namespace HomeAI;

public static class UnitConversion
{
    public static double CelsiusToFahrenheit(this double celsius) => celsius * 9.0 / 5.0 + 32.0;
    public static double FahrenheitToCelsius(this double fahrenheit) => (fahrenheit - 32) * 5.0 / 9.0;
}

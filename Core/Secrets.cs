namespace HomeAI;

public static class Secrets
{
    public static string HubWriteKey => "HUB_WRITE_KEY";

    public static bool CanWriteWithKey(string key) => key == HubWriteKey;
}

using Hub.Data;
using Hub.AI;
using Microsoft.AspNetCore.Mvc;

namespace Hub.Controllers;

public class BackupController : ControllerBase
{
    [Route("backup/Home.db"), HttpGet]
    public async Task<IActionResult> GetDevicesAsync(HomeDatabase db)
    {
        return File(await db.GetBackupAsync().ConfigureAwait(false), "application/octet-stream", "Home.db");
    }
}

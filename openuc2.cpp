#include "openUC2.h"
#include "UC2Hub.h"
#include "XYStage.h"
#include "ZStage.h"
#include "Laser.h"
#include "ModuleInterface.h"
#include <cstring>
#include "UC2DEFAULTS.h"

// Module API: registers devices for Micro-Manager

MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_HubName,     MM::HubDevice,    "openUC2 hub device");
   RegisterDevice(g_XYStageName, MM::XYStageDevice,"XY Stage for openUC2");
   RegisterDevice(g_ZStageName,  MM::StageDevice,  "Z Stage for openUC2");
   // We have several shutter devices
   RegisterDevice(g_LaserName1, MM::ShutterDevice, UC2Defaults::laserName1);
   RegisterDevice(g_LaserName2, MM::ShutterDevice, UC2Defaults::laserName2);
   RegisterDevice(g_LaserName3, MM::ShutterDevice, UC2Defaults::laserName3);
   RegisterDevice(g_LaserName4, MM::ShutterDevice, UC2Defaults::laserName4);
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (!deviceName)
      return 0;
   
   
   if (strcmp(deviceName, g_HubName) == 0)
      return new UC2Hub();
   else if (strcmp(deviceName, g_XYStageName) == 0)
      return new XYStage();
   else if (strcmp(deviceName, g_ZStageName) == 0)
      return new ZStage();
   else if (strcmp(deviceName, g_LaserName1) == 0)
      return new UC2Laser(UC2Defaults::laserId_1);
   else if (strcmp(deviceName, g_LaserName2) == 0)
       return new UC2Laser(UC2Defaults::laserId_2);
   else if (strcmp(deviceName, g_LaserName3) == 0)
       return new UC2Laser(UC2Defaults::laserId_3);
   else if (strcmp(deviceName, g_LaserName4) == 0)
       return new UC2Laser(UC2Defaults::laserId_4);

   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

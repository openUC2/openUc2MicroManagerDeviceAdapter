
#include "openUC2.h"
#include "UC2Hub.h"
#include "XYStage.h"
#include "ZStage.h"
#include "Shutter.h"

// Required Micro-Manager API:
MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_HubName,     MM::HubDevice,    "openUC2 hub device");
   RegisterDevice(g_XYStageName, MM::XYStageDevice,"XY Stage for openUC2");
   RegisterDevice(g_ZStageName,  MM::StageDevice,  "Z Stage for openUC2");
   RegisterDevice(g_ShutterName, MM::ShutterDevice,"LED/Laser Shutter for openUC2");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (!deviceName)
      return nullptr;

   if (strcmp(deviceName, g_HubName) == 0)
      return new UC2Hub();
   else if (strcmp(deviceName, g_XYStageName) == 0)
      return new XYStage();
   else if (strcmp(deviceName, g_ZStageName) == 0)
      return new ZStage();
   else if (strcmp(deviceName, g_ShutterName) == 0)
      return new UC2Shutter();

   // Unknown device
   return nullptr;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

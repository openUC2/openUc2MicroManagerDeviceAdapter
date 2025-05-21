#ifndef _OPENUC2_H_
#define _OPENUC2_H_

#include "MMDevice.h"
#include "DeviceBase.h"
#include <string>

//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////

// Global device names
static const char* g_HubName       = "openUC2-Hub";
static const char* g_XYStageName   = "openUC2-XYStage";
static const char* g_ZStageName    = "openUC2-ZStage";
static const char* g_LaserName1    = "openUC2-Laser1";
static const char* g_LaserName2 = "openUC2-Laser2";
static const char* g_LaserName3 = "openUC2-Laser3";
static const char* g_LaserName4 = "openUC2-Laser4";

// Example error code
enum {
   ERR_NO_PORT_SET = 1001
};

#endif // _OPENUC2_H_

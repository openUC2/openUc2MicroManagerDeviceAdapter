
#include "XYStage.h"
#include "UC2Hub.h"
#include "ModuleInterface.h"
#include <sstream>

XYStage::XYStage() :
   initialized_(false),
   hub_(nullptr),
   stepSizeUm_(0.05),
   posX_(0.0),
   posY_(0.0)
{
   // default step size and positions
}

XYStage::~XYStage()
{
   Shutdown();
}

int XYStage::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Get pointer to our hub
   hub_ = dynamic_cast<UC2Hub*>(GetParentHub());
   if (!hub_) {
      return ERR_NO_PORT_SET;
   }

   // set property list if needed (speed, acceleration, etc.)
   // e.g.
   // CreateFloatProperty("XY-Speed", 5000.0, false, ...);

   // Mark done
   initialized_ = true;
   return DEVICE_OK;
}

int XYStage::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}

bool XYStage::Busy()
{
   // if the device has a means to say it's moving, you can query it:
   return false;
}

int XYStage::SetPositionUm(double x, double y)
{
   if (!initialized_)
      return DEVICE_NOT_CONNECTED;

   // Convert to steps if needed
   long stepsX = (long)((x - posX_) / stepSizeUm_);
   long stepsY = (long)((y - posY_) / stepSizeUm_);

   // Example JSON to do absolute move:
   std::ostringstream ss;
   ss << R"({"task":"/motor_act","motor":{"steppers":[)"
      << R"({"stepperid":1,"position":)" << (long)(x / stepSizeUm_) << R"(,"speed":5000,"isabs":1},)"
      << R"({"stepperid":2,"position":)" << (long)(y / stepSizeUm_) << R"(,"speed":5000,"isabs":1})"
      << "]}}";

   std::string cmd = ss.str();
   std::string reply;
   int ret = hub_->SendJsonCommand(cmd, reply, false);
   if (ret != DEVICE_OK)
      return ret;

   // If success, update cached positions
   posX_ = x;
   posY_ = y;
   return DEVICE_OK;
}

int XYStage::GetPositionUm(double& x, double& y)
{
   // Example JSON to request position:
   std::string cmd = R"({"task":"/motor_get","position":true})";
   std::string reply;
   int ret = hub_->SendJsonCommand(cmd, reply, false);
   if (ret != DEVICE_OK)
      return ret;

   // In real code, parse 'reply' to find the positions for stepperid=1 and 2
   // For example:
   //   "motor":{"steppers":[ {"stepperid":1,"position":1234}, {"stepperid":2,"position":5678} ]}
   // Then multiply by stepSizeUm_ to get X, Y in microns.
   //
   // For now, just return our cached posX_, posY_ to keep it simple:
   x = posX_;
   y = posY_;
   return DEVICE_OK;
}

int XYStage::SetRelativePositionUm(double dx, double dy)
{
   return SetPositionUm(posX_ + dx, posY_ + dy);
}

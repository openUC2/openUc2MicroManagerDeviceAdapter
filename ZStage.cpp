
#include "ZStage.h"
#include "UC2Hub.h"
#include <sstream>

ZStage::ZStage() :
   initialized_(false),
   hub_(nullptr),
   stepSizeUm_(0.1),
   posZ_(0.0)
{
}

ZStage::~ZStage()
{
   Shutdown();
}

int ZStage::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   hub_ = dynamic_cast<UC2Hub*>(GetParentHub());
   if (!hub_)
      return ERR_NO_PORT_SET;

   // Possibly create properties for speed, acceleration, etc.

   initialized_ = true;
   return DEVICE_OK;
}

int ZStage::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}

bool ZStage::Busy()
{
   // Query if needed
   return false;
}

int ZStage::SetPositionUm(double z)
{
   if (!initialized_) return DEVICE_NOT_CONNECTED;

   // Example JSON for absolute Z move: stepperid=3, isabs=1
   long stepsZ = (long)(z / stepSizeUm_);

   std::ostringstream ss;
   ss << R"({"task":"/motor_act","motor":{"steppers":[)"
      << R"({"stepperid":3,"position":)" << stepsZ << R"(,"speed":2000,"isabs":1})"
      << "]}}";

   std::string cmd = ss.str();
   std::string reply;
   int ret = hub_->SendJsonCommand(cmd, reply);
   if (ret != DEVICE_OK)
      return ret;

   posZ_ = z;
   return DEVICE_OK;
}

int ZStage::GetPositionUm(double& z)
{
   // Same approach as XY: parse motor_get reply or rely on cached posZ_.
   z = posZ_;
   return DEVICE_OK;
}

int ZStage::SetRelativePositionUm(double dz)
{
   return SetPositionUm(posZ_ + dz);
}

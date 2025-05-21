
#include "Laser.h"
#include "UC2Hub.h"
#include <sstream>
#include "UC2DEFAULTS.h"

UC2Laser::UC2Laser(int laserId) :
   initialized_(false),
   hub_(nullptr),
   open_(false),
   id_(laserId),
   power_(0)

{
   // constructor
}

UC2Laser::~UC2Laser()
{
   Shutdown();
}


void UC2Laser::GetName(char* name) const 
{ 
    if (id_ == UC2Defaults::laserId_1) CDeviceUtils::CopyLimitedString(name, g_LaserName1); 
    if (id_ == UC2Defaults::laserId_2) CDeviceUtils::CopyLimitedString(name, g_LaserName2);
    if (id_ == UC2Defaults::laserId_3) CDeviceUtils::CopyLimitedString(name, g_LaserName3);
    if (id_ == UC2Defaults::laserId_4) CDeviceUtils::CopyLimitedString(name, g_LaserName4);
}
int UC2Laser::Initialize()
{


   if (initialized_)
      return DEVICE_OK;

   hub_ = dynamic_cast<UC2Hub*>(GetParentHub());
   if (!hub_)
      return ERR_NO_PORT_SET;

   if (initialized_)
       return DEVICE_OK;
   
   

   CreateProperty("UC2LaserPower", "0", MM::Integer, false, new CPropertyAction(this, &UC2Laser::OnPower));
   SetPropertyLimits("UC2LaserPower", UC2Defaults::minLaserPower, UC2Defaults::maxLaserPower);

   
   initialized_ = true;
   return DEVICE_OK;
}

int UC2Laser::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}

bool UC2Laser::Busy()
{
   return false;
}

int UC2Laser::SetOpen(bool open)
{
   if (!initialized_)
      return DEVICE_NOT_CONNECTED;
   std::string cmd;
   if (open)
   {
       hub_->GetUC2Tools().cmdSetLaser(id_, power_, cmd);
   }
   else hub_->GetUC2Tools().cmdSetLaser(id_, 0, cmd);
   hub_->SendJsonCommand(cmd);
   open_ = open;
   return DEVICE_OK;
}

int UC2Laser::GetOpen(bool& open)
{
   // We cannot easily query the device, so we return our cached state:
   open = open_;
   return DEVICE_OK;
}




int UC2Laser::OnPower(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)power_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        power_ = static_cast<int>(val);
        std::string cmd;
        if (open_)
        {
            hub_->GetUC2Tools().cmdSetLaser(id_, power_, cmd);
            hub_->SendJsonCommand(cmd);
        }

    }
    return DEVICE_OK;
}
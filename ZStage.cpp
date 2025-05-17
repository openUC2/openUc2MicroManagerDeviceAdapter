#include "ZStage.h"
#include "UC2Hub.h"
#include <sstream>
#include "UC2DEFAULTS.h"

ZStage::ZStage() :
   initialized_(false),
   hub_(nullptr),
    homeSpeedZ_(UC2Defaults::ZAxHomingSpeed),
    homeTimeOutZ_(UC2Defaults::ZAxHomingTimeOut),
    homeDirZ_(UC2Defaults::ZAxHomingDirection),
    homePolarityZ_(UC2Defaults::ZAxHomingEndStopPolarity)
{}

ZStage::~ZStage()
{
   Shutdown();
}

int ZStage::Initialize()
{
   if (initialized_)
      return DEVICE_OK;
   speedZ_ = UC2Defaults::ZAxSpeed;
   std::string sZSpeed = std::to_string(UC2Defaults::ZAxSpeed);
   

   //homeSpeedZ_ = UC2Defaults::ZAxHomingSpeed;
   //homeTimeOutZ_ = UC2Defaults::ZAxHomingTimeOut;
   homeDirZ_ = (UC2Defaults::ZAxHomingDirection >= 0) ? 1 : -1;
   homePolarityZ_ = (UC2Defaults::ZAxHomingEndStopPolarity >= 0) ? 1 : -1;
   std::string shZSpeed = std::to_string(UC2Defaults::XAxHomingSpeed);
   std::string shZto = std::to_string(UC2Defaults::XAxHomingTimeOut);
   std::string shZd = (homeDirZ_ >= 0) ? "POS" : "NEG";
   std::string shZp = (homePolarityZ_ >= 0) ? "POS" : "NEG";
   

   CreateProperty("SpeedZ", sZSpeed.c_str(), MM::Integer, false, new CPropertyAction(this, &ZStage::OnSpeedZ));
   SetPropertyLimits("SpeedZ", UC2Defaults::ZAxMinSpeed, UC2Defaults::ZAxMaxSpeed);

   CreateProperty("HomeSpeedZ", shZSpeed.c_str(), MM::Integer, false, new CPropertyAction(this, &ZStage::OnHomeSpeedZ));
   SetPropertyLimits("HomeSpeedZ", UC2Defaults::XAxMinSpeed, UC2Defaults::XAxMaxSpeed);

   CreateProperty("HomeTimeoutX", shZto.c_str(), MM::Integer, false, new CPropertyAction(this, &ZStage::OnHomeTimeoutZ));
   SetPropertyLimits("HomeTimeoutX", UC2Defaults::XAxMinSpeed, UC2Defaults::XAxMaxSpeed);

   CreateProperty("HomePolarityX", shZp.c_str(), MM::String, false, new CPropertyAction(this, &ZStage::OnHomePolarityZ));
   AddAllowedValue("HomePolarityX", "POS");
   AddAllowedValue("HomePolarityX", "NEG");

   CreateProperty("HomeDirectionX", shZd.c_str(), MM::String, false, new CPropertyAction(this, &ZStage::OnHomeDirectionZ));
   AddAllowedValue("HomeDirectionX", "POS");
   AddAllowedValue("HomeDirecitonX", "NEG");


   hub_ = dynamic_cast<UC2Hub*>(GetParentHub());
   if (!hub_)
      return ERR_NO_PORT_SET;

   hub_->zReqiresFastPoll(false);
   std::string cmd;
   std::string reply;
   hub_->GetUC2Tools().cmdGetPos(cmd);
   hub_->SendJsonCommand(cmd, reply);
   returnState ret_ = hub_->GetUC2Tools().parseJSon(reply);

   if (ret_ == MOTOR)
   {
       long posZSteps_ = hub_->GetUC2Tools().GetPos(UC2Defaults::ZAxID);
       
       std::string sz = "UC2 Z Stage:: Init Z Posistion is " + std::to_string(posZSteps_);
       
       LogMessage(sz.c_str(), true);
       initialized_ = true;
       return DEVICE_OK;
   }
   std::string _msg = "Unexpected device response upon positioning request: " + UC2Tools::returnStateToString(ret_);
   SetErrorText(ERR_UC2JSON_ERR, _msg.c_str());
   return ERR_UC2JSON_ERR;
}

int ZStage::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}

void ZStage::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_ZStageName);
}

bool ZStage::Busy()
{
    // is called after "SetPositionSteps in a loop until returns false
       // stores also what ever what was read into serialparsers
    hub_->zReqiresFastPoll(true);
    hub_->FastPollSerial();
    // feeding to json parsers (which sets the isMoving flag, if correct command comes in)
    int ret = hub_->GetUC2Tools().parseJSon(hub_->GetSerParser().getFirstReady());
    bool moving_ = hub_->GetUC2Tools().GetStepper(UC2Defaults::ZAxID).isMoving;
    hub_->zReqiresFastPoll(moving_);
    return moving_;
}

int ZStage::SetPositionSteps(long steps)
{
    if (!initialized_) return DEVICE_NOT_CONNECTED;
    std::string msg = "Moving to steps z = " + std::to_string(steps) ;
    LogMessage(msg.c_str(), true);
    std::string cmd;
    
    bool move_ = (hub_->GetUC2Tools().GetPos(UC2Defaults::ZAxID) != steps);
    if (move_) {

        int ret = hub_->GetUC2Tools().cmdMoveMotorAbs(UC2Defaults::ZAxID, steps, speedZ_, cmd);
        if (ret != DEVICE_OK)
        {
            SetErrorText(ret, cmd.c_str());
            return ret;
        }
        hub_->SendJsonCommand(cmd);
    }
    return DEVICE_OK;
}

int ZStage::GetPositionSteps(long& steps)
{
   if (!initialized_) return DEVICE_NOT_CONNECTED;
   steps = hub_->GetUC2Tools().GetPos(UC2Defaults::ZAxID);
   return DEVICE_OK;
}

// Additional helper; not declared in base so remove 'override'
int ZStage::SetRelativePositionSteps(long steps)
{
    if (!initialized_) return DEVICE_NOT_CONNECTED;
    std::string msg = "Moving to steps z = " + std::to_string(steps);
    LogMessage(msg.c_str(), true);
    
    std::string cmd;
    if (steps != 0)
    {
        int ret = hub_->GetUC2Tools().cmdMoveMotorRel(UC2Defaults::ZAxID, steps, speedZ_, cmd);
        if (ret != DEVICE_OK)
        {
            SetErrorText(ret, cmd.c_str());
            return ret;
        }
        hub_->SendJsonCommand(cmd);
    }
    return DEVICE_OK;
}

int ZStage::Home()
{
    if (!initialized_) return DEVICE_NOT_CONNECTED;
    std::string cmd;
    int ret = hub_->GetUC2Tools().cmdHome(UC2Defaults::ZAxID, homeTimeOutZ_, homeSpeedZ_, homeDirZ_, homePolarityZ_, cmd);
    if (ret != DEVICE_OK)
    {
        SetErrorText(ret, cmd.c_str());
        return ret;
    }
    // if any is moving prepare fast polling mode, to listen for serial response
    hub_->SendJsonCommand(cmd);
    return DEVICE_OK;
}

int ZStage::Stop()
{
    std::string cmd;
    int ret = hub_->GetUC2Tools().cmdStopMotor(UC2Defaults::ZAxID, cmd);
    hub_->SendJsonCommand(cmd);
    return DEVICE_OK;
}

int ZStage::GetLimits(double& min, double& max)
{
   min = UC2Defaults::ZAxMinPos * UC2Defaults::ZStepSizeUM;
   max = UC2Defaults::ZAxMaxPos * UC2Defaults::ZStepSizeUM; 
   return DEVICE_OK;
}

int ZStage::SetPositionUm(double z)
{
   long steps = static_cast<long>(z / UC2Defaults::ZStepSizeUM);
   return SetPositionSteps(steps);
}

int ZStage::GetPositionUm(double& z)
{
   long steps = 0;
   int ret = GetPositionSteps(steps);
   if (ret != DEVICE_OK)
      return ret;
   z = steps * UC2Defaults::ZStepSizeUM;
   return DEVICE_OK;
}

int ZStage::SetOrigin()
{
 
   return DEVICE_OK;
}

int ZStage::IsStageSequenceable(bool& isSequenceable) const
{
   isSequenceable = false;
   return DEVICE_OK;
}

bool ZStage::IsContinuousFocusDrive() const
{
   return false;
}


int ZStage::OnSpeedZ(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)speedZ_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        speedZ_ = static_cast<int>(val);

    }
    return DEVICE_OK;
}


int ZStage::OnHomeSpeedZ(MM::PropertyBase* pProp, MM::ActionType eAct) {

    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)homeSpeedZ_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        homeSpeedZ_ = static_cast<int>(val);

    }
    return DEVICE_OK;
};
int ZStage::OnHomePolarityZ(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::AfterSet)
    {
        std::string val;
        pProp->Get(val);
        homePolarityZ_ = (val == "POS") ? 1 : -1;
    }
    return DEVICE_OK;
};
int ZStage::OnHomeDirectionZ(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::AfterSet)
    {
        std::string val;
        pProp->Get(val);
        homeDirZ_ = (val == "POS") ? 1 : -1;
    }
    return DEVICE_OK;


};
int ZStage::OnHomeTimeoutZ(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)homeTimeOutZ_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        homeTimeOutZ_ = static_cast<int>(val);

    }
    return DEVICE_OK;
};
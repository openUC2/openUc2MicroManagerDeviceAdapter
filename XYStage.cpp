#include "XYStage.h"
#include "UC2Hub.h"
#include <sstream>
#include "UC2DEFAULTS.h"



XYStage::XYStage() :
   initialized_(false),
   hub_(nullptr),
   speedX_(UC2Defaults::XAxSpeed),
   speedY_(UC2Defaults::YAxSpeed),
    homeSpeedX_(UC2Defaults::XAxHomingSpeed),
    homeSpeedY_(UC2Defaults::YAxHomingSpeed),
    homeTimeOutX_(UC2Defaults::XAxHomingTimeOut),
    homeTimeOutY_(UC2Defaults::YAxHomingTimeOut),
    homeDirX_(UC2Defaults::XAxHomingDirection),
    homeDirY_(UC2Defaults::YAxHomingDirection),
    homePolarityX_(UC2Defaults::XAxHomingEndStopPolarity),
    homePolarityY_(UC2Defaults::YAxHomingEndStopPolarity)
{
}

XYStage::~XYStage()
{
   Shutdown();
}

int XYStage::Initialize()
{
    // SpeedX Property
    speedX_ = UC2Defaults::XAxSpeed;
    speedY_ = UC2Defaults::YAxSpeed;
    homeSpeedX_ = UC2Defaults::XAxHomingSpeed;
    homeSpeedY_ = UC2Defaults::YAxHomingSpeed;
    homeTimeOutX_ = UC2Defaults::XAxHomingTimeOut;
    homeTimeOutY_ = UC2Defaults::YAxHomingTimeOut;
    homeDirX_ = (UC2Defaults::XAxHomingDirection >= 0) ? 1 : -1;
    homeDirY_ = (UC2Defaults::YAxHomingDirection >= 0) ? 1 : -1;
    homePolarityX_ = (UC2Defaults::XAxHomingEndStopPolarity >= 0) ? 1 : -1;
    homePolarityY_ = (UC2Defaults::YAxHomingEndStopPolarity >= 0) ? 1 : -1;
    std::string sXSpeed = std::to_string(UC2Defaults::XAxSpeed);
    std::string sYSpeed = std::to_string(UC2Defaults::YAxSpeed);
    std::string shXSpeed = std::to_string(UC2Defaults::XAxHomingSpeed);
    std::string shYSpeed = std::to_string(UC2Defaults::YAxHomingSpeed);
    std::string shXto = std::to_string(UC2Defaults::XAxHomingTimeOut);
    std::string shYto = std::to_string(UC2Defaults::YAxHomingTimeOut);
    std::string shXd = (homeDirX_ >= 0) ? "POS" : "NEG";
    std::string shYd = (homeDirY_ >= 0) ? "POS" : "NEG";
    std::string shXp = (homePolarityX_ >= 0) ? "POS" : "NEG";
    std::string shYp = (homePolarityY_ >= 0) ? "POS" : "NEG";


    CreateProperty("SpeedX", sXSpeed.c_str(), MM::Integer, false, new CPropertyAction(this, &XYStage::OnSpeedX));
    SetPropertyLimits("SpeedX", UC2Defaults::XAxMinSpeed, UC2Defaults::XAxMaxSpeed);

    
    CreateProperty("SpeedY", sYSpeed.c_str(), MM::Integer, false, new CPropertyAction(this, &XYStage::OnSpeedY));
    SetPropertyLimits("SpeedY", UC2Defaults::YAxMinSpeed, UC2Defaults::YAxMaxSpeed);

    
    CreateProperty("HomeSpeedX", shXSpeed.c_str(), MM::Integer, false, new CPropertyAction(this, &XYStage::OnHomeSpeedX));
    SetPropertyLimits("HomeSpeedX", UC2Defaults::XAxMinSpeed, UC2Defaults::XAxMaxSpeed);

    CreateProperty("HomeSpeedY", shYSpeed.c_str(), MM::Integer, false, new CPropertyAction(this, &XYStage::OnHomeSpeedY));
    SetPropertyLimits("HomeSpeedY", UC2Defaults::YAxMinSpeed, UC2Defaults::YAxMaxSpeed);

    CreateProperty("HomeTimeoutX", shXto.c_str(), MM::Integer, false, new CPropertyAction(this, &XYStage::OnHomeTimeoutX));
    SetPropertyLimits("HomeTimeoutX", UC2Defaults::XAxMinSpeed, UC2Defaults::XAxMaxSpeed);

    CreateProperty("HomeTimeoutY", shYto.c_str(), MM::Integer, false, new CPropertyAction(this, &XYStage::OnHomeTimeoutY));
    SetPropertyLimits("HomeTimeoutY", UC2Defaults::YAxMinSpeed, UC2Defaults::YAxMaxSpeed);

    CreateProperty("HomePolarityX", shXp.c_str(), MM::String, false, new CPropertyAction(this, &XYStage::OnHomePolarityX));
    AddAllowedValue("HomePolarityX", "POS");
    AddAllowedValue("HomePolarityX", "NEG");

    CreateProperty("HomePolarityY", shYp.c_str(), MM::String, false, new CPropertyAction(this, &XYStage::OnHomePolarityY));
    AddAllowedValue("HomePolarityY", "POS");
    AddAllowedValue("HomePolarityY", "NEG");

    CreateProperty("HomeDirectionX", shXd.c_str(), MM::String, false, new CPropertyAction(this, &XYStage::OnHomeDirectionX));
    AddAllowedValue("HomeDirectionX", "POS");
    AddAllowedValue("HomeDirecitonX", "NEG");

    CreateProperty("HomeDirectionY", shYd.c_str(), MM::String, false, new CPropertyAction(this, &XYStage::OnHomeDirectionY));
    AddAllowedValue("HomeDirectionY", "POS");
    AddAllowedValue("HomeDirecitonY", "NEG");
    
    
    


    // SetOriginHere (can only be set)
    //CreateProperty("SetOriginHere", "0", MM::Integer, false, new CPropertyAction(this, &XYStage::OnSetOriginHere));
    //AddAllowedValue("SetOriginHere", "0");
    //AddAllowedValue("SetOriginHere", "1");

    
   if (initialized_)
      return DEVICE_OK;
   hub_ = dynamic_cast<UC2Hub*>(GetParentHub());
   if (!hub_)
      return ERR_NO_PORT_SET;
   hub_->xyReqiresFastPoll(false);
   std::string cmd;
   std::string reply;
   hub_->GetUC2Tools().cmdGetPos(cmd);
   hub_->SendJsonCommand(cmd, reply);
   returnState ret_ = hub_->GetUC2Tools().parseJSon(reply);
   
   if (ret_ == MOTOR)
   {
       long posXSteps_ = hub_->GetUC2Tools().GetPos(UC2Defaults::XAxID);
       long posYSteps_ = hub_->GetUC2Tools().GetPos(UC2Defaults::YAxID);
       std::string sx = "UC2 XY Stage:: Init X Posistion is " + std::to_string(posXSteps_);
       std::string sy = "UC2 XY Stage:: Init Y Posistion is " + std::to_string(posYSteps_);
       LogMessage(sx.c_str(), true);
       LogMessage(sy.c_str(), true);
       initialized_ = true;
       return DEVICE_OK;
   }
   std::string _msg = "Unexpected device response upon positioning request: " +UC2Tools::returnStateToString(ret_);
   SetErrorText(ERR_UC2JSON_ERR, _msg.c_str());
   return ERR_UC2JSON_ERR;
}

int XYStage::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}

void XYStage::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_XYStageName);
}

bool XYStage::Busy()
{
    // is called after "SetPositionSteps in a loop until returns false
    // stores also what ever what was read into serialparsers
    hub_->xyReqiresFastPoll(true);
    hub_->FastPollSerial();
    // feeding to json parsers (which sets the isMoving flag, if correct command comes in)
    int ret = hub_->GetUC2Tools().parseJSon(hub_->GetSerParser().getFirstReady());
    bool moving_ = hub_->GetUC2Tools().GetStepper(UC2Defaults::XAxID).isMoving || hub_->GetUC2Tools().GetStepper(UC2Defaults::YAxID).isMoving;
    hub_->xyReqiresFastPoll(moving_);
    return moving_;
    
}


int XYStage::SetPositionSteps(long x, long y)
{
    
   if (!initialized_) return DEVICE_NOT_CONNECTED;
   std::string msg = "Moving to steps (x = " + std::to_string(x) + " ; y = " + std::to_string(y) + ")";
   LogMessage(msg.c_str(), true);
   std::string cmdX;
   std::string cmdY;
   bool moveX_ = (hub_->GetUC2Tools().GetPos(UC2Defaults::XAxID) != x);
   bool moveY_ = (hub_->GetUC2Tools().GetPos(UC2Defaults::YAxID) != y);

   if (moveX_) {
       int retX = hub_->GetUC2Tools().cmdMoveMotorAbs(UC2Defaults::XAxID, x, UC2Defaults::XAxSpeed, cmdX);
       if (retX != DEVICE_OK)
       {
           SetErrorText(retX, cmdX.c_str());
           return retX;
       }
   }
   if (moveY_) {

       int retY = hub_->GetUC2Tools().cmdMoveMotorAbs(UC2Defaults::YAxID, y, UC2Defaults::YAxSpeed, cmdY);
       if (retY != DEVICE_OK)
       {
           SetErrorText(retY, cmdY.c_str());
           return retY;
       }
   }
   
   if (moveX_) hub_->SendJsonCommand(cmdX);
   if (moveY_) hub_->SendJsonCommand(cmdY);
   return DEVICE_OK;
}

int XYStage::GetPositionSteps(long& x, long& y)
{
   if (!initialized_) return DEVICE_NOT_CONNECTED;
   x = hub_->GetUC2Tools().GetPos(UC2Defaults::XAxID);
   y = hub_->GetUC2Tools().GetPos(UC2Defaults::YAxID);
   return DEVICE_OK;
}

int XYStage::SetRelativePositionSteps(long x, long y)
{
    if (!initialized_) return DEVICE_NOT_CONNECTED;
    std::string msg = "Moving to steps (x = " + std::to_string(x) + " ; y = " + std::to_string(y) + ")";
    LogMessage(msg.c_str(), true);
    std::string cmdX;
    std::string cmdY;
    if (x != 0)
    {
        int retX = hub_->GetUC2Tools().cmdMoveMotorRel(UC2Defaults::XAxID, x, speedX_, cmdX);
        if (retX != DEVICE_OK)
        {
            SetErrorText(retX, cmdX.c_str());
            return retX;
        }
    }
    if (y != 0)
    {
        int retY = hub_->GetUC2Tools().cmdMoveMotorRel(UC2Defaults::YAxID, y, speedY_, cmdY);
        if (retY != DEVICE_OK)
        {
            SetErrorText(retY, cmdY.c_str());
            return retY;
        }
    }
    // if any is moving prepare fast polling mode, to listen for serial response
    
    if (x!=0) hub_->SendJsonCommand(cmdX);
    if (y!=0) hub_->SendJsonCommand(cmdY);
    return DEVICE_OK;
    
}

int XYStage::Home()
{
    if (!initialized_) return DEVICE_NOT_CONNECTED;
    std::string cmdX;
    std::string cmdY;
    int retX = hub_->GetUC2Tools().cmdHome(UC2Defaults::XAxID, homeTimeOutX_, homeSpeedX_, homeDirX_, homePolarityX_, cmdX);
    if (retX != DEVICE_OK)
    {
        SetErrorText(retX, cmdX.c_str());
        return retX;
    }
    int retY = hub_->GetUC2Tools().cmdHome(UC2Defaults::YAxID, homeTimeOutY_, homeSpeedY_, homeDirY_, homePolarityY_, cmdY);
    if (retY != DEVICE_OK)
    {
        SetErrorText(retY, cmdY.c_str());
        return retY;
    }
    // if any is moving prepare fast polling mode, to listen for serial response
    hub_->SendJsonCommand(cmdX);
    hub_->SendJsonCommand(cmdY);
    return DEVICE_OK;
}

int XYStage::Stop()
{
    std::string cmd;
    int ret = hub_->GetUC2Tools().cmdStopMotor(UC2Defaults::XAxID, cmd);
    hub_->SendJsonCommand(cmd);
    ret = hub_->GetUC2Tools().cmdStopMotor(UC2Defaults::YAxID, cmd);
    hub_->SendJsonCommand(cmd);
   // If hardware supports a stop command, send it here.
   return DEVICE_OK;
}

int XYStage::GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax)
{
   // one could as well go via hub_->GetUC2Tools().GetStepper, but this is simpler
   xMin = UC2Defaults::XAxMinPos;
   xMax = UC2Defaults::XAxMaxPos;
   yMin = UC2Defaults::YAxMinPos;
   yMax = UC2Defaults::YAxMaxPos;
   return DEVICE_OK;
}


// required? What does it do? -> is required!
int XYStage::IsXYStageSequenceable(bool& isSequenceable) const
{
   isSequenceable = false;
   return DEVICE_OK;
}

int XYStage::GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax)
{
   long lxMin, lxMax, lyMin, lyMax;
   int ret = GetStepLimits(lxMin, lxMax, lyMin, lyMax);
   if (ret != DEVICE_OK)
      return ret;
   xMin = lxMin * UC2Defaults::XStepSizeUM;
   xMax = lxMax * UC2Defaults::XStepSizeUM;
   yMin = lyMin * UC2Defaults::YStepSizeUM;
   yMax = lyMax * UC2Defaults::YStepSizeUM;
   return DEVICE_OK;
}

int XYStage::SetOrigin()
{
   // Define the current position as the origin (0,0).
   // TODO:  Shift limits etc!
 //  posXSteps_ = 0;
 //  posYSteps_ = 0;
   return DEVICE_OK;
}


int XYStage::OnSpeedX(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)speedX_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        speedX_ = static_cast<int>(val);
        
    }
    return DEVICE_OK;
}

int XYStage::OnSpeedY(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)speedY_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        speedY_ = static_cast<int>(val);
        
    }
    return DEVICE_OK;
}


int XYStage::OnHomeSpeedX(MM::PropertyBase* pProp, MM::ActionType eAct) {
    
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)homeSpeedX_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        homeSpeedX_ = static_cast<int>(val);

    }
    return DEVICE_OK;
};
int XYStage::OnHomePolarityX(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::AfterSet)
    {
        std::string val;
        pProp->Get(val);
        homePolarityX_ = (val == "POS") ? 1 : -1;
    }
    return DEVICE_OK;
};
int XYStage::OnHomeDirectionX(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::AfterSet)
    {
        std::string val;
        pProp->Get(val);
        homeDirX_ = (val == "POS") ? 1 : -1;
    }
    return DEVICE_OK;


};
int XYStage::OnHomeTimeoutX(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)homeTimeOutX_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        homeTimeOutX_ = static_cast<int>(val);

    }
    return DEVICE_OK;
};
int XYStage::OnHomeSpeedY(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)homeSpeedY_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        homeSpeedY_ = static_cast<int>(val);

    }
    return DEVICE_OK;
};
int XYStage::OnHomePolarityY(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::AfterSet)
    {
        std::string val;
        pProp->Get(val);
        homePolarityY_ = (val == "POS") ? 1 : -1;
    }
    return DEVICE_OK;
};
int XYStage::OnHomeDirectionY(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::AfterSet)
    {
        std::string val;
        pProp->Get(val);
        homeDirY_ = (val == "POS") ? 1 : -1;
    }
    return DEVICE_OK;
};
int XYStage::OnHomeTimeoutY(MM::PropertyBase* pProp, MM::ActionType eAct) {
    if (eAct == MM::BeforeGet)
    {
        pProp->Set((long)homeTimeOutY_);
    }
    else if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);
        homeTimeOutY_ = static_cast<int>(val);

    }
    return DEVICE_OK;

};


int XYStage::OnSetOriginHere(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::AfterSet)
    {
        long val;
        pProp->Get(val);

        

        if (val == 1)
        {
            // Sende Kommando an Hardware oder setze Software-Offset
            //ResetStageOrigin();  // eigene Methode oder Serial-Befehl
        }
    }
    return DEVICE_OK;
}
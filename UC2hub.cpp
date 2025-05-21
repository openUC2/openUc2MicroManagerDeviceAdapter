
#include "openUC2.h"
#include "UC2Hub.h"
#include "ModuleInterface.h"
#include "UC2SerialResponseParser.h"
#include "UC2Tools.h"
#include "UC2DEFAULTS.h"
//#ifdef WIN32
//#include <windows.h>
//#endif

UC2Hub::UC2Hub() : initialized_(false), port_("Undefined"), core_(0), isFastPolling_(false)
{
   // Pre-initialization property: the serial port
   CPropertyAction* pAct = new CPropertyAction(this, &UC2Hub::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

}

UC2Hub::~UC2Hub()
{
   Shutdown();
}

void UC2Hub::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_HubName);
   LogMessage("Getting name of UC2Hub");

}

bool UC2Hub::Busy()
{   
   return false;
}

bool UC2Hub::SupportsDeviceDetection(void)
{
   return true; 
}

MM::DeviceDetectionStatus UC2Hub::DetectDevice(void)
{
   if (port_ == "Undefined" || port_.length() == 0)
      return MM::Misconfigured;

   // Attempt some minimal communication if we like:
   // e.g. see if we can open port, flush, etc.
   // If that works, return MM::CanCommunicate.
   return MM::CanCommunicate;
}

int UC2Hub::DetectInstalledDevices()
{
   ClearInstalledDevices();

   // We can add known sub-devices:
   MM::Device* pDev = 0;

   pDev = CreateDevice(g_XYStageName);
   if (pDev) AddInstalledDevice(pDev);

   pDev = CreateDevice(g_ZStageName);
   if (pDev) AddInstalledDevice(pDev);

   pDev = CreateDevice(g_LaserName1);
   if (pDev) AddInstalledDevice(pDev);
   pDev = CreateDevice(g_LaserName2);
   if (pDev) AddInstalledDevice(pDev);
   pDev = CreateDevice(g_LaserName3);
   if (pDev) AddInstalledDevice(pDev);
   pDev = CreateDevice(g_LaserName4);
   if (pDev) AddInstalledDevice(pDev);

   return DEVICE_OK;
}

int UC2Hub::Initialize()
{
   core_ = GetCoreCallback();
   if (initialized_)
   {
       return DEVICE_OK;
   }
   char hubDevName[MM::MaxStrLength];
   core_->GetLoadedDeviceOfType(this, MM::HubDevice, hubDevName, 0);
   
   if (hubDevName == 0)
   {
       LogMessage("Hub Dev Name is null at 0");
   }
   else
   {
       std::string hubDev = std::string(hubDevName);
   }
   
   if (port_ == "Undefined")
   {
       char portBuffer[256] = { 0 };
       if (GetProperty(MM::g_Keyword_Port, portBuffer) == DEVICE_OK) {
           port_ = std::string(portBuffer);
       }
       else
       {
           return DEVICE_ERR;
       }
   }

   // Purge com port only once
   int ret = PurgeComPort(port_.c_str());
   if (ret != DEVICE_OK)
   {
       LogMessage("Error purging COM port");
       return ret;
   }


   // Example: optional firmware check
   if (!CheckFirmware()) {
       SetErrorText(ERR_INCORRECT_FIRMWARE, "Invalid firmware for UC2 Device");
       initialized_ = false;
       return ERR_INCORRECT_FIRMWARE;
   }
 
   // Mark as initialized
   initialized_ = true;

   return DEVICE_OK;
}

int UC2Hub::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}



int UC2Hub::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet) {
      
      pProp->Set(port_.c_str());
   }
   else if (eAct == MM::AfterSet) {
      if (initialized_) {
        //  Once initialized, port changes are not allowed
           pProp->Set(port_.c_str());
      } else {
          std::string newPort_;
         pProp->Get(newPort_);
         port_ = newPort_;
      }
   }
    return DEVICE_OK;
}



bool UC2Hub::CheckFirmware()
{
    std::string serialReply;
    std::string resp;
    std::string cmd;
    uctool_.cmdFirmWareInfo(cmd);
    int ret = SendJsonCommand(cmd);
    if (ret != DEVICE_OK)
        return false;

    hubReqFasPoll_ = true;
    while (true)
    {
        FastPollSerial();
        // read first sensfull respons form serParser
        resp = GetSerParser().getFirstReady();
        switch (uctool_.parseJSon(resp)) 
        {
            case UNKNOWN:
            {
                serParser_.clear();
                LogMessage("Can't Interprete json Response: " + resp);
                hubReqFasPoll_ = false;
                setFastPolling();
                return false;
            }
            case JSON_CORRUPT: {
                serParser_.clear();
                LogMessage("Can't Interprete json Response: " + resp);
                hubReqFasPoll_ = false;
                setFastPolling();
                return false;
            }
            case FIRMWARE_INVALID:
            {
                LogMessage("Firmware Invalid", true);
                serParser_.clear();
                hubReqFasPoll_ = false;
                setFastPolling();
                return false;
            }
            case FIRMWARE_VALID:
            {
                LogMessage("Firmware Valid", true);
                serParser_.clear();
                hubReqFasPoll_ = false;
                setFastPolling();
                return true;
            }

        //default:                return "INVALID_STATE";
        }
    }
}
   
 


/// <summary>
/// No response expected
/// </summary>
/// <param name="jsonCmd"></param>
/// <returns></returns>
int UC2Hub::SendJsonCommand(const std::string& jsonCmd)
{
   std::lock_guard<std::mutex> guard(ioMutex_); // Thread safety
   
   
   // don't purge com ports, since it will delete read and write buffer. Could loose data
   int ret = SendSerialCommand(port_.c_str(), jsonCmd.c_str(), "\n");
   if (ret != DEVICE_OK)
   {
       LogMessage("Error sending to COM port: " + jsonCmd);
       return ret;
   }

   return DEVICE_OK;
}

/// <summary>
/// Response expected, 
/// </summary>
/// <param name="jsonCmd"></param>
/// <returns></returns>
int UC2Hub::SendJsonCommand(const std::string& jsonCmd, std::string& jsonResponse)
{
    std::lock_guard<std::mutex> guard(ioMutex_); // Thread safety
    std::string msg = "";
    if (isFastPolling_)
    {
        LogMessage("Cannot use SendJSonCommand with expecting return, when in fast polling mode");
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    // don't purge com ports, since it will delete read and write buffer. Could loose data
    int ret = SendSerialCommand(port_.c_str(), jsonCmd.c_str(), "\n");
    if (ret != DEVICE_OK)
    {
        LogMessage("Error sending to COM port: " + jsonCmd);
        return ret;
    }
    
    
        while (msg == "")
        {
            std::string ans;
            ret = GetSerialAnswer(port_.c_str(), "\r", ans);
            if (ret != DEVICE_OK) {
                serParser_.clear();
                return ret;
            }
            serParser_.addInput(ans);
            // Reflect the command in the serial response property
            // TODOret = SetProperty(g_Keyword_Response, ans.c_str());
            msg = serParser_.getFirstReady();
        }
        jsonResponse = msg;
        LogMessage("Message is " + msg, true);
        serParser_.clear();
    
    return DEVICE_OK;
}


int UC2Hub::setFastPolling()
{
    if (hubReqFasPoll_ || xyReqFastPoll_ || zReqFastPoll_)
    {
        if (!isFastPolling_)
        {
            SetProperty("AnswerTimeout", "0");
            isFastPolling_ = true;
        }
        return 0;
    }
    SetProperty("AnswerTimeout", "500.0000");  
    isFastPolling_ = false;
    return 0;
}

void UC2Hub::xyReqiresFastPoll(bool req) { xyReqFastPoll_ = req; }
void UC2Hub::zReqiresFastPoll(bool req) { zReqFastPoll_ = req; }


// only works in fastpolling mode
int UC2Hub::FastPollSerial()
{
    std::lock_guard<std::mutex> guard(ioMutex_); // Thread safety
    setFastPolling();
    if (isFastPolling_)
    {
        unsigned char buffer[256];
        unsigned long bytesRead = 0;

        int ret = ReadFromComPort(port_.c_str(), buffer, sizeof(buffer), bytesRead);
        if (ret == DEVICE_OK && bytesRead > 0) {

            std::string result(reinterpret_cast<const char*>(buffer), bytesRead);
            serParser_.addInput(result);
        }
        return ret;
    }
    else return 0;
}

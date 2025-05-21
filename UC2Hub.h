#ifndef _UC2HUB_H_
#define _UC2HUB_H_


#include "MMDevice.h"
#include "DeviceBase.h"
#include "UC2Tools.h"
#include "UC2SerialResponseParser.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <mutex>


//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////

class UC2Hub : public HubBase<UC2Hub>
{
public:
   UC2Hub();
   ~UC2Hub();

   // MMDevice API:
   int  Initialize();
   int  Shutdown();
   void GetName(char* pszName) const;
   bool Busy();

   // Hub API:
   bool SupportsDeviceDetection(void);
   MM::DeviceDetectionStatus DetectDevice(void);
   int  DetectInstalledDevices();

   // Action handlers
   int  OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);

  

   // Possibly check firmware
   bool CheckFirmware();

   // Provide access to port name
   std::string GetPort() { return port_; }

   
   /// <summary>
   /// Send json - No response expected
   /// </summary>
   /// <param name="jsonCmd"></param>
   /// <returns></returns>
   int SendJsonCommand(const std::string & jsonCmd);
   
   /// <summary>
   /// Sen json command, Response expected
   /// </summary>
   /// <param name="jsonCmd"></param>
   /// <returns></returns>
   int SendJsonCommand(const std::string& jsonCmd, std::string& jsonRepsonse);
   
   /// <summary>
   /// Polls the serial interface. the response will be added to serParser_ -> acces via GetSerParser()
   /// </summary>
   /// <returns></returns>
   int FastPollSerial();

  
   
   // getter for tools object -> not so nice! This can change the UC2Tools object
   UC2Tools& GetUC2Tools() { return uctool_; }
   UC2SerialResponseParser& GetSerParser() { return serParser_; }

   void xyReqiresFastPoll(bool req);
   void zReqiresFastPoll(bool req);

	

private:
   bool        initialized_;
   std::string port_;
   MM::Device* device_;
   MM::Core* core_;
   std::mutex  ioMutex_;
   UC2Tools uctool_;
   UC2SerialResponseParser serParser_;
   bool isFastPolling_;
   bool xyReqFastPoll_;
   bool zReqFastPoll_;
   bool hubReqFasPoll_;
   
   int setFastPolling();
};

#endif // _UC2HUB_H_

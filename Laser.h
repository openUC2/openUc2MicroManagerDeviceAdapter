#ifndef _OPENUC2_LASER_H_
#define _OPENUC2_LASER_H_


#include "DeviceBase.h"
#include "openUC2.h"
#include <string>

//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////


class UC2Hub;

class UC2Laser : public CShutterBase<UC2Laser>
{
public:
   UC2Laser(int id);
   ~UC2Laser();

   // MMDevice API
   int  Initialize();
   int  Shutdown();
   void GetName(char* name) const;
   bool Busy();

   // Shutter API
   int SetOpen(bool open);
   int GetOpen(bool& open);
   int Fire(double /*deltaT*/) { return DEVICE_UNSUPPORTED_COMMAND; }

   // set power via properties
   
   int OnPower(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool     initialized_;
   UC2Hub*  hub_;
   bool     open_;
   int		power_;
   int		id_;

};

#endif

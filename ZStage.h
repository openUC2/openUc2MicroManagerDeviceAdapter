#ifndef _OPENUC2_ZSTAGE_H_
#define _OPENUC2_ZSTAGE_H_


#include "DeviceBase.h"
#include "openUC2.h"
#include <string>

class UC2Hub;

class ZStage : public CStageBase<ZStage>
{
public:
   ZStage();
   ~ZStage();

   // MMDevice API
   int  Initialize();
   int  Shutdown();
   void GetName(char* name) const { CDeviceUtils::CopyLimitedString(name, g_ZStageName); }
   bool Busy();

   // Stage API
   int SetPositionUm(double z);
   int GetPositionUm(double& z);
   int SetRelativePositionUm(double dz);
   // optional step-based overrides if you prefer

private:
   bool     initialized_;
   UC2Hub*  hub_;
   double   stepSizeUm_;
   double   posZ_;
};

#endif

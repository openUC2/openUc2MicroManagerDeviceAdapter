#ifndef _OPENUC2_XYSTAGE_H_
#define _OPENUC2_XYSTAGE_H_


#include "DeviceBase.h"
#include "openUC2.h"
#include <string>

class UC2Hub;

class XYStage : public CXYStageBase<XYStage>
{
public:
   XYStage();
   ~XYStage();

   // MMDevice API
   int  Initialize();
   int  Shutdown();
   void GetName(char* name) const { CDeviceUtils::CopyLimitedString(name, g_XYStageName); }
   bool Busy();

   // XYStage API
   int SetPositionUm(double x, double y);
   int GetPositionUm(double& x, double& y);
   int SetRelativePositionUm(double dx, double dy);

   double GetStepSizeXUm() { return stepSizeUm_; }
   double GetStepSizeYUm() { return stepSizeUm_; }

   // Optional: override methods for step-based position if you prefer
   // int SetPositionSteps(long x, long y); ...
   // etc.

private:
   bool     initialized_;
   UC2Hub*  hub_;
   double   stepSizeUm_;
   double   posX_;
   double   posY_;
};

#endif

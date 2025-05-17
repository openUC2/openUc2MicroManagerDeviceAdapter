#ifndef _OPENUC2_XYSTAGE_H_
#define _OPENUC2_XYSTAGE_H_

#include "DeviceBase.h"
#include "openUC2.h"
#include <string>
#include "UC2DEFAULTS.h"


//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////

class UC2Hub;

class XYStage : public CXYStageBase<XYStage>
{
public:
   XYStage();
   ~XYStage();

   // MMDevice API
   int Initialize() override;
   int Shutdown() override;
   void GetName(char* name) const override;
   bool Busy() override;

   // Required XYStage API overrides
   double GetStepSizeXUm() override { return UC2Defaults::XStepSizeUM; }
   double GetStepSizeYUm() override { return UC2Defaults::YStepSizeUM; }
   int SetPositionSteps(long x, long y) override;
   int GetPositionSteps(long& x, long& y) override;
   int SetRelativePositionSteps(long x, long y) override;
   int Home() override;
   int Stop() override;
   int GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax) override;
   int IsXYStageSequenceable(bool& isSequenceable) const override;


   // Additional methods required by MM::XYStage:
   int GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax) override;
   int SetOrigin() override;

   int OnSpeedX(MM::PropertyBase* pProp, MM::ActionType eAct);
   
   int OnSpeedY(MM::PropertyBase* pProp, MM::ActionType eAct);
   
   int OnHomeSpeedX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomePolarityX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomeDirectionX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomeTimeoutX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomeSpeedY(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomePolarityY(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomeDirectionY(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomeTimeoutY(MM::PropertyBase* pProp, MM::ActionType eAct);


   int OnSetOriginHere(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool     initialized_;
   UC2Hub*  hub_;
   int speedX_;
   int speedY_;
   int homeDirX_;
   int homeDirY_;
   int homeSpeedX_;
   int homeSpeedY_;
   int homePolarityX_;
   int homePolarityY_;
   int homeTimeOutX_;
   int homeTimeOutY_;

   
};

#endif

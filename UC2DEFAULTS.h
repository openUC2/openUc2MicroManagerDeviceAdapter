
#pragma once
#include <string>

#define ERR_INCORRECT_FIRMWARE 10001
#define ERR_GOAL_POSITION_OUT_OF_BOUNDS 10002
#define ERR_SPEED_TOO_LARGE 10003
#define ERR_SPEED_TOO_SMALL 10004
#define ERR_UC2JSON_ERR  10005


//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////

class UC2Defaults {
public:
    
    // GENERAL
    static constexpr int QID = 0;
    //static constexpr unsigned int SerialPollingIntervalMS = 100;

    //////
    ///
    /// NOTE ON AXIS (KC 05.05.2025) -> No endswitch is implemented yet, so AxMinPos and AxMaxPos is used for defining minimum and maximum positions
    /// 
    //////

    // X Axis
    static constexpr int XAxID = 1;                 // Must not exist mutliple times!
    static constexpr int XAxSpeed = 5000;
    static constexpr int XAxIsAccel = 0;
    static constexpr int XAxMinSpeed = 1;
    static constexpr int XAxMaxSpeed = 20000;
    static constexpr int XAxMinPos = -100000;         // Min Pos used if no endswitch is installed (if endswitch, than needs to be implemented here!)
    static constexpr int XAxMaxPos = 1000000;   // Max Pos used if no endswitch is triggered (if endswitch, than needs to be implemented here!)
    static constexpr int XAxHomingTimeOut = 1000;
    static constexpr int XAxHomingEndStopPolarity = 1;
    static constexpr int XAxHomingSpeed = 1000;
    static constexpr int XAxHomingDirection = 1; // neg. or >=0
    static constexpr double XStepSizeUM = 0.3125;  // 1mm lift is 3200 steps -> 1/3.2 = 0.3125 µm / step

    // Y Axis
    static constexpr int YAxID = 2;// Must not exist mutliple times!
    static constexpr int YAxSpeed = 5000;
    static constexpr int YAxIsAccel = 0;
    static constexpr int YAxMinSpeed = 1;
    static constexpr int YAxMaxSpeed = 20000;
    static constexpr int YAxMinPos = -100000;         // Min Pos used if no endswitch is installed (if endswitch, than needs to be implemented here!)
    static constexpr int YAxMaxPos = 1000000;   // Max Pos used if no endswitch is triggered (if endswitch, than needs to be implemented here!)
    static constexpr int YAxHomingTimeOut = 1000;
    static constexpr int YAxHomingEndStopPolarity = 1;
    static constexpr int YAxHomingSpeed = 1000;
    static constexpr int YAxHomingDirection = 1; // neg. or >=0
    static constexpr double YStepSizeUM = 0.3125;  // 1mm lift is 3200 steps -> 1/3.2 = 0.3125 µm / step

    // Z Axis
    static constexpr int ZAxID = 3;// Must not exist mutliple times!
    static constexpr int ZAxSpeed = 5000;
    static constexpr int ZAxIsAccel = 0;
    static constexpr int ZAxMinSpeed = 1;
    static constexpr int ZAxMaxSpeed = 20000;
    static constexpr int ZAxMinPos = 0;         // Min Pos used if no endswitch is installed (if endswitch, than needs to be implemented here!)
    static constexpr int ZAxMaxPos = 1000000;   // Max Pos used if no endswitch is triggered (if endswitch, than needs to be implemented here!)
    static constexpr int ZAxHomingTimeOut = 1000;
    static constexpr int ZAxHomingEndStopPolarity = 1;
    static constexpr int ZAxHomingSpeed = 1000;
    static constexpr int ZAxHomingDirection = 1; // neg. or >=0
    static constexpr double ZStepSizeUM = 0.3125;  // 1mm lift is 3200 steps -> 1/3.2 = 0.3125 µm / step

    // UNDEF Axis
    
    static constexpr int dAxSpeed = 5000;
    static constexpr int dAxIsAccel = 0;
    static constexpr int dAxMinSpeed = 1;
    static constexpr int dAxMaxSpeed = 20000;
    static constexpr int dAxMinPos = 0;         // Min Pos used if no endswitch is installed (if endswitch, than needs to be implemented here!)
    static constexpr int dAxMaxPos = 1;   // Max Pos used if no endswitch is triggered (if endswitch, than needs to be implemented here!)
    static constexpr int dAxHomingTimeOut = 0;
    static constexpr int dAxHomingEndStopPolarity = 1;
    static constexpr int dAxHomingSpeed = 1000;
    static constexpr int dAxHomingDirection = 1; // neg. or >=0
    static constexpr double dStepSizeUM = 0.3125;  // 1mm lift is 3200 steps -> 1/3.2 = 0.3125 µm / step




    // State info for confirming firmware -> currently only checking for UC2_Feather
    static constexpr const char* StateIdentName = "UC2_Feather";
    //static constexpr const char* StateIdentId = "V2.0";
    //static constexpr const char* StateIdentDate = "Mar  9 202520:25:00";
    //static constexpr const char* StateIdentAuthor = "BD";
    //static constexpr const char* StateIdentName2 = "uc2-esp";
    //static constexpr int StateConfigSet = 0;
    //static constexpr const char* StatePindef = "UC2_3";  //;
    //static constexpr int StateI2CSlave = 0;

    // Laser updates
    static constexpr int laserId_1 = 0;
    static constexpr int laserId_2 = 1;  // set ID to -1 to deactivate laser
    static constexpr int laserId_3 = 2;
    static constexpr int laserId_4 = 3;
    static constexpr const char* laserName1 = "UC2Laser1";
    static constexpr const char* laserName2 = "UC2Laser2";
    static constexpr const char* laserName3 = "UC2Laser3";
    static constexpr const char* laserName4 = "UC2Laser4";
    static constexpr int maxLaserPower = 1024;
    static constexpr int minLaserPower = 0;

};



#pragma once
#include <string>


//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////

/// <summary>
/// Struct for managing steppers
/// </summary>
struct Stepper {
    const int stepperid = 0;
    long position = 0;
    int trigOff = 0;
    int trigPer = -1;
    int trigPin = -1;
    
    bool isDualAxisZ = false;  
    bool isActivated = false;  
    const int isAccel;
    const int maxSpeed;
    const int minSpeed;
    const long maxPos;
    const long minPos;
    const int homingDirection;
    const int homingTimeOut;
    const int homingSpeed;
    const int homingEndStopPolarity;
    const double stepSizeUM;
    // This stuff is from the UC2 Rest API but currently not working, so wie keep it but comment it out
    //bool isDone = false;  // is Set based on what is read from Rest API
    //bool isStop = false;  // is Set based on what is read from Rest API


    // these are specific flags for the MM control (and the constructor)
    bool isEndSwitchTriggered = false;
    bool isMoving = false;
    Stepper(int id, int isAcc, int maxSpeed, int minSpeed, long maxPos, long minPos, int homingDirection, int homingTimeOut, int homingSpeed, int homingEndStopPol, double stepSizeUm) :stepperid(id),
        isAccel(isAcc),
        maxSpeed(maxSpeed),
        minSpeed(minSpeed),
        maxPos(maxPos),
        minPos(minPos),
        homingDirection(homingDirection),
        homingTimeOut(homingTimeOut),
        homingSpeed(homingSpeed),
        homingEndStopPolarity(homingEndStopPol),
        stepSizeUM(stepSizeUm)
    
    {}
};

/// <summary>
/// Struct for managing lasers - Not used currently, but can be used in future to manage properties such as e.g. laser pin
/// </summary>
struct Laser {
    const int laserid = 0;
    int Val = 0;
    int Pin = 0; 
    Laser(int id) :laserid(id)
    {}
};




enum returnState
{
    UNKNOWN,
    EMPTY,
    WRONG_QID,
    JSON_CORRUPT,
    FIRMWARE_INVALID,
    FIRMWARE_VALID,
    LASER,
    MOTOR,
    HOME,
    MESSAGE_SUCCESS
};

class UC2Tools {
public:
    UC2Tools();
    /// <summary>
    /// Command for homing one axis
    /// </summary>
    /// <param name="stepperId"></param>
    /// <param name="timeOut"></param>
    /// <param name="speed"></param>
    /// <param name="direction"></param>
    /// <param name="endstoppolarity"></param>
    /// <returns></returns>
    int cmdHome(int stepperId, int timeOut, int speed, int direction, int polarity, std::string& cmd) const;

    /// <summary>
    /// Command for setting a laser
    /// </summary>
    /// <param name="laserId">Id </param>
    /// <param name="value">should be between >=0 and <1024. If == 0 -> laser is off</param>
    /// <returns></returns>
    int cmdSetLaser(int laserId, int value, std::string& cmd) const;

    /// <summary>
    /// Stops motoer
    /// </summary>
    /// <param name="stepperId"></param>
    /// <returns></returns>
    int cmdStopMotor(int stepperId, std::string& cmd) const;

    /// <summary>
    /// Move motor relative 
    /// </summary>
    /// <param name="stepperId"></param>
    /// <param name="position"></param>
    /// <param name="speed"></param>
    /// <returns>0 or ERR_SPEED_TOO_LARGE or ERR_SPEED_TOO_SMALL or ERR_GOAL_POSITION_OUT_OF_BOUNDS</returns>
    int cmdMoveMotorRel(int stepperId, long position, int speed, std::string& cmd);

    /// <summary>
    /// Move motor absolute
    /// </summary>
    /// <param name="stepperId"></param>
    /// <param name="position"></param>
    /// <param name="speed"></param>
    /// <returns>0 or ERR_SPEED_TOO_LARGE or ERR_SPEED_TOO_SMALL or ERR_GOAL_POSITION_OUT_OF_BOUNDS</returns>
    int cmdMoveMotorAbs(int stepperId, long position, int speed, std::string& cmd);

    /// <summary>
    /// Get command for getting all stepper positions
    /// </summary>
    /// <param name="stepperId"></param>
    /// <returns></returns>
    int cmdGetPos(std::string& cmd) const;

    /// <summary>
    ///  command for all motor informaiton
    /// </summary>
    /// <returns></returns>
    int cmdMotorInfo(std::string& cmd) const;

    /// <summary>
    ///  command for Firmware information -> returns state
    /// </summary>
    /// <returns></returns>
    int cmdFirmWareInfo(std::string& cmd) const;

    /// <summary>
    /// Moves forever neg
    /// </summary>
    /// <param name="stepperId"></param>
    /// <param name="speed"></param>
    /// <param name="cmd"></param>
    /// <returns>0 or ERR_SPEED_TOO_LARGE</returns>
    int cmdMoveMotorInfNeg(int stepperId, int speed, std::string& cmd);

    /// <summary>
    /// Moves forever pos
    /// </summary>
    /// <param name="stepperId"></param>
    /// <param name="speed"></param>
    /// <param name="cmd"></param>
    /// <returns>0 or ERR_SPEED_TOO_LARGE</returns>
    int cmdMoveMotorInfPos(int stepperId, int speed, std::string& cmd);

    /// <summary>
    /// analyzses the json string and returns the type of UC2 JSon Response. Creates state Struct, Laser Vector, Motor Vector for respective home typye, sets isSuccessFlag in case of MESSAGE_SUCCESS
    /// </summary>
    /// <param name="JsonString"></param>
    /// <returns></returns>
    returnState parseJSon(const std::string& JsonString);

    static std::string returnStateToString(returnState state);

    // Getter/Setter
    const Stepper& GetStepper(int idx) const { return steppers_[idx]; }
    const Laser& GetLaser(int idx) const { return lasers_[idx]; }
    const long GetPos(int idx) const { return steppers_[idx].position; }
    const bool IsSuccess() { return isSuccess_; }

private:
    std::vector<Stepper> steppers_;
    std::vector<Laser> lasers_;

    static Stepper stepperFactory(int id);

    bool isSuccess_;
    std::string json_;
    bool extractStringValue(const std::string& text, const std::string& key, std::string& value);
    bool extractIntValue(const std::string& text, const std::string& key, int& value);
    bool extractLongValue(const std::string& text, const std::string& key, long& value);
    bool extractFirstKey(const std::string& text, std::string& key);
    bool extractElementList(const std::string& text, std::vector<std::string>& elements);
    /// <summary>
    /// returns true if updated
    /// </summary>
    /// <param name="jsonString"></param>
    /// <returns></returns>
    bool updateStepper(const std::string& jsonString);

    /// <summary>
    /// returns true if updated
    /// </summary>
    /// <param name="jsonString"></param>
    /// <returns></returns>
    bool updateLaser(const std::string& jsonString);

    /// <summary>
    /// returns true if state is valid (according to what was set in defaults
    /// </summary>
    /// <param name="jsonString"></param>
    /// <returns></returns>
    bool updateState(const std::string& jsonString);
    
    void setMoving(int id) { steppers_[id].isMoving = true; };

};
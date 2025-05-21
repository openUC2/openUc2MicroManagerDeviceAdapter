#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "UC2Tools.h"
#include "UC2Defaults.h"






    // Indidcate succes in send command:   {"qid":0,"success":0}
    // Indicate Motion Done: 
    // {"steppers":[{"stepperid":2,"position":0,"isDone":0}],"qid":0}
    // Indicate Get Position 
    // {"motor":{"steppers":[{"stepperid":0,"position":3000},{"stepperid":1,"position":-5335},{"stepperid":2,"position":0},{"stepperid":3,"position":0}]},"qid":0}
    // Indicate Get all Motor info
    // {"motor":{"steppers":[   {"stepperid":0,"position":3000,"isActivated":1,"trigOff":0,"trigPer":-1,"trigPin":-1,"isStop":0,"isDualAxisZ":0},        {"stepperid":1,"position":-5335,"isActivated":1,"trigOff":0,"trigPer":-1,"trigPin":-1,"isStop":0,"isDualAxisZ":0},        {"stepperid":2,"position":0,"isActivated":1,"trigOff":0,"trigPer":-1,"trigPin":-1,"isStop":0,"isDualAxisZ":0}, {"stepperid":3,"position":0,"isActivated":1,"trigOff":0,"trigPer":-1,"trigPin":-1,"isStop":0,"isDualAxisZ":0}]},"qid":0}    
    // Indicate laser get 
    // {"laser":{"LASER1pin":12,"LASER2pin":4,"LASER3pin":2,"LASER1val":0,"LASER2val":22,"LASER3val":0}}
    // indicate state 
    // {"state":{"identifier_name":"UC2_Feather","identifier_id":"V2.0","identifier_date":"Mar  9 202520:25:00","identifier_author":"BD","IDENTIFIER_NAME":"uc2-esp","configIsSet":0,"pindef":"UC2_3","I2C_SLAVE":0},"qid":0}


    // repsonses after Homing
    //    {"home":{"steppers":{"axis":2,"pos":0,"isDone":1}},"qid":0,"qid":0}
    //  {"steppers":[{"stepperid":2,"position":0,"isDone":1}],"qid":-1}

    // command for laser on / off {"task": "/laser_act", "LASERid": 0, "LASERval": 0}  val > 0 laser on <1024
    // command for home stepper  {"task":"/home_act","home":{"steppers":[{"stepperid":2,"timeout":20000,"speed":15000,"direction":-1,"endstoppolarity":0}]}}
    // command for stop stepper {"task":"/motor_act","motor":{"steppers":[{"stepperid":1,"isstop":1}]}}
    // command for get position {"task":"/motor_get", "position" : 1 }
    // command for all motor info {"task":"/motor_get"}
    // command for getting state info (for firmware) {"task":"/state_get", "state" : 1 }

/// <summary>
/// If more than 4 lasers or more than 4 steppers are used: that has to be set up in here
/// </summary>
UC2Tools::UC2Tools() :
    steppers_{ stepperFactory(0), stepperFactory(1), stepperFactory(2), stepperFactory(3) },  // note: Id  is always postiion in list!
    lasers_{ Laser(0),Laser(1) ,Laser(2) ,Laser(3) },  // up to 4 lasers Note: Not used now, but prepared for future saving properties
    isSuccess_(false)
{}

returnState UC2Tools::parseJSon(const std::string& json) {

    if (json == "") return EMPTY;
    // state, laser steppers home motor
    json_ = json;

    std::string sVal_ = "";
    std::string firstKey = "";
    int iVal_ = -1;

    // no key for indication
    if (!extractFirstKey(json_, firstKey)) return JSON_CORRUPT;


    // COMMANDS WITHOUT QID FLAG

    // laser update
    if (firstKey == "laser")
    {
        updateLaser(json_);
        return LASER;
    }

    // COMMANDS WITH QUID FLAG
    // no qid flag -> unkown situation
    if (!extractIntValue(json_, "qid", iVal_)) return UNKNOWN;

    // qid != 0
    if (iVal_ != 0) return WRONG_QID;

    // success indicator
    iVal_ = -1;
    if (extractIntValue(json_, "success", iVal_))
    {
        isSuccess_ = (iVal_ == 0);
        return MESSAGE_SUCCESS;
    }

    //  position info or all information or stepper is done
    if (firstKey == "motor" || firstKey == "steppers")
    {
        std::vector<std::string> stepperEls_;
        if (!extractElementList(json_, stepperEls_)) return JSON_CORRUPT;
        // update all steppers
        for (const std::string& s : stepperEls_)
        {
            updateStepper(s);
        }
        return MOTOR;
    }

    // state and check if firmware is correct

    if (firstKey == "state")
    {
        if (updateState(json_)) return FIRMWARE_VALID;
        else return FIRMWARE_INVALID;
    }

    return UNKNOWN;
}

/// <summary>
/// Command for homing one axis
/// </summary>
/// <param name="stepperId"></param>
/// <param name="timeOut"></param>
/// <param name="speed"></param>
/// <param name="direction"> >=0 -> 1; <0 -> -1</param>
/// <param name="endstoppolarity"></param>
/// <returns>0,ERR_SPEED_TOO_LARGE,ERR_SPEED_TOO_SMALL</returns>
int UC2Tools::cmdHome(int stepperId, int timeOut, int speed, int direction, int polarity, std::string& cmd) const
{
    int dir;
    if (direction) dir = -1;
    else dir = 1;
    
    std::ostringstream oss;
    if (steppers_[stepperId].homingSpeed > steppers_[stepperId].maxSpeed)
    {
        oss << "HOMING ERROR AXIS " << stepperId << " HOMING SPEED TOO LARGE";
        cmd = oss.str();
        return ERR_SPEED_TOO_LARGE;
    }
    if (steppers_[stepperId].homingSpeed < steppers_[stepperId].minSpeed)
    {
        oss << "HOMING ERROR AXIS " << stepperId << " HOMING SPEED TOO SMALL";
        cmd = oss.str();
        return ERR_SPEED_TOO_SMALL;
    }
    oss << "{\"task\":\"/home_act\",\"home\":{\"steppers\":[{";
    oss << "\"stepperid\":" << stepperId << ",";
    oss << "\"timeout\":" << timeOut << ",";
    oss << "\"speed\":" << speed << ",";
    oss << "\"direction\":" << dir << ",";
    oss << "\"endstoppolarity\":" << polarity << "}]}}";
    cmd = oss.str();
    return 0;
}

int UC2Tools::cmdFirmWareInfo(std::string& cmd) const {

    std::ostringstream oss;
    oss << "{\"task\":\"/state_get\", \"state\" : 1}";
    cmd = oss.str();
    return 0;
}


/// <summary>
/// Command for setting a laser
/// </summary>
/// <param name="laserId">Id </param>
/// <param name="value">should be between 0 and 1024. If == 0 : laser is off</param>
/// <returns></returns>
int UC2Tools::cmdSetLaser(int laserId, int value, std::string& cmd) const {
    if (value < 0) value = 0;
    if (value >= 1024) value = 1024;
    std::ostringstream oss;
    oss << "{\"task\":\"/laser_act\",\"LASERid\": " << laserId << ", \"LASERval\": " << value << "}";
    cmd = oss.str();
    return 0;
}
/// <summary>
/// Moves motor to abs position
/// </summary>
/// <param name="stepperId"></param>
/// <returns>0,ERR_SPEED_TOO_LARGE,ERR_SPEED_TOO_SMALL, ERR_GOAL_POSITION_OUT_OF_BOUNDS</returns>
int UC2Tools::cmdMoveMotorAbs(int stepperId, long position, int speed, std::string& cmd) {
    std::ostringstream oss;
    speed = std::abs(speed);
    
    // DBG
    //cmd = "Stepper ID = " + std::to_string(stepperId) + " ----  is at position : " + std::to_string(steppers_[stepperId].position) + " MOVING? : "+std::to_string(steppers_[stepperId].isMoving);
    //return ERR_UC2JSON_ERR;


    if (speed > steppers_[stepperId].maxSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO LARGE";
        cmd = oss.str();
        return ERR_SPEED_TOO_LARGE;
    }
    if (speed < steppers_[stepperId].minSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO SMALL";
        cmd = oss.str();
        return ERR_SPEED_TOO_SMALL;
    }
    if (position < steppers_[stepperId].minPos)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " GOAL POSITION TOO SMALL (Min is "<< steppers_[stepperId].minPos<<" but goal is "<<position;
        cmd = oss.str();
        return ERR_GOAL_POSITION_OUT_OF_BOUNDS;
    }
    if (position > steppers_[stepperId].maxPos)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " GOAL POSITION TOO LARGE (Max is " << steppers_[stepperId].maxPos << " but goal is " << position;
        cmd = oss.str();
        return ERR_GOAL_POSITION_OUT_OF_BOUNDS;
    }
    
    int isAc = 0;
    if (steppers_[stepperId].isAccel != 0) isAc = 1;
    
    steppers_[stepperId].isMoving = true;
    oss << "{\"task\":\"/motor_act\",\"motor\":{\"steppers\":[{\"stepperid\":" << stepperId << ", \"position\":"<<position<<", \"speed\":"<<speed<<", \"isabs\":1, \"isaccel\" : "<<isAc<<"}] }}";
    cmd = oss.str();
    return 0;
}

/// <summary>
/// Moves motor to rel position
/// </summary>
/// <param name="stepperId"></param>
/// <returns>0,ERR_SPEED_TOO_LARGE,ERR_SPEED_TOO_SMALL, ERR_GOAL_POSITION_OUT_OF_BOUNDS</returns>
int UC2Tools::cmdMoveMotorRel(int stepperId, long position, int speed, std::string& cmd)  {
    std::ostringstream oss;
    speed = std::abs(speed);
    if (speed > steppers_[stepperId].maxSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO LARGE";
        cmd = oss.str();
        return ERR_SPEED_TOO_LARGE;
    }
    if (speed < steppers_[stepperId].minSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO SMALL";
        cmd = oss.str();
        return ERR_SPEED_TOO_SMALL;
    }
    long curPos_ = steppers_[stepperId].position;
    if (curPos_-position < steppers_[stepperId].minPos)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " GOAL POSITION TOO SMALL (Min is " << steppers_[stepperId].minPos << " but goal is " << curPos_-position;
        cmd = oss.str();
        return ERR_GOAL_POSITION_OUT_OF_BOUNDS;
    }
    if (curPos_+position > steppers_[stepperId].maxPos)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " GOAL POSITION TOO LARGE (Max is " << steppers_[stepperId].maxPos << " but goal is " << curPos_+position;
        cmd = oss.str();
        return ERR_GOAL_POSITION_OUT_OF_BOUNDS;
    }
    int isAc = 0;
    if (steppers_[stepperId].isAccel != 0) isAc = 1;
    steppers_[stepperId].isMoving = true;
    oss << "{\"task\":\"/motor_act\",\"motor\":{\"steppers\":[{\"stepperid\":" << stepperId << ", \"position\":" << position << ", \"speed\":" << speed << ", \"isabs\":0, \"isaccel\" : " << isAc << "}] }}";
    cmd = oss.str();
    return 0;
}
/// <summary>
/// moves infinity negative
/// </summary>
/// <param name="stepperId"></param>
/// <param name="speed"></param>
/// <param name="cmd"></param>
/// <returns>0,ERR_SPEED_TOO_LARGE,ERR_SPEED_TOO_SMALL</returns>
int UC2Tools::cmdMoveMotorInfNeg(int stepperId, int speed, std::string& cmd)
{
    std::ostringstream oss;
    if (speed > steppers_[stepperId].maxSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO LARGE";
        cmd = oss.str();
        return ERR_SPEED_TOO_LARGE;
    }
    if (speed < steppers_[stepperId].minSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO SMALL";
        cmd = oss.str();
        return ERR_SPEED_TOO_SMALL;
    }
    int isAc = 0;
    if (steppers_[stepperId].isAccel != 0) isAc = 1;
    steppers_[stepperId].isMoving = true;
    oss << "{\"task\":\"/motor_act\",\"motor\":{\"steppers\":[{\"stepperid\":" << stepperId << ", \"isforever\":1, \"speed\":" << -1*speed << ", \"isabs\":0, \"isaccel\" : " << isAc << "}] }}";
    cmd = oss.str();
    return 0;
}
/// <summary>
/// moves infinity negative
/// </summary>
/// <param name="stepperId"></param>
/// <param name="speed"></param>
/// <param name="cmd"></param>
/// <returns>0,ERR_SPEED_TOO_LARGE,ERR_SPEED_TOO_SMALL</returns>
int UC2Tools::cmdMoveMotorInfPos(int stepperId, int speed, std::string& cmd)
{
    std::ostringstream oss;
    if (speed > steppers_[stepperId].maxSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO LARGE";
        cmd = oss.str();
        return ERR_SPEED_TOO_LARGE;
    }
    if (speed < steppers_[stepperId].minSpeed)
    {
        oss << "MOTION ERROR AXIS " << stepperId << " SPEED TOO SMALL";
        cmd = oss.str();
        return ERR_SPEED_TOO_SMALL;
    }
    int isAc = 0;
    if (steppers_[stepperId].isAccel != 0) isAc = 1;
    steppers_[stepperId].isMoving = true;
    oss << "{\"task\":\"/motor_act\",\"motor\":{\"steppers\":[{\"stepperid\":" << stepperId << ", \"isforever\":1, \"speed\":" << speed << ", \"isabs\":0, \"isaccel\" : " << isAc << "}] }}";
    cmd = oss.str();
    return 0;
}

/// <summary>
/// Stops motor
/// </summary>
/// <param name="stepperId"></param>
/// <returns></returns>
int UC2Tools::cmdStopMotor(int stepperId, std::string& cmd) const {
    std::ostringstream oss;
    oss << "{\"task\":\"/motor_act\",\"motor\":{\"steppers\":[{\"stepperid\":" << stepperId << ", \"isstop\":1 }] }}";
    cmd = oss.str();
    return 0;
}

/// <summary>
/// Get command for getting all stepper positions
/// </summary>
/// <param name="stepperId"></param>
/// <returns></returns>
int UC2Tools::cmdGetPos(std::string& cmd) const{
    std::ostringstream oss;
    oss << "{\"task\":\"/motor_get\", \"position\" : 1 }";
    cmd = oss.str();
    return 0;
}

/// <summary>
///  command for all motor informaiton
/// </summary>
/// <returns></returns>
int UC2Tools::cmdMotorInfo(std::string& cmd)  const {
    std::ostringstream oss;
    oss << "{\"task\":\"/motor_get\"}";
    cmd = oss.str();
    return 0;
}



/// <summary>
/// Extracts the string value of "Key". returns true if key was found, else false
/// </summary>
/// <param name="text">text to look in </param>
/// <param name="key">key to look for</param>
/// <param name="value"></param>
/// <returns></returns>
bool UC2Tools::extractStringValue(const std::string& text, const std::string& key, std::string& value) {
    size_t pos = text.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = json_.find(':', pos);
    pos = json_.find_first_of("\"", pos);
    size_t end = json_.find_first_of("\"", pos + 1);
    if (pos == std::string::npos || end == std::string::npos) return false;
    value = json_.substr(pos + 1, end - pos - 1);
    return true;
}


std::string UC2Tools::returnStateToString(returnState state) {
    switch (state) {
    case UNKNOWN:           return "UNKNOWN";
    case WRONG_QID:         return "WRONG_QID";
    case JSON_CORRUPT:      return "JSON_CORRUPT";
    case FIRMWARE_INVALID:  return "FIRMWARE_INVALID";
    case FIRMWARE_VALID:    return "FIRMWARE_VALID";
    case LASER:             return "LASER";
    case MOTOR:             return "MOTOR";
    case HOME:              return "HOME";
    case MESSAGE_SUCCESS:   return "MESSAGE_SUCCESS";
    case EMPTY:             return "EMPTY";
    default:                return "INVALID_STATE";
    }
}

/// <summary>
/// extracts an int value. If not found  returns false. else true
/// </summary>
/// <param name="text">Text to search in </param>
/// <param name="key">Key of int value</param>
/// <param name="value">Value to be extracted</param>
/// <returns></returns>
bool UC2Tools::extractIntValue(const std::string& text, const std::string& key, int& value) {
    size_t pos = text.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = text.find(':', pos);
    if (pos == std::string::npos) return false;

    size_t start = text.find_first_of("-0123456789", pos);
    size_t end = text.find_first_not_of("-0123456789", start);
    if (start == std::string::npos) return false;
    std::string numberStr = text.substr(start, end - start);
    value = std::stoi(numberStr);
    return true;
}

/// <summary>
/// extracts an int value. If not found  returns false. else true
/// </summary>
/// <param name="text">Text to search in </param>
/// <param name="key">Key of int value</param>
/// <param name="value">Value to be extracted</param>
/// <returns></returns>
bool UC2Tools::extractLongValue(const std::string& text, const std::string& key, long& value) {
    size_t pos = text.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = text.find(':', pos);
    if (pos == std::string::npos) return false;

    size_t start = text.find_first_of("-0123456789", pos);
    size_t end = text.find_first_not_of("-0123456789", start);
    if (start == std::string::npos) return false;
    std::string numberStr = text.substr(start, end - start);
    value = std::stol(numberStr);
    return true;
}

/// <summary>
/// Find the first key of the text
/// </summary>
/// <param name="text"></param>
/// <param name="key"></param>
/// <returns></returns>
bool UC2Tools::extractFirstKey(const std::string& text, std::string& key)
{

    size_t pos = text.find("\"");
    if (pos == std::string::npos) return false;
    if (text.length() == pos) return false;
    size_t start = pos + 1;
    size_t end = text.find("\"", start);
    key = text.substr(start, end - start);
    return true;
}

/// <summary>
/// Extracts string elements in [] and save them in elements. if no [] returns false -> not no stacked list!
/// </summary>
/// <param name="text"></param>
/// <param name="elements"></param>
/// <returns></returns>
bool UC2Tools::extractElementList(const std::string& text, std::vector<std::string>& elements)
{

    size_t start = text.find('[');
    size_t end = text.find(']', start);

    if (start == std::string::npos || end == std::string::npos || end <= start + 1)   return false;
    std::string listPart = text.substr(start + 1, end - start - 1);
    elements.clear();
    size_t pos = 0;
    size_t posE = 0;

    while (true)
    {
        pos = listPart.find("{", pos);
        if (pos == std::string::npos) break;
        if (pos == listPart.length()) break;
        posE = listPart.find("}", pos);
        if (posE == std::string::npos) break;
        elements.push_back(listPart.substr(pos, posE - pos));
        pos = posE;
    }
    return true;
}

/// <summary>
/// Updates a stepper from vector stepper with all that was found in the json string
/// returns false if no stepper was updated
/// </summary>
/// <param name="jsonString"></param>
/// <returns></returns>
bool UC2Tools::updateStepper(const std::string& jsonString)
{
    //{"stepperid":0, "position" : 3000, "isActivated" : 1, "trigOff" : 0, "trigPer" : -1, "trigPin" : -1, "isStop" : 0, "isDualAxisZ" : 0},
    //{"stepperid":2,"position":0,"isDone":0}]
    int id_ = -1;
    int ival_ = -1;
    long lval_ = -1;
    // no Id return false or not 0,1,2,3
    if (!extractIntValue(jsonString, "stepperid", id_)) return false;
    if (id_ < 0 || id_>3) return false;

    if (extractLongValue(jsonString, "position", lval_)) steppers_[id_].position = lval_;
    if (extractIntValue(jsonString, "isActivated", ival_)) steppers_[id_].isActivated = ival_ == 1;
    if (extractIntValue(jsonString, "trigOff", ival_)) steppers_[id_].trigOff = ival_;
    if (extractIntValue(jsonString, "trigPer", ival_)) steppers_[id_].trigPer = ival_;
    if (extractIntValue(jsonString, "trigPin", ival_)) steppers_[id_].trigPin = ival_;
    
    if (extractIntValue(jsonString, "isDualAxisZ", ival_)) steppers_[id_].isDualAxisZ = ival_ == 1;
    if (extractIntValue(jsonString, "isDone", ival_)) steppers_[id_].isMoving = false;
    


    //if (extractIntValue(jsonString, "isStop", ival_)) steppers_[id_].isStop = ival_ == 1;
    //steppers_[id_].hasIsDoneFlag = extractIntValue(jsonString, "isDone", ival_);
    //if (steppers_[id_].hasIsDoneFlag) steppers_[id_].isDone = ival_ == 1;

    // TODO: as soon as end swithc triggering is available to be impelemtned here!
    return true;
}

/// <summary>
/// Updates a stepper from vector stepper with all that was found in the json string
/// returns false if no stepper was updated
/// </summary>
/// <param name="jsonString"></param>
/// <returns></returns>
bool UC2Tools::updateLaser(const std::string& jsonString)
{
    // Possible values sets (10.05.25)
    // Indicate laser get {"laser":{"LASER1pin":12,"LASER2pin":4,"LASER3pin":2,"LASER1val":0,"LASER2val":22,"LASER3val":0}}

    int ival_ = -1;
    if (extractIntValue(jsonString, "LASER1pin", ival_)) lasers_[0].Pin = ival_;
    if (extractIntValue(jsonString, "LASER2pin", ival_)) lasers_[1].Pin = ival_;
    if (extractIntValue(jsonString, "LASER3pin", ival_)) lasers_[2].Pin = ival_;
    if (extractIntValue(jsonString, "LASER4pin", ival_)) lasers_[3].Pin = ival_;
    if (extractIntValue(jsonString, "LASER1val", ival_)) lasers_[0].Val = ival_;
    if (extractIntValue(jsonString, "LASER2val", ival_)) lasers_[1].Val = ival_;
    if (extractIntValue(jsonString, "LASER3val", ival_)) lasers_[2].Val = ival_;
    if (extractIntValue(jsonString, "LASER4val", ival_)) lasers_[3].Val = ival_;
    if (ival_ == -1) return false;
    return true;
}


/// <summary>
/// returns true if state is valid (according to what was set in defaults)
/// </summary>
/// <param name="jsonString"></param>
/// <returns></returns>
bool UC2Tools::updateState(const std::string& jsonString)
{
    // Possible set {"state":{"identifier_name":"UC2_Feather","identifier_id":"V2.0","identifier_date":"Mar  9 202520:25:00","identifier_author":"BD","IDENTIFIER_NAME":"uc2-esp","configIsSet":0,"pindef":"UC2_3","I2C_SLAVE":0},"qid":0}
    std::string sVal_ = "";
    int iVal_ = -1;
    if (!extractStringValue(jsonString, "identifier_name", sVal_)) return false;
    if (sVal_ != UC2Defaults::StateIdentName) return false;

    // Uncomment for making Firmware definition more critical
    //if (!extractStringValue(jsonString, "identifier_id", sVal_)) return false;
    //if (sVal_ != UC2Defaults::StateIdentId) return false;
    //if (!extractStringValue(jsonString, "identifier_date", sVal_)) return false;
    //if (sVal_ != UC2Defaults::StateIdentDate) return false;
    //if (!extractStringValue(jsonString, "identifier_author", sVal_)) return false;
    //if (sVal_ != UC2Defaults::StateIdentAuthor) return false;
    //if (!extractStringValue(jsonString, "IDENTIFIER_NAME", sVal_)) return false;
    //if (sVal_ != UC2Defaults::StateIdentName2) return false;
    //if (!extractIntValue(jsonString, "configIsSet", iVal_)) return false;
    //if (iVal_ != UC2Defaults::StateConfigSet) return false;
    //if (!extractStringValue(jsonString, "pindef", sVal_)) return false;
    //if (sVal_ != UC2Defaults::StatePindef) return false;
    //if (!extractIntValue(jsonString, "I2C_SLAVE", iVal_)) return false;
    //if (iVal_ != UC2Defaults::StateI2CSlave) return false;
    return true;
}


/// <summary>
/// Factory class for stepper
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
Stepper UC2Tools::stepperFactory(int id)
{
    //Stepper(int id, int isAcc, int maxSpeed, int minSpeed, long maxPos, long minPos, int homingDirection, int homingTimeOut)
    switch (id)
    {
        case UC2Defaults::XAxID: return { id, UC2Defaults::XAxIsAccel, UC2Defaults::XAxMaxSpeed, UC2Defaults::XAxMinSpeed, UC2Defaults::XAxMaxPos, UC2Defaults::XAxMinPos, UC2Defaults::XAxHomingDirection, UC2Defaults::XAxHomingTimeOut, UC2Defaults::XAxHomingSpeed, UC2Defaults::XAxHomingEndStopPolarity, UC2Defaults::XStepSizeUM };
        case UC2Defaults::YAxID: return { id, UC2Defaults::YAxIsAccel, UC2Defaults::YAxMaxSpeed, UC2Defaults::YAxMinSpeed, UC2Defaults::YAxMaxPos, UC2Defaults::YAxMinPos, UC2Defaults::YAxHomingDirection, UC2Defaults::YAxHomingTimeOut, UC2Defaults::YAxHomingSpeed, UC2Defaults::YAxHomingEndStopPolarity, UC2Defaults::YStepSizeUM };
        case UC2Defaults::ZAxID: return { id, UC2Defaults::ZAxIsAccel, UC2Defaults::ZAxMaxSpeed, UC2Defaults::ZAxMinSpeed, UC2Defaults::ZAxMaxPos, UC2Defaults::ZAxMinPos, UC2Defaults::ZAxHomingDirection, UC2Defaults::ZAxHomingTimeOut, UC2Defaults::ZAxHomingSpeed, UC2Defaults::ZAxHomingEndStopPolarity, UC2Defaults::ZStepSizeUM };
        default: return { id, UC2Defaults::dAxIsAccel, UC2Defaults::dAxMaxSpeed, UC2Defaults::dAxMinSpeed, UC2Defaults::dAxMaxPos, UC2Defaults::dAxMinPos, UC2Defaults::dAxHomingDirection, UC2Defaults::dAxHomingTimeOut, UC2Defaults::dAxHomingSpeed, UC2Defaults::dAxHomingEndStopPolarity, UC2Defaults::dStepSizeUM };
    }
}



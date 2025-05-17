//////////////////////////////////
///
/// This is for parsing strings that are sequentially been read from the buffer of the Serial interface and do have the following structure
/// 
/// ++
/// SOME STRING OF UNKOWN LENGTH
/// --
/// 
/// The class manages also to handle fragmented strings or several strings in one catch
/// 
//////////////////////////////////


//////////////////////////////////////////////////
///                                            ///
///	Developped by Christian Karras, April 2025 ///
///                                            ///
//////////////////////////////////////////////////

#pragma once
#include <string>
#include <queue>
#include <mutex>

class UC2SerialResponseParser {
public:
    /// <summary>
    /// add a new raw string
    /// </summary>
    /// <param name="input"></param>
    void addInput(const std::string& input);
    /// <summary>
    /// get the first found string that was ready
    /// </summary>
    /// <returns></returns>
    std::string getFirstReady();

    /// <summary>
    /// Clear buffer;
    /// </summary>
    void clear();

private:
    std::string buffer_;
    std::queue<std::string> readyMessages_;
    std::mutex mutex_;
    std::string removeControlChars(const std::string& str);
};
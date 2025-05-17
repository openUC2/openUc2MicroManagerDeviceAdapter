#include "UC2SerialResponseParser.h"

/// <summary>
/// Add a new input to the string parser
/// </summary>
/// <param name="input"></param>
void UC2SerialResponseParser::addInput(const std::string& input) {


    std::lock_guard<std::mutex> lock(mutex_);
    buffer_ += removeControlChars(input);
    while (true) {
        size_t start = buffer_.find("++");
        if (start == std::string::npos) {
            if (buffer_.empty()) break;
            // string ends with + then we expect another + in the next round, so we don't clear the buffer
            if (buffer_.back() != '+') buffer_.clear();
            break;
        }

        size_t end = buffer_.find("--", start + 2);
        if (end == std::string::npos) {
            buffer_ = buffer_.substr(start);
            break;
        }

        size_t messageStart = start + 2;
        std::string message = buffer_.substr(messageStart, end - messageStart);
        readyMessages_.push(message);
        buffer_ = buffer_.substr(end + 2);
    }
}

/// <summary>
/// Read first found element. If none, returns empty string
/// </summary>
/// <returns></returns>
std::string UC2SerialResponseParser::getFirstReady() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (readyMessages_.empty())
        return "";
    std::string msg = readyMessages_.front();
    readyMessages_.pop();
    return msg;
}

void UC2SerialResponseParser::clear()
{
    buffer_ = "";
    while (!readyMessages_.empty()) getFirstReady();
}

std::string UC2SerialResponseParser::removeControlChars(const std::string& str) {
    std::string clean;
    for (char c : str) {
        // remove \r, \n, \t, \0 and all < 0x20 
        if (c >= 0x20 || c == '\n') {  // keep \n maybe?
            clean += c;
        }
    }
    return clean;
}
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "SimpleLogger.h"

SimpleLogger& SimpleLogger::getInstance()
{
    static SimpleLogger instance("log.txt");   // 懒汉模式，在C++11之后已经是线程安全的
    return instance;
}

void SimpleLogger::log(Level level, const char* file, int line, const char* function, const std::string& message) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ostringstream ostringstream;     // 用于输出的字符串流对象
    ostringstream << getCurrentTime() << " "
        << getLevelString(level) << " "
        << file << ": " << line << " "
        << function << "() - "
        << message;

    if (!m_file.is_open()) 
    {
         throw std::runtime_error("Log file is not open");
    }
    m_file << ostringstream.str() << std::endl;
}

SimpleLogger::SimpleLogger(const std::string& filename)
{
    m_file.open(filename, std::ios::app);
    if (!m_file.is_open()) 
    {
        throw std::runtime_error("Unable to open log file: " + filename);
    }
}

std::string SimpleLogger::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ostringstream;
    ostringstream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ostringstream.str();
}
 
std::string SimpleLogger::getLevelString(Level level) 
{
    switch (level) 
    {
    case Level::INFO:       return "INFO";
    case Level::WARNING:    return "WARNING";
    case Level::ERROR2:     return "ERROR";
    case Level::FATAL:      return "FATAL";
    default:                return "UNKNOWN";
    }
}

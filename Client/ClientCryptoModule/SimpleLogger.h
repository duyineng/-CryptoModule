#pragma once
#include <fstream>
#include <mutex>
#include <string>

class SimpleLogger  
{
public:
    enum class Level 
    {
        INFO,
        WARNING,
        ERROR2,
        FATAL
    };

    ~SimpleLogger() = default;
    static SimpleLogger& getInstance();
    void log(Level level, const std::string& message, const char* file, int line, const char* function);

private:
    SimpleLogger(const std::string& filename);              // 构造函数私有化

    SimpleLogger(const SimpleLogger&) = delete;             // 删除拷贝构造
    SimpleLogger& operator=(const SimpleLogger&) = delete;  // 删除拷贝赋值运算符

    SimpleLogger(SimpleLogger&&) = delete;                  // 删除移动构造
    SimpleLogger& operator=(SimpleLogger&&) = delete;       // 删除移动赋值运算符

    std::string getCurrentTime();
    std::string getLevelString(Level level);

    std::ofstream m_file;
    std::mutex m_mutex;
};

#define SIMPLE_LOG(level, message) SimpleLogger::getInstance().log(SimpleLogger::Level::level, message, __FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO(message)    SIMPLE_LOG(INFO, message)
#define LOG_WARNING(message) SIMPLE_LOG(WARNING, message)
#define LOG_ERROR(message)   SIMPLE_LOG(ERROR2, message)
#define LOG_FATAL(message)   SIMPLE_LOG(FATAL, message)

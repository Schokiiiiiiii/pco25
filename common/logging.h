#ifndef LOGGING_H
#define LOGGING_H

// Logging idea taken from:
// https://stackoverflow.com/questions/511768/how-to-use-my-logging-class-like-a-std-c-stream

#include <sstream>
#include <functional>
#include <iostream>
#include <mutex>

void LogFunction(const std::string& str);

#define LOG(loggingFuntion) \
    Log(loggingFuntion).GetStream()

#define Logging LOG(LogFunction)

class Log {
    using LogFunctionType = std::function<void(const std::string&)>;

public:
    explicit Log(LogFunctionType logFunction) : m_logFunction(std::move(logFunction)) { }
    std::ostringstream& GetStream() { return m_stringStream; }
    ~Log() { getMutex().lock();m_logFunction(m_stringStream.str());getMutex().unlock(); }

    std::mutex & getMutex() {
        static std::mutex themutex;
        return themutex;
    }

private:
    std::ostringstream m_stringStream;
    LogFunctionType m_logFunction;
};




#endif // LOGGING_H

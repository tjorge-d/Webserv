#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
private:
    static bool consoleOutput;
    static LogLevel minLevel;

    Logger() {} // private constructor for singleton-like behavior

public:
    static void init(std::string filename, bool console = true, LogLevel level = INFO);
    static void log(LogLevel level, const std::string& message);
    static void close();
};

std::string intToString(int value);

#endif
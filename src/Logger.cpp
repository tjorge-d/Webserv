#include "../includes/Logger.hpp"
#include <sstream>

std::ofstream logFile;
bool Logger::consoleOutput = true;
LogLevel Logger::minLevel = INFO;

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void Logger::init(std::string filename, bool console, LogLevel level) {
    logFile.open(filename.c_str());
    consoleOutput = console;
    minLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel) return;

    std::time_t now = std::time(NULL);
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    std::string levelStr;
    switch(level) {
        case DEBUG: levelStr = "DEBUG"; break;
        case INFO: levelStr = "INFO"; break;
        case WARN: levelStr = "WARN"; break;
        case ERROR: levelStr = "ERROR"; break;
    }

    std::string logEntry = std::string(timeStr) + " [" + levelStr + "] " + message + "\n";

    if (consoleOutput) {
        std::cout << logEntry;
    }

    if (logFile.is_open()) {
        logFile << logEntry;
        logFile.flush();
    }
}

void Logger::close() {
    if (logFile.is_open()) {
        logFile.close();
    }
}
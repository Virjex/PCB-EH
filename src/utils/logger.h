#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

namespace Logger {
    void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }

    void logWarning(const std::string& message) {
        std::cout << "[WARNING] " << message << std::endl;
    }

    void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
}

#endif // LOGGER_H
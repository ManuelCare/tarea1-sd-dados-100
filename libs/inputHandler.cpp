#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include "../libs/logger.cpp"

class InputHandler {
private:
    Logger *logger;
    std::atomic<bool>& isRunning;
    std::mutex& isRunningMutex;

public:
    InputHandler(Logger *logger, std::atomic<bool>& isRunning, std::mutex& isRunningMutex)
        : logger(logger), isRunning(isRunning), isRunningMutex(isRunningMutex) {}

    void run() {
        std::string userInput;
        while (isRunning) {
            std::getline(std::cin, userInput);
            logger->log(LogType::USER, userInput);
            if (userInput == "stop") {
                logger->log(LogType::INFO, "Server stopping...");
                isRunning = false;
            } else if (userInput == "help") {
                logger->log(LogType::INFO, "Here is a list of commands:\n\tstop: ends the program.\n\thelp: shows available commands.");
            } else {
                logger->log(LogType::ERROR, "Invalid input. Please try again or type 'help' to show available commands.");
            }
        }
    }
};

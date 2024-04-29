#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <csignal>
#include <vector>
#include "../libs/json.hpp"
#include "../libs/logger.cpp"
#include "../libs/SafeQueue.hpp"
#include "../libs/inputHandler.cpp"
#include "../networking/socketManager.cpp"
#include "../networking/packageHandler.cpp"
using json = nlohmann::json;

int clientSocket;

void signalHandler(int signum)
{
    std::cout << "Received Ctrl+C signal. Closing the socket..." << std::endl;
    // Close the socket
    close(clientSocket);
    exit(signum);
}

int main()
{
    //initialize some variables and the logger with its output streams.
    std::mutex isRunningMutex;
    std::atomic<bool> isRunning = true;
    Logger *logger = new Logger();
    logger->addOutput(std::cout);
    std::ofstream file_output("logs/log.txt", std::ios::app);
    logger->addOutput(file_output);

    logger->log(LogType::INFO,"Server starting...");

    //set up queues
    logger->log(LogType::INFO, "starting I/O Queues...");
    SafeQueue<json> inputQueue;
    SafeQueue<json> outputQueue;
    logger->log(LogType::INFO, "I/O Queues started.");


    //set up and running the input handler thread
    logger->log(LogType::INFO, "starting Input Handling Thread...");
    InputHandler inputHandler(logger, isRunning, isRunningMutex);
    std::thread inputHandlerThread([&inputHandler]() { inputHandler.run(); });
    logger->log(LogType::INFO, "Input Handling started.");

    // Create a UDP socketudp
    logger->log(LogType::INFO, "setting socket...");
    SocketManager *socketmanager = new SocketManager(PORT);
    if(!socketmanager->initialize()){
        logger->log(LogType::ERROR, "Socker couldn't initialize.");
        return 1;
    }
    logger->log(LogType::INFO, "socket set.");
    // Create packagehandler for the UDP socket
    PackageHandler *packagehandler = new PackageHandler(*socketmanager);
    logger->log(LogType::INFO,"Server started.");
    while (isRunning) {
        packagehandler->handleReceivedPackage(logger,&inputQueue);
        packagehandler->sendPackageToClient(logger,&outputQueue);
    }
    delete packagehandler;
    //close the socket
    delete socketmanager;
    logger->log(LogType::INFO,"Server stopped.");
    // Wait for the command thread to finish
    inputHandlerThread.join();
    gameThread.join();
    file_output.close();
    
    return 0;
}
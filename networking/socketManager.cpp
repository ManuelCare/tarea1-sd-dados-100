#pragma once

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <csignal>
#include <thread>
#include <mutex>
#include <atomic>
#include "../libs/json.hpp"
#include "../libs/logger.cpp"

class SocketManager {
private:
    int serverSocket;
    int port;
    struct sockaddr_in serverAddress;
    struct timeval timeout;
public:
    SocketManager(int port) : port(port) {}
    bool initialize() {
        serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (serverSocket == -1) {
            return false;
        }
        // Set up the server address struct
        serverAddress.sin_family = AF_INET; // IPv4
        serverAddress.sin_port = htons(port); // Port number
        serverAddress.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
        // Bind the socket to the address and port
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            return false;
        }
        // Time to wait for a package
        timeout.tv_sec = 0; // seconds
        timeout.tv_usec = 50; // microseconds
        // Set receive timeout for the serverSocket
        if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            std::cerr << "Error setting socket receive timeout" << std::endl;
            close(serverSocket);
            return false;
        }
        return true;
    }

    int getSocket() const {
        return serverSocket;
    }

    ~SocketManager() {
        close(serverSocket);
    }
};
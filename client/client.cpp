#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h> // Include poll header
#include "../libs/json.hpp"

using json = nlohmann::json;

// Function to read server IP address and port from .env file
bool readServerConfig(std::string& serverIp, int& serverPort) {
    std::ifstream envFile(".env");
    if (!envFile.is_open()) {
        std::cerr << ".env file not found\n";
        return false;
    } else {
        std::cout << ".env file found\n"; // Print statement to verify file existence
    }

    std::string line;
    while (std::getline(envFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "SERVER_IP") {
                serverIp = value;
                std::cout << "Server IP: " << serverIp << std::endl; // Print server IP
            } else if (key == "SERVER_PORT") {
                serverPort = std::stoi(value);
                std::cout << "Server Port: " << serverPort << std::endl; // Print server port
            }
        }
    }

    return true;
}

int main() {
    std::string serverIp;
    int serverPort;

    // Read server IP address and port from .env file
    if (!readServerConfig(serverIp, serverPort)) {
        return 1;
    }
    std::cout << "ip: " << serverIp << "     port: " << serverPort << std::endl;
    // Create a UDP socket
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    } else {
        std::cout << "Socket created\n"; // Print statement to verify socket creation
    }

    // Server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed\n";
        close(clientSocket);
        return 1;
    } else {
        std::cout << "Connected to server\n"; // Print statement to verify connection
    }

    // Main communication loop
    std::string action;
    std::string data;
    char buffer[1024];
    struct pollfd fds[1]; // Array of pollfd structures
    fds[0].fd = clientSocket; // Set the file descriptor to monitor
    fds[0].events = POLLIN; // Set events to poll for (incoming data)

    while (true) {
        // Take input from the user
        std::cout << "Enter action to send (or 'quit' to exit): ";
        std::getline(std::cin, action);

        if (action == "quit") {
            break; // Exit the loop if the user wants to quit
        }

        // Take data input from the user
        std::cout << "Enter data to send: ";
        std::getline(std::cin, data);

        // Create a JSON message with action and data
        json message = {
            {"action", action},
            {"data", data}
        };

        // Send the JSON message to the server
        send(clientSocket, message.dump().c_str(), message.dump().length(), 0);

        // Wait for one second for a response from the server
        int pollResult = poll(fds, 1, 1000); // Timeout of 1000 milliseconds (1 second)

        if (pollResult > 0) { // If there's data to read
            if (fds[0].revents & POLLIN) { // Check if the socket is ready to read
                // Receive response from the server
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    // Parse JSON response
                    json response = json::parse(buffer);
                    std::cout << std::setw(4) << response << std::endl;
                } else {
                    std::cerr << "Error receiving response from server\n";
                }
            }
        } else if (pollResult == 0) { // If timeout occurred
            std::cout << "Timeout occurred. No response received from server.\n";
        } else { // If poll error occurred
            std::cerr << "Poll error occurred\n";
            break;
        }
    }

    // Close the socket
    close(clientSocket);

    return 0;
}

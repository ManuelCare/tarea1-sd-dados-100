#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <csignal>
#include <atomic>
#include "../libs/json.hpp"
#include "../libs/logger.cpp"
#include "../libs/SafeQueue.hpp"
#include "../networking/socketManager.cpp"

using json = nlohmann::json;

class PackageHandler {
private:
    SocketManager& socketManager;   
    const int bufferSize = 1024;
    char *buffer;
    struct sockaddr_in clientAddress;
    socklen_t clientLength;
public:
    PackageHandler(SocketManager& socketManager) : socketManager(socketManager) {
        buffer = new char[bufferSize];
        clientLength = sizeof(clientAddress);
    }
    void handleReceivedPackage(Logger *logger, SafeQueue<json> *i_queue) {
        int bytesRead = recvfrom(socketManager.getSocket(), buffer, bufferSize, 0,
                                    (struct sockaddr*)&clientAddress, &clientLength);
        if (bytesRead > 0) {
            // Handle received data
            buffer[bytesRead] = '\0';
            std::string receivedData(buffer);
            std::string transmitterIP = inet_ntoa(clientAddress.sin_addr);
            int transmitterPort = ntohs(clientAddress.sin_port);
            logger->log(LogType::INFO, "Package received from " + transmitterIP + ":" + std::to_string(transmitterPort));
            try{
                json jsonData = json::parse(buffer);
                logger->log(LogType::INFO, "Package: " + receivedData);
                jsonData["ip"] = transmitterIP;
                jsonData["port"] = transmitterPort;
                //logic to accept packages by syntax
                if(jsonData.contains("action") && jsonData.contains("data")){
                    i_queue->Produce(std::move(jsonData));
                    logger->log(LogType::INFO, "package pushed into input queue.");
                }
                else logger->log(LogType::ERROR, "Invalid package received.");
            }
            catch(const json::exception& e){
                logger->log(LogType::ERROR, e.what());
            }
            
            // Send a response to the client
            //const char* response = "tu mensaje fue recibido, gracias!";
            //sendto(socketManager.getSocket(), response, strlen(response), 0,
            //        (struct sockaddr*)&clientAddress, sizeof(clientAddress));
        } else if (bytesRead == 0) {
            // Connection closed    
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                //logger->log(LogType::ERROR, "Receive timeout occurred.");
            } else {
                logger->log(LogType::ERROR, "Error receiving data: " + std::string(strerror(errno)));
            }
        }
    }
    void sendPackageToClient(Logger *logger,SafeQueue<json> *o_queue){
        if (o_queue->Size()>0) {
            json jsonData;
            o_queue->Consume(jsonData);
            //store address from json
            std::string recieverIP = jsonData["ip"];
            int recieverport = jsonData["port"];

            //Set up the message
            struct sockaddr_in clientAddress;
            memset(&clientAddress, 0, sizeof(clientAddress));
            clientAddress.sin_family = AF_INET;
            clientAddress.sin_port = htons(recieverport);
            inet_pton(AF_INET, recieverIP.c_str(), &(clientAddress.sin_addr));
            
            //Delete exceeding data from json (adress)
            jsonData.erase("ip");
            jsonData.erase("port");

            // Convert JSON to string and then to *char
            std::string response = jsonData.dump();

            // Send the response to the client
            sendto(socketManager.getSocket(), response.c_str(), response.size(), 0,
               (struct sockaddr*)&clientAddress, sizeof(clientAddress));
               
            // Log the sent response
            std::string logMessage = "Message sent to " + recieverIP + ":" + std::to_string(recieverport) + ", containing: " + response;
            logger->log(LogType::INFO, logMessage);
        }
    }
};
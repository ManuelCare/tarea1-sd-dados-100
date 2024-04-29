#pragma once
#include <string>

class Match;

class Player
{
private:
    std::string ip;
    int port;
    bool isInGame;
    bool isReady;
public:
    Player(std::string ip_, int port_);
    std::string getIp();
    Match *current_match;
    int getPort();
    void setReady(bool ready_);
    bool getReady();
    void setIsInGame(bool iig){isInGame = iig;}
    bool getIsInGame(){return isInGame;}
    std::string getName(){ return (ip + ":" + std::to_string(port));}
    ~Player();

};

Player::Player(std::string ip_, int port_) : ip(ip_), port(port_)
{
    isReady = false;
    isInGame = false;
}
std::string Player::getIp(){
    return ip;
}
int Player::getPort(){
    return port;
}

void Player::setReady(bool ready_)
{
    isReady = ready_;
}

bool Player::getReady()
{
    return isReady;
}
Player::~Player()
{
}

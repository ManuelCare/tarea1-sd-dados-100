#pragma once

#include <vector>
#include "./player.cpp"

class Team
{
private:
    std::string name;
public:
    std::vector<Player*> players;
    int score;
    Team(std::string name);
    std::string getName();
    bool isReady();
    ~Team
();
};

Team::Team(std::string name_)
{
    score = 0;
    name = name_;
}
std::string Team::getName(){return name;}
bool Team::isReady(){
    if(players.size()== 0) return false;
    for(int i=0; i<players.size(); i++){
        if(!(players[i]->getReady())) return false;
    }
    return true;
}
Team::~Team()
{
}

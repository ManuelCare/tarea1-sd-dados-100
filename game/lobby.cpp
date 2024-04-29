#pragma once

#include <vector>
#include <atomic>
#include <mutex>
#include "./match.cpp"
#include "../libs/json.hpp"
#include "../libs/SafeQueue.hpp"
#include "../libs/logger.cpp"
using json = nlohmann::json;
class Lobby
{
private:
    std::atomic<bool> &isRunning;
    Logger *logger;
    std::mutex &isRunningMutex;
    SafeQueue<json> *i_queue;
    SafeQueue<json> *o_queue;

public:
    Lobby(std::atomic<bool> &isRunning, std::mutex &isRunningMutex, SafeQueue<json> *i, SafeQueue<json> *o, Logger *logger)
        : isRunning(isRunning), isRunningMutex(isRunningMutex), logger(logger)
    {
        i_queue = i;
        o_queue = o;
    }
    std::vector<Player> players;
    std::vector<Match*> matches;
    void serverResponseMessage(std::string action, json data, std::string ip, int port);
    Player *lookForPlayer(std::string, int port);
    void handleAction(const json &package);
    void handleMyInfo(const json &package);
    void handleConnect(const json &package);
    void handleLobbyInfo(const json &package);
    void handleJoin(const json &package);
    void handleGameInfo(const json &package);
    void handleSwitch(const json &package);
    void handleAccept(const json &package);
    void handleReady(const json &package);
    void handleRoll(const json &package);
    void handleLeave(const json &package);
    void handleCreate(const json &package);
    void handleSetMaxTeams(const json &package);
    void handleSetMaxTeamSize(const json &package);
    void handleSetMinDiceRoll(const json &package);
    void handleSetMaxDiceRoll(const json &package);
    void handleSetMaxPositions(const json &package);
    void handleDisconnect(const json &package);
    void run();
    ~Lobby();
};

void Lobby::run()
{
    json jsonData;
    json package;
    while (isRunning)
    {
        if (i_queue->Size() > 0)
        {
            i_queue->Consume(package);
            logger->log(LogType::INFO, "Package pulled from input queue.");
            handleAction(package);
        }
    }
}
void Lobby::handleAction(const json &package)
{
    std::string action = package["action"];

    if (action == "connect")
    {
        handleConnect(package);
    }
    else if (action == "lobby_info")
    {
        handleLobbyInfo(package);
    }
    else if (action == "join")
    {
        handleJoin(package);
    }
    else if (action == "game_info")
    {
        handleGameInfo(package);
    }
    else if (action == "switch")
    {
        handleSwitch(package);
    }
    else if (action == "accept")
    {
        handleAccept(package);
    }
    else if (action == "roll")
    {
        handleRoll(package);
    }
    else if (action == "ready")
    {
        handleReady(package);
    }
    else if (action == "leave")
    {
        handleLeave(package);
    }
    else if (action == "create")
    {
        handleCreate(package);
    }
    else if (action == "set_max_teams")
    {
        handleSetMaxTeams(package);
    }
    else if (action == "set_max_team_size")
    {
        handleSetMaxTeamSize(package);
    }
    else if (action == "set_min_dice_roll")
    {
        handleSetMinDiceRoll(package);
    }
    else if (action == "set_max_dice_roll")
    {
        handleSetMaxDiceRoll(package);
    }
    else if (action == "set_max_positions")
    {
        handleSetMaxDiceRoll(package);
    }
    else if (action == "disconnect")
    {
        handleDisconnect(package);
    }
    else if (action == "my_info"){
        handleMyInfo(package);
    }
    else
    {
        logger->log(LogType::ERROR, "Bad Action: " + action);
    }
}

void Lobby::serverResponseMessage(std::string action, json data, std::string ip, int port)
{
    json response;
    response["ip"] = ip;
    response["port"] = port;
    response["action"] = action;
    response ["data"] = data;
    logger->log(LogType::INFO, "response created: " + response.dump());
    o_queue->Produce(std::move(response));
}

Player *Lobby::lookForPlayer(std::string ip, int port)
{
    int i = 0;
    while (i < players.size() && !(players[i].getIp() == ip && players[i].getPort() == port))
        i++;
    if (i < players.size())
        return &(players[i]);
    return nullptr;
}
void Lobby::handleMyInfo(const json &package){
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){serverResponseMessage("lobby_info", "null", package["ip"], package["port"]);}
    else{
        json obj;
        obj["name"] = p->getName();
        obj["isReady"] = p->getReady();
        obj["inMatch"] = p->getIsInGame();
        serverResponseMessage("lobby_info", obj, package["ip"], package["port"]);

    }
}
void Lobby::handleConnect(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){
        players.push_back(Player(package["ip"].get<std::string>(),package["port"].get<int>()));
        logger->log(LogType::INFO, "New player connected: " + players.back().getName());
        players.back().current_match = nullptr;
        players.back().setIsInGame(false);
        players.back().setReady(false);
    }
    std::cout << "antes" << std::endl;
    serverResponseMessage("connect", "successful", package["ip"], package["port"]);
}
void Lobby::handleLobbyInfo(const json &package)
{
    json infoPackage = json::array();
    std::cout << matches.size() << std::endl;
    for(int i = 0; i<matches.size(); i++){
        json obj;
        obj["id"] = matches[i]->getId();
        obj["owner"] = matches[i]->getAdmin()->getIp() + std::to_string(matches[i]->getAdmin()->getPort());
        obj["status"] = matches[i]->ended ? "ended" : (matches[i]->getHasStarted() ? "playing" : "in lobby");
        obj["players"] = matches[i]->getHowManyPlayers();
        obj["maxPlayers"] = matches[i]->getMaxTeamSize()*matches[i]->getMaxTeams();
        obj["minRoll"] = matches[i]->getMinDiceOutcome();
        obj["maxRoll"] = matches[i]->getMaxDiceOutcome();
        obj["maxPositions"] = matches[i]->getMaxPositions();
        infoPackage.push_back(obj);
    }
    serverResponseMessage("lobby_info", infoPackage, package["ip"], package["port"]);
}
void Lobby::handleJoin(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  }
    else if(p->getIsInGame()) {serverResponseMessage("join","failure",package["ip"],package["port"]);}
    else{
        std::string curr_match_id = package["data"].get<std::string>();
        int i=0;
        while(i<matches.size() && matches[i]->getId()!=curr_match_id) i++;
        if(i<matches.size()){
            p->setIsInGame(true);
            p->current_match = (matches[i]);
            p->current_match->waittojoin(p);
            serverResponseMessage("join","successful",package["ip"],package["port"]);
        }
        else{ serverResponseMessage("join","failure",package["ip"],package["port"]); }
    }
}
void Lobby::handleGameInfo(const json &package)
{
    std::cout << "ma" << std::endl;
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("game_info","failure",package["ip"],package["port"]);  return;}
    if(p->current_match != nullptr){
        json data;
        std::cout << "sha" << std::endl;
        json teamsPackage = json::array();
        for(int i=0; i<p->current_match->teams.size(); i++){
            json obj;
            std::cout << "wa" << std::endl;
            obj["name"] = p->current_match->teams[i]->getName();
            obj["score"] = p->current_match->teams[i]->score;
            std::cout << "ra" << std::endl;
            json playersPackage = json::array();
            for(int j=0; j<p->current_match->teams[i]->players.size(); j++){
                json p_obj;
                p_obj["name"] = p->current_match->teams[i]->players[j]->getName();
                playersPackage.push_back(p_obj);
            }
            std::cout << "sha" << std::endl;
            obj["players"] = playersPackage;
            teamsPackage.push_back(obj);
            std::cout << "bom" << std::endl;
        }
        data["teams"] = teamsPackage;
        json waitingPackage = json::array();
        std::cout << "ba" << std::endl;
        for(int i=0; i<p->current_match->wtj.size(); i++){
            json pw_obj;
            std::cout << "cha" << std::endl;
            pw_obj["name"] = p->current_match->wtj[i].player !=nullptr ? p->current_match->wtj[i].player->getName() : "none";
            pw_obj["desired_team"] = p->current_match->wtj[i].desiredTeam!=nullptr ? p->current_match->wtj[i].desiredTeam->getName() : "none";
            std::cout << "ra" << std::endl;
            waitingPackage.push_back(pw_obj);
        }
        data["waiting"] = waitingPackage;
        std::cout << "ta" << std::endl;
        data["round"] = p->current_match->ronda;
        if(p->current_match->getHasStarted()) data["team_turno"] = p->current_match->team_turno->getName();
        serverResponseMessage("game_info", data, package["ip"], package["port"]);
    }
    else serverResponseMessage("game_info", "failure", package["ip"], package["port"]);
    
}
void Lobby::handleSwitch(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("switch","failure",package["ip"],package["port"]);  return;}
    p->current_match->joinTeam(p,package["data"]);
    p->current_match->tryAcceptAll();
}
void Lobby::handleAccept(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("accept","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr){
        p->current_match->addAccept(p,package["data"].get<std::string>());
    }
}
void Lobby::handleReady(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("ready","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr){
        if(!(p->current_match->getHasStarted())){
            p->setReady(!(p->getReady()));
            if(p->current_match->start()){
            p->current_match->setHasStarted(true);
            }
        }
    }
}
void Lobby::handleRoll(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("roll","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr) {
        p->current_match->roll(p,std::atoi(package["data"].get<std::string>().c_str()));
    }
}
void Lobby::handleLeave(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);

    if(p==nullptr){ serverResponseMessage("leave","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr) p->current_match->leave(p);
}
void Lobby::handleCreate(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){
        serverResponseMessage("create","failure",package["ip"],package["port"]); 
        return;
    }
    if(p->current_match==nullptr){
        matches.push_back(new Match(p));
        p->setIsInGame(true);
        p->setReady(false);
        serverResponseMessage("create","successful",package["ip"],package["port"]); 
    }
}
void Lobby::handleSetMaxTeams(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr && p->current_match->getAdmin()==p){
        if(!(p->current_match->getHasStarted())){
            p->current_match->setMaxTeams(std::atoi(package["data"].get<std::string>().c_str()));
        }
    }
}
void Lobby::handleSetMaxTeamSize(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr && p->current_match->getAdmin()==p){
        if(!(p->current_match->getHasStarted())){
            p->current_match->setMaxTeamSize(std::atoi(package["data"].get<std::string>().c_str()));
        }
    }
}
void Lobby::handleSetMinDiceRoll(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr && p->current_match->getAdmin()==p){
        if(!(p->current_match->getHasStarted())){
            p->current_match->setMinDiceOutcome(std::atoi(package["data"].get<std::string>().c_str()));
        }
    }
}
void Lobby::handleSetMaxDiceRoll(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr && p->current_match->getAdmin()==p){
        if(!(p->current_match->getHasStarted())){
            p->current_match->setMaxDiceOutcome(std::atoi(package["data"].get<std::string>().c_str()));
        }
    }
}
void Lobby::handleSetMaxPositions(const json &package)
{
    Player *p = lookForPlayer(package["ip"],package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  return;}
    if(p->current_match!=nullptr && p->current_match->getAdmin()==p){
        if(!(p->current_match->getHasStarted())){
            p->current_match->setMaxPositions(std::atoi(package["data"].get<std::string>().c_str()));
        }
    }
}
void Lobby::handleDisconnect(const json &package)
{
    Player *p = lookForPlayer(package["ip"], package["port"]);
    if(p==nullptr){ serverResponseMessage("join","failure",package["ip"],package["port"]);  return;}
    handleLeave(package);
    int i=0;
    while(i<players.size() && players[i].getName() != p->getName()) i++;
    if(i<players.size()){
        players.erase(players.begin()+i);
    }
    logger->log(LogType::INFO, "Player disconnected: " + package["ip"].get<std::string>() + ":" + package["port"].get<std::string>());
}

Lobby::~Lobby()
{
}

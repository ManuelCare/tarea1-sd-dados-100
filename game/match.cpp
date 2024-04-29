#pragma once

#include "./team.cpp"
#include <vector>
#include <string> // Add this include for std::string
#include <chrono>
#include <random>
std::string generateMatchID()
{
    // Get current time in milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch().count();

    // Generate a random number for additional uniqueness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9999);
    int random = dis(gen);

    // Combine timestamp and random number to form the ID
    std::string matchID = std::to_string(value) + std::to_string(random);
    return matchID;
}
struct waiting_to_join
{
    Player *player;
    Team *desiredTeam;
    std::vector<Player *> teamAcceptance;
};
struct waiting_to_roll
{
    Player *player;
};

class Match
{
private:
    std::string id;
    Player *admin;
    bool hasStarted;
    int maxTeams;
    int maxTeamSize;
    int maxPositions;
    int minDiceOutcome;
    int maxDiceOutcome;


public:
    int idx_turno;
    Team *team_turno;
    std::vector<Team *> teams; // Use pointers to Team objects
    bool ended;
    int ronda;
    // Constructors
    Match(Player *p);
    Match(Player *p, int maxTeams_, int maxTeamSize_, int minDiceOutcome_, int maxDiceOutcome_, int maxPositions_);

    // Destructor
    ~Match();

    // Getters and setters
    std::vector<waiting_to_join> wtj;
    std::vector<waiting_to_roll> wtr;
    std::string getId() const { return id; }

    Player *getAdmin() const { return admin; }
    void setAdmin(Player *newAdmin) { admin = newAdmin; }

    bool getHasStarted() const { return hasStarted; }
    void setHasStarted(bool value) { hasStarted = value; }

    int getMaxTeamSize() const { return maxTeamSize; }
    void setMaxTeamSize(int value) { maxTeamSize = value; }
    int getMaxTeams() const { return maxTeams; }
    void setMaxTeams(int maxTeams_) { 
    for(int i=maxTeams; i<maxTeams; i++){
        addTeam();
    }
    maxTeams = maxTeams_; 
    }
    int getMaxPositions() const { return maxPositions; }
    void setMaxPositions(int value) { maxPositions = value; }

    int getMinDiceOutcome() const { return minDiceOutcome; }
    void setMinDiceOutcome(int value) { minDiceOutcome = value; }

    int getMaxDiceOutcome() const { return maxDiceOutcome; }
    void setMaxDiceOutcome(int value) { maxDiceOutcome = value; }

    // Methods
    void leave(Player *p)
    {
        int i = 0;
        while (i < wtj.size() && wtj[i].player != p)
            i++;
        if (i < wtj.size())
        {
            wtj.erase(wtj.begin() + i);
        }
        i = 0;
        while (i < wtr.size() && wtr[i].player != p)
            i++;
        if (i < wtr.size())
        {
            wtr.erase(wtr.begin() + i);
        }
        int j = 0;
        while (j < teams.size())
        {
            i = 0;
            while (i < teams[j]->players.size() && teams[j]->players[i] != p)
                i++;
            if (i < teams[j]->players.size())
            {
                teams[j]->players.erase(teams[j]->players.begin() + i);
                j = teams.size();
            }
            j++;
        }
        if (p = admin)
            admin = nullptr;
        p->current_match = nullptr;
        p->setReady(false);
        p->setIsInGame(false);
    }
    void addTeam();
    bool canStart();
    int getHowManyPlayers()
    {
        int c = 0;
        for (int i = 0; i < teams.size(); i++)
        {
            c += teams[i]->players.size();
        }
        return c;
    }
    bool start()
    {
        if (canStart())
        {
            setHasStarted(true);
            ronda = 1;
            idx_turno = 0;
            team_turno = teams[idx_turno];
            wtr.clear();
            for (int i = 0; i < team_turno->players.size(); i++)
            {
                waiting_to_roll a;
                a.player = team_turno->players[i];
                wtr.push_back(a);
            }
            return true;
        }
        return false;
    }
    bool isTurnFinished()
    {
        return !wtr.size();
    }
    bool nextTurn()
    {
        if (!isTurnFinished())
            return false;
        idx_turno += 1;
        if (idx_turno >= teams.size())
        {
            idx_turno = 0;
            ronda += 1;
        }
        team_turno = teams[idx_turno];
        wtr.clear();
        for (int i = 0; i < team_turno->players.size(); i++)
        {
            waiting_to_roll a;
            a.player = team_turno->players[i];
            wtr.push_back(a);
        }
        return true;
    }
    bool addAccept(Player *p, std::string data)
    {
        int i = 0;
        while (i < wtj.size() && data != (wtj[i].player->getIp() + ":" + std::to_string(wtj[i].player->getPort())))
            i++;
        if (i >= wtj.size())
            return false;
        int j = 0;
        while (j < wtj[i].teamAcceptance.size() && p != wtj[i].teamAcceptance[j])
            j++;
        if (j < wtj.size())
            return false;
        wtj[i].teamAcceptance.push_back(p);
        return true;
    }
    bool tryAcceptAll()
    {
        int i = 0;
        while (i < wtj.size())
        {
            if (tryAcceptHelper(&(wtj[i])))
            {
                wtj.erase(wtj.begin() + i);
            }
            else
                i++;
        }
    }
     bool tryAcceptHelper(waiting_to_join *wtj)
    {
        for (int i = 0; i < wtj->desiredTeam->players.size(); i++)
        {
            int j = 0;
            while (j < wtj->teamAcceptance.size() && (wtj->desiredTeam->players[i] != wtj->teamAcceptance[j]))
                j++;
            if (j >= wtj->teamAcceptance.size())
                return false;
        }
        wtj->desiredTeam->players.push_back(wtj->player);
        return true;
    }
    bool joinTeam(Player *p, std::string name)
    {
        if (ended) // si termino
            return false;
        for (int k = 0; k < wtr.size(); k++) //si me peran pa rolear no
        {
            if (wtr[k].player = p)
                return false;
        }
        int i = 0;
        while (i < wtj.size() && wtj[i].player != p) // si ya estoy esperando para conectarme
            i++;
        if (i < wtj.size())
        {
            for (int k = 0; k < teams.size(); k++)
            {
                if (teams[k]->getName() == name)
                    wtj[i].desiredTeam = teams[k];
            }
        }
    }
    void waittojoin(Player *p)
    {
        waiting_to_join a;
        a.player = p;
        a.desiredTeam = teams[0];
        wtj.push_back(a);
    }
    void end()
    {
        for (int i = 0; i < teams.size(); i++)
        {
            for (int j = 0; j < teams[i]->players.size(); j++)
            {
                teams[i]->players[j]->current_match = nullptr;
                teams[i]->players[j]->setReady(false);
                teams[i]->players[j]->setIsInGame(false);
            }
        }
        for (int i = 0; i < wtj.size(); i++)
        {
            wtj[i].player->current_match = nullptr;
            wtj[i].player->setIsInGame(false);
            wtj[i].player->setReady(false);
        }
    }
    void roll(Player *p, int value)
    {
        if (minDiceOutcome <= value && value <= maxDiceOutcome)
        {
            int i = 0;
            while (i < wtr.size() && wtr[i].player != p)
                i++;
            if (i < wtr.size())
            {
                team_turno->score += value;
                wtr.erase(wtr.begin() + i);
                if (team_turno->score >= maxPositions)
                    ended = true;
            }
            nextTurn();
        }
    }
};

// Default constructor
Match::Match(Player *p)
{
    admin = p;
    p->current_match = this;
    idx_turno = -1;
    team_turno = nullptr;
    id = generateMatchID();
    addTeam();
    teams[0]->players.push_back(p);
    addTeam();
    hasStarted = false;
    maxTeams = 2;
    maxTeamSize = 5;
    maxPositions = 100;
    minDiceOutcome = 1;
    maxDiceOutcome = 6;
    ended = false;
    ronda = 0;
}

// Parameterized constructor
Match::Match(Player *p, int maxTeams_, int maxTeamSize_, int minDiceOutcome_, int maxDiceOutcome_, int maxPositions_)
{
    admin = p;
    p->current_match = this;
    idx_turno = -1;
    team_turno = nullptr;
    id = generateMatchID();
    addTeam();
    teams[0]->players.push_back(p);
    addTeam();
    hasStarted = false;
    maxTeams = maxTeams_;
    maxTeamSize = maxTeamSize_;
    minDiceOutcome = minDiceOutcome_;
    maxDiceOutcome = maxDiceOutcome_;
    maxPositions = maxPositions_;
    ended = false;
    ronda = 0;
}

// Destructor
Match::~Match()
{
    end();
    for (auto &team : teams)
    {
        delete team;
    }
    teams.clear();
}

// Method to add a team
void Match::addTeam()
{
    std::string teamName;
    if (teams.empty())
    {
        teamName = "team_a";
    }
    else
    {
        // Get the last team name
        std::string lastTeamName = teams.back()->getName();

        // Extract the last character from the last team name
        char lastChar = lastTeamName.back();

        // Increment the last character to get the next team name
        teamName = "team_";
        teamName += static_cast<char>(lastChar + 1);
    }
    // Create a new Team object with the generated name
    teams.push_back(new Team(teamName));
}

bool Match::canStart()
{
    std::cout << "entro" << std::endl;
    if (teams.size() == 0)
        return false;
    std::cout << "siguio" << std::endl;
    for (int i = 0; i < teams.size(); i++)
    {
        if (!(teams[i]->isReady()) || teams[i]->players.size() == 0)
            return false;
    }
    return true;
}

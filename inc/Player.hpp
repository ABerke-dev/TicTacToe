#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <utility>
#include "CellState.hpp"

class Player {
protected:
    std::string name;
    CellState symbol;
    int score;

public:
    Player(const std::string& playerName, CellState playerSymbol);
    virtual ~Player();
    
    virtual std::pair<int, int> makeMove() = 0;
    
    std::string getName() const { return name; }
    CellState getSymbol() const { return symbol; }
    int getScore() const { return score; }
    void incrementScore() { ++score; }
};

#endif
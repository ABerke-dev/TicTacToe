#ifndef HUMANPLAYER_HPP
#define HUMANPLAYER_HPP

#include "Player.hpp"

class HumanPlayer : public Player {
private:
    int clickRow;
    int clickCol;
    bool hasClick;

public:
    HumanPlayer(const std::string& name, CellState symbol);
    std::pair<int, int> makeMove() override;
    void setClick(int row, int col);
};

#endif
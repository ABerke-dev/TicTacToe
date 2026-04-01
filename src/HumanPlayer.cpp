#include "../inc/HumanPlayer.hpp"
#include <iostream>

HumanPlayer::HumanPlayer(const std::string& name, CellState symbol) 
    : Player(name, symbol), clickRow(-1), clickCol(-1), hasClick(false) {
}

std::pair<int, int> HumanPlayer::makeMove() {
    if (hasClick) {
        hasClick = false;
        return {clickRow, clickCol};
    }
    return {-1, -1};
}

void HumanPlayer::setClick(int row, int col) {
    clickRow = row;
    clickCol = col;
    hasClick = true;
    std::cout << "[HumanPlayer] Click registered at (" << row << ", " << col << ")" << std::endl;
}
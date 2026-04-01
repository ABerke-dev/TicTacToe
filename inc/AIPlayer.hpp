#ifndef AIPLAYER_HPP
#define AIPLAYER_HPP

#include "Player.hpp"
#include "Board.hpp"
#include <random>

enum class Difficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

class AIPlayer : public Player {
private:
    Difficulty difficulty;
    std::mt19937 rng;
    const Board* boardRef;
    
    std::pair<int, int> makeRandomMove(const Board& board);
    std::pair<int, int> findWinningMove(const Board& board, CellState symbol);
    std::pair<int, int> findBestMove(const Board& board);
    bool checkWinAt(const Board& board, int row, int col, CellState symbol);
    int minimax(Board& board, int depth, bool isMaximizing, int alpha, int beta);

public:
    AIPlayer(const std::string& name, CellState symbol, Difficulty diff);
    std::pair<int, int> makeMove() override;
    
    void setBoard(const Board* b) { boardRef = b; }
    void setDifficulty(Difficulty diff) { difficulty = diff; }
};

#endif

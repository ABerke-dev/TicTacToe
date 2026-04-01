#include "../inc/AIPlayer.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

AIPlayer::AIPlayer(const std::string& name, CellState symbol, Difficulty diff)
    : Player(name, symbol), difficulty(diff), boardRef(nullptr) {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));
    std::cout << "[AIPlayer] Created with difficulty: ";
    if (diff == Difficulty::EASY) std::cout << "EASY";
    else if (diff == Difficulty::MEDIUM) std::cout << "MEDIUM";
    else if (diff == Difficulty::HARD) std::cout << "HARD";
    else std::cout << "EXPERT";
    std::cout << std::endl;
}

std::pair<int, int> AIPlayer::makeMove() {
    if (!boardRef) {
        return {-1, -1};
    }
    
    std::cout << "[AIPlayer] Thinking..." << std::endl;
    
    if (difficulty == Difficulty::EASY) {
        return makeRandomMove(*boardRef);
    } 
    else if (difficulty == Difficulty::MEDIUM) {
        auto winMove = findWinningMove(*boardRef, symbol);
        if (winMove.first != -1) {
            std::cout << "[AIPlayer] Found winning move!" << std::endl;
            return winMove;
        }
        return makeRandomMove(*boardRef);
    } 
    else if (difficulty == Difficulty::HARD) {
        auto winMove = findWinningMove(*boardRef, symbol);
        if (winMove.first != -1) {
            std::cout << "[AIPlayer] Found winning move!" << std::endl;
            return winMove;
        }
        
        CellState opponentSymbol = (symbol == CellState::X) ? CellState::O : CellState::X;
        auto blockMove = findWinningMove(*boardRef, opponentSymbol);
        if (blockMove.first != -1) {
            std::cout << "[AIPlayer] Blocking opponent!" << std::endl;
            return blockMove;
        }
        
        int size = boardRef->getSize();
        int center = size / 2;
        if (boardRef->getCellAt(center, center) == CellState::EMPTY) {
            std::cout << "[AIPlayer] Taking center!" << std::endl;
            return {center, center};
        }
        
        return makeRandomMove(*boardRef);
    }
    else { // EXPERT
        return findBestMove(*boardRef);
    }
}

std::pair<int, int> AIPlayer::makeRandomMove(const Board& board) {
    std::vector<std::pair<int, int>> availableMoves;
    int size = board.getSize();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.getCellAt(i, j) == CellState::EMPTY) {
                availableMoves.push_back({i, j});
            }
        }
    }
    
    if (availableMoves.empty()) return {-1, -1};
    
    std::uniform_int_distribution<int> dist(0, static_cast<int>(availableMoves.size()) - 1);
    auto move = availableMoves[dist(rng)];
    std::cout << "[AIPlayer] Random move: (" << move.first << ", " << move.second << ")" << std::endl;
    return move;
}

std::pair<int, int> AIPlayer::findWinningMove(const Board& board, CellState targetSymbol) {
    int size = board.getSize();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.getCellAt(i, j) == CellState::EMPTY) {
                if (checkWinAt(board, i, j, targetSymbol)) {
                    return {i, j};
                }
            }
        }
    }
    return {-1, -1};
}

bool AIPlayer::checkWinAt(const Board& board, int row, int col, CellState checkSymbol) {
    Board tempBoard(board.getSize(), nullptr);
    
    int size = board.getSize();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            CellState cell = board.getCellAt(i, j);
            if (cell != CellState::EMPTY) {
                tempBoard.placeMark(i, j, cell);
            }
        }
    }
    
    tempBoard.placeMark(row, col, checkSymbol);
    
    return tempBoard.countMatches(checkSymbol) > 0;
}

std::pair<int, int> AIPlayer::findBestMove(const Board& board) {
    int size = board.getSize();
    
    if (size > 4) {
        auto winMove = findWinningMove(board, symbol);
        if (winMove.first != -1) return winMove;
        
        CellState opponentSymbol = (symbol == CellState::X) ? CellState::O : CellState::X;
        auto blockMove = findWinningMove(board, opponentSymbol);
        if (blockMove.first != -1) return blockMove;
        
        return makeRandomMove(board);
    }
    
    int bestScore = -10000;
    std::pair<int, int> bestMove = {-1, -1};
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.getCellAt(i, j) == CellState::EMPTY) {
                Board tempBoard(size, nullptr);
                for (int x = 0; x < size; x++) {
                    for (int y = 0; y < size; y++) {
                        CellState cell = board.getCellAt(x, y);
                        if (cell != CellState::EMPTY) {
                            tempBoard.placeMark(x, y, cell);
                        }
                    }
                }
                
                tempBoard.placeMark(i, j, symbol);
                
                int moveScore = minimax(tempBoard, 0, false, -10000, 10000);
                
                if (moveScore > bestScore) {
                    bestScore = moveScore;
                    bestMove = {i, j};
                }
            }
        }
    }
    
    std::cout << "[AIPlayer] Best move found with minimax: (" 
              << bestMove.first << ", " << bestMove.second << ")" << std::endl;
    return bestMove;
}

int AIPlayer::minimax(Board& board, int depth, bool isMaximizing, int alpha, int beta) {
    CellState opponentSymbol = (symbol == CellState::X) ? CellState::O : CellState::X;
    
    int myMatches = board.countMatches(symbol);
    int oppMatches = board.countMatches(opponentSymbol);
    
    if (myMatches > 0) return 10 - depth;
    if (oppMatches > 0) return depth - 10;
    if (board.isFull() || depth > 6) return 0;
    
    int size = board.getSize();
    
    if (isMaximizing) {
        int maxScore = -10000;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board.getCellAt(i, j) == CellState::EMPTY) {
                    board.placeMark(i, j, symbol);
                    int moveScore = minimax(board, depth + 1, false, alpha, beta);
                    board.placeMark(i, j, CellState::EMPTY);
                    
                    maxScore = std::max(maxScore, moveScore);
                    alpha = std::max(alpha, moveScore);
                    if (beta <= alpha) break;
                }
            }
        }
        return maxScore;
    } else {
        int minScore = 10000;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board.getCellAt(i, j) == CellState::EMPTY) {
                    board.placeMark(i, j, opponentSymbol);
                    int moveScore = minimax(board, depth + 1, true, alpha, beta);
                    board.placeMark(i, j, CellState::EMPTY);
                    
                    minScore = std::min(minScore, moveScore);
                    beta = std::min(beta, moveScore);
                    if (beta <= alpha) break;
                }
            }
        }
        return minScore;
    }
}
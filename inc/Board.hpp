#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <SDL2/SDL.h>
#include "CellState.hpp"

class Board {
private:
    std::vector<std::vector<CellState>> grid;
    int size;
    SDL_Renderer* renderer;

public:
    Board(int boardSize, SDL_Renderer* rend);
    ~Board();
    
    void reset();
    bool isFull() const;
    bool placeMark(int row, int col, CellState mark);
    CellState getCellAt(int row, int col) const;
    int countMatches(CellState mark) const;  // NEW: Count all 3-in-a-rows
    void render(int offsetX, int offsetY);
    void renderWinningLines(int offsetX, int offsetY, CellState mark);  // NEW: Render ALL winning lines
    std::pair<int, int> getClickedCell(int mouseX, int mouseY, int offsetX, int offsetY);
    
    int getSize() const { return size; }
    int getCellValue(int row, int col) const;
    
    struct WinLine {
        int startRow, startCol, endRow, endCol;
        bool isValid;
    };
    
    std::vector<WinLine> getAllWinningLines(CellState mark) const;  // NEW
};

#endif
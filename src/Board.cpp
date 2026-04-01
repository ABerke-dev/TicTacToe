#include "../inc/Board.hpp"
#include <iostream>
#include <cmath>

Board::Board(int boardSize, SDL_Renderer* rend) 
    : size(boardSize), renderer(rend) {
    grid.resize(size, std::vector<CellState>(size, CellState::EMPTY));
    std::cout << "[Board] Created " << size << "x" << size << " board" << std::endl;
}

Board::~Board() {
    std::cout << "[Board] Destroyed" << std::endl;
}

void Board::reset() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell = CellState::EMPTY;
        }
    }
    std::cout << "[Board] Reset complete" << std::endl;
}

bool Board::isFull() const {
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell == CellState::EMPTY) return false;
        }
    }
    return true;
}

bool Board::placeMark(int row, int col, CellState mark) {
    if (row < 0 || row >= size || col < 0 || col >= size) {
        return false;
    }
    if (grid[row][col] != CellState::EMPTY) {
        return false;
    }
    grid[row][col] = mark;
    std::cout << "[Board] Placed " << (mark == CellState::X ? "X" : "O") 
              << " at (" << row << ", " << col << ")" << std::endl;
    return true;
}

CellState Board::getCellAt(int row, int col) const {
    if (row < 0 || row >= size || col < 0 || col >= size) {
        return CellState::EMPTY;
    }
    return grid[row][col];
}

int Board::countMatches(CellState mark) const {
    int count = 0;
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j <= size - 3; ++j) {
            if (grid[i][j] == mark && 
                grid[i][j+1] == mark && 
                grid[i][j+2] == mark) {
                count++;
            }
        }
    }
    
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i <= size - 3; ++i) {
            if (grid[i][j] == mark && 
                grid[i+1][j] == mark && 
                grid[i+2][j] == mark) {
                count++;
            }
        }
    }
    
    for (int i = 0; i <= size - 3; ++i) {
        for (int j = 0; j <= size - 3; ++j) {
            if (grid[i][j] == mark && 
                grid[i+1][j+1] == mark && 
                grid[i+2][j+2] == mark) {
                count++;
            }
        }
    }
    
    for (int i = 0; i <= size - 3; ++i) {
        for (int j = 2; j < size; ++j) {
            if (grid[i][j] == mark && 
                grid[i+1][j-1] == mark && 
                grid[i+2][j-2] == mark) {
                count++;
            }
        }
    }
    
    return count;
}

std::vector<Board::WinLine> Board::getAllWinningLines(CellState mark) const {
    std::vector<WinLine> lines;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j <= size - 3; ++j) {
            if (grid[i][j] == mark && 
                grid[i][j+1] == mark && 
                grid[i][j+2] == mark) {
                lines.push_back({i, j, i, j+2, true});
            }
        }
    }
    
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i <= size - 3; ++i) {
            if (grid[i][j] == mark && 
                grid[i+1][j] == mark && 
                grid[i+2][j] == mark) {
                lines.push_back({i, j, i+2, j, true});
            }
        }
    }
    
    for (int i = 0; i <= size - 3; ++i) {
        for (int j = 0; j <= size - 3; ++j) {
            if (grid[i][j] == mark && 
                grid[i+1][j+1] == mark && 
                grid[i+2][j+2] == mark) {
                lines.push_back({i, j, i+2, j+2, true});
            }
        }
    }
    
    for (int i = 0; i <= size - 3; ++i) {
        for (int j = 2; j < size; ++j) {
            if (grid[i][j] == mark && 
                grid[i+1][j-1] == mark && 
                grid[i+2][j-2] == mark) {
                lines.push_back({i, j, i+2, j-2, true});
            }
        }
    }
    
    return lines;
}

void Board::render(int offsetX, int offsetY) {
    int maxBoardSize = 600;
    int boardWidth = std::min(maxBoardSize, 1280 - 400);
    int boardHeight = boardWidth;
    int cellSize = boardWidth / size;
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i <= size; ++i) {
        int x = offsetX + i * cellSize;
        SDL_RenderDrawLine(renderer, x, offsetY, x, offsetY + boardHeight);
        
        int y = offsetY + i * cellSize;
        SDL_RenderDrawLine(renderer, offsetX, y, offsetX + boardWidth, y);
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int x = offsetX + j * cellSize + cellSize / 2;
            int y = offsetY + i * cellSize + cellSize / 2;
            int markSize = cellSize / 3;
            
            if (markSize > 50) markSize = 50;
            
            if (grid[i][j] == CellState::X) {

                SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
                for (int thick = 0; thick < 8; thick++) {
                    SDL_RenderDrawLine(renderer, 
                        x - markSize - thick, y - markSize, 
                        x + markSize + thick, y + markSize);
                    SDL_RenderDrawLine(renderer, 
                        x - markSize, y - markSize - thick, 
                        x + markSize, y + markSize + thick);
                    SDL_RenderDrawLine(renderer, 
                        x + markSize + thick, y - markSize, 
                        x - markSize - thick, y + markSize);
                    SDL_RenderDrawLine(renderer, 
                        x + markSize, y - markSize - thick, 
                        x - markSize, y + markSize + thick);
                }
            } 
            else if (grid[i][j] == CellState::O) {

                SDL_SetRenderDrawColor(renderer, 80, 200, 255, 255);
                for (int thick = 0; thick < 8; thick++) {
                    for (int angle = 0; angle < 360; angle += 1) {
                        int x1 = x + (markSize + thick) * cos(angle * 3.14159 / 180);
                        int y1 = y + (markSize + thick) * sin(angle * 3.14159 / 180);
                        SDL_RenderDrawPoint(renderer, x1, y1);
                    }
                }
            }
        }
    }
}

void Board::renderWinningLines(int offsetX, int offsetY, CellState mark) {
    auto lines = getAllWinningLines(mark);
    if (lines.empty()) return;
    
    int maxBoardSize = 600;
    int boardWidth = std::min(maxBoardSize, 1280 - 400);
    int cellSize = boardWidth / size;
    
    for (const auto& line : lines) {
        for (int step = 0; step <= 2; step++) {
            int cellX = line.startCol + step * (line.endCol - line.startCol) / 2;
            int cellY = line.startRow + step * (line.endRow - line.startRow) / 2;
            
            int centerX = offsetX + cellX * cellSize + cellSize / 2;
            int centerY = offsetY + cellY * cellSize + cellSize / 2;
            
            SDL_Color glowColor = (mark == CellState::X) ? 
                SDL_Color{255, 100, 100, 220} : SDL_Color{100, 200, 255, 220};
            
            int maxRadius = cellSize / 2 - 5;  
            
            for (int i = 0; i < 80; i++) {  
                int angle = i * 4.5;
                int radius = 10 + (i % 8) * (maxRadius / 8); 
                int px = centerX + radius * cos(angle * 3.14159 / 180);
                int py = centerY + radius * sin(angle * 3.14159 / 180);
                
                SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, glowColor.a);
                for (int r = 0; r < 5; r++) {
                    SDL_RenderDrawPoint(renderer, px + r, py);
                    SDL_RenderDrawPoint(renderer, px, py + r);
                    SDL_RenderDrawPoint(renderer, px - r, py);
                    SDL_RenderDrawPoint(renderer, px, py - r);
                }
            }
            
            for (int radius = 5; radius < maxRadius; radius += 4) {
                Uint8 alpha = 200 - (radius * 200 / maxRadius);  
                SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, alpha);
                for (int angle = 0; angle < 360; angle += 8) {
                    int px = centerX + radius * cos(angle * 3.14159 / 180);
                    int py = centerY + radius * sin(angle * 3.14159 / 180);
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }
    }
}

std::pair<int, int> Board::getClickedCell(int mouseX, int mouseY, int offsetX, int offsetY) {
    int maxBoardSize = 600;
    int boardWidth = std::min(maxBoardSize, 1280 - 400);
    int boardHeight = boardWidth;
    int cellSize = boardWidth / size;
    
    if (mouseX < offsetX || mouseX > offsetX + boardWidth ||
        mouseY < offsetY || mouseY > offsetY + boardHeight) {
        return {-1, -1};
    }
    
    int col = (mouseX - offsetX) / cellSize;
    int row = (mouseY - offsetY) / cellSize;
    
    if (row < 0 || row >= size || col < 0 || col >= size) {
        return {-1, -1};
    }
    
    std::cout << "[Board] Click at mouse(" << mouseX << "," << mouseY 
              << ") -> grid[" << row << "][" << col << "]" << std::endl;
    
    return {row, col};
}

int Board::getCellValue(int row, int col) const {
    return static_cast<int>(getCellAt(row, col));
}
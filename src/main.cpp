#include "../inc/GameEngine.hpp"
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char* argv[]) {
    try {
        #ifdef _WIN32
        SetConsoleOutputCP(65001);
        #endif
        
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   TIC-TAC-TOE BATTLE - Version 2.0    \n";
        std::cout << "     Alara Alpaslan & Ahmet Berke      \n";
        std::cout << "========================================\n";
        std::cout << "\n";
        std::cout << "Initializing game systems...\n";
        std::cout << "\n";
        
        GameEngine game;
        game.run();
        
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "       Thanks for playing!             \n";
        std::cout << "========================================\n";
        std::cout << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\n!!! FATAL ERROR !!!" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "\nPlease check:\n";
        std::cerr << "1. SDL2 libraries are installed\n";
        std::cerr << "2. All DLL files are present\n";
        std::cerr << "3. Graphics drivers are up to date\n\n";
        
        #ifdef _WIN32
        MessageBoxA(nullptr, e.what(), "TicTacToe - Fatal Error", MB_OK | MB_ICONERROR);
        #endif
        
        return 1;
    }
    
    return 0;
}
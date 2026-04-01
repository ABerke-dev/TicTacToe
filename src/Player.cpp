#include "../inc/Player.hpp"
#include <iostream>

Player::Player(const std::string& playerName, CellState playerSymbol) 
    : name(playerName), symbol(playerSymbol), score(0) {
    std::cout << "[Player] Created: " << name << std::endl;
}

Player::~Player() {
    std::cout << "[Player] Destroyed: " << name << std::endl;
}
#include "../inc/GameEngine.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <map>
#include <algorithm>

GameEngine::GameEngine() 
    : window(nullptr), renderer(nullptr), titleFont(nullptr), normalFont(nullptr), smallFont(nullptr),
      state(GameState::LANGUAGE_SELECT), currentLanguage(Language::ENGLISH),
      gameMode(GameMode::SINGLE_PLAYER), currentLevel(1), moveCount(0), 
      roundIntroTimer(0), levelWinTimer(0), fightAnimTimer(0),
      running(true), isFullscreen(false),
      player1Hearts(3), player2Hearts(3), player1Matches(0), player2Matches(0),
      player1RoundWins(0), player2RoundWins(0),
      scaleX(1.0f), scaleY(1.0f), viewportX(0), viewportY(0),
      menuSelection(0), optionsSelection(0),
      starAnimTime(0),fpsTimer(0), fpsCounter(0), currentFPS(0){ //gridAnimTime(0) 
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TIC-TAC-TOE CYBER BATTLE - Enhanced" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    glowEffect.pulse = 0;
    glowEffect.intensity = 255;
    
    initSDL();
    createStarField();
    //createGridFloor();
    
    int boardSize = getBoardSize();
    board = std::make_unique<Board>(boardSize, renderer);
    player1 = std::make_shared<HumanPlayer>("PLAYER 1", CellState::X);
    player2 = std::make_shared<AIPlayer>("AI", CellState::O, Difficulty::EASY);
    std::static_pointer_cast<AIPlayer>(player2)->setBoard(board.get());
    currentPlayer = player1;
    
    placePowerups();
    
    std::cout << "[GameEngine] Initialization complete!" << std::endl;
}

GameEngine::~GameEngine() {
    std::cout << "[GameEngine] Shutting down..." << std::endl;
    cleanupSDL();
}

int GameEngine::getBoardSize() const {
    return std::min(3 + (currentLevel - 1), 20);
}

void GameEngine::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error("SDL Init failed");
    }
    
    if (TTF_Init() < 0) {
        throw std::runtime_error("TTF Init failed");
    }
    
    window = SDL_CreateWindow("TIC-TAC-TOE CYBER BATTLE",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window) {
        throw std::runtime_error("Window creation failed");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        throw std::runtime_error("Renderer creation failed");
    }

    SDL_RenderSetLogicalSize(renderer, 1280, 720);
    SDL_RenderSetIntegerScale(renderer, SDL_FALSE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arialbd.ttf", 100);
    normalFont = TTF_OpenFont("C:\\Windows\\Fonts\\arialbd.ttf", 45);
    smallFont = TTF_OpenFont("C:\\Windows\\Fonts\\arialbd.ttf", 30);
    
    updateMouseScale();
}

void GameEngine::cleanupSDL() {
    if (titleFont) TTF_CloseFont(titleFont);
    if (normalFont) TTF_CloseFont(normalFont);
    if (smallFont) TTF_CloseFont(smallFont);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    TTF_Quit();
    SDL_Quit();
}

void GameEngine::placePowerups() {
    cellPowerups.clear();
    int size = getBoardSize();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, size - 1);
    
    int numHearts = 2 + (currentLevel / 2);
    
    int numDoubleHearts = 1 + (currentLevel / 4);
    
    int numBombs = 2 + (currentLevel / 3);
    
    for (int i = 0; i < numHearts; i++) {
        int row = dist(gen);
        int col = dist(gen);
        cellPowerups[{row, col}] = CellPowerup::HEART;
    }
    
    for (int i = 0; i < numDoubleHearts; i++) {
        int row, col;
        do {
            row = dist(gen);
            col = dist(gen);
        } while (cellPowerups.find({row, col}) != cellPowerups.end());
        cellPowerups[{row, col}] = CellPowerup::DOUBLE_HEART;
    }
    
    for (int i = 0; i < numBombs; i++) {
        int row, col;
        do {
            row = dist(gen);
            col = dist(gen);
        } while (cellPowerups.find({row, col}) != cellPowerups.end());
        cellPowerups[{row, col}] = CellPowerup::BOMB;
    }
    
    std::cout << "[Powerups] Placed " << numHearts << " hearts, " 
              << numDoubleHearts << " double hearts, and " 
              << numBombs << " bombs" << std::endl;
}

void GameEngine::checkForPowerup(int row, int col) {
    auto it = cellPowerups.find({row, col});
    if (it != cellPowerups.end()) {
        CellPowerup powerup = it->second;
        cellPowerups.erase(it);
        
        if (powerup == CellPowerup::HEART) {
            if (currentPlayer == player1) {
                player1Hearts = std::min(player1Hearts + 1, MAX_HEARTS);
                std::cout << "[Powerup] Player 1 found a HEART! +1 Life" << std::endl;
            } else {
                player2Hearts = std::min(player2Hearts + 1, MAX_HEARTS);
                std::cout << "[Powerup] Player 2 found a HEART! +1 Life" << std::endl;
            }
        }
        else if (powerup == CellPowerup::DOUBLE_HEART) {
            if (currentPlayer == player1) {
                player1Hearts = std::min(player1Hearts + 2, MAX_HEARTS);
                std::cout << "[Powerup] Player 1 found a DOUBLE HEART! +2 Lives!" << std::endl;
            } else {
                player2Hearts = std::min(player2Hearts + 2, MAX_HEARTS);
                std::cout << "[Powerup] Player 2 found a DOUBLE HEART! +2 Lives!" << std::endl;
            }
        }
        else if (powerup == CellPowerup::BOMB) {
            if (currentPlayer == player1) {
                player1Hearts = std::max(player1Hearts - 1, 0);
                std::cout << "[Powerup] Player 1 hit a BOMB! -1 Life" << std::endl;
            } else {
                player2Hearts = std::max(player2Hearts - 1, 0);
                std::cout << "[Powerup] Player 2 hit a BOMB! -1 Life" << std::endl;
            }
        }
    }
}

void GameEngine::checkMatchesAndUpdateScore() {
    int xMatches = board->countMatches(CellState::X);
    int oMatches = board->countMatches(CellState::O);
    
    std::cout << "[Matches] X: " << xMatches << ", O: " << oMatches << std::endl;
    
    player1Matches = xMatches;
    player2Matches = oMatches;
    
    if (player1Hearts <= 0) {
        player2RoundWins++; 
        std::cout << "[GAME OVER] Player 1 ran out of hearts! Player 2 WINS!" << std::endl;
        menuSelection = 0;
        state = GameState::GAME_OVER;
        return;
    }
    
    if (player2Hearts <= 0) {
        player1RoundWins++;  
        std::cout << "[GAME OVER] Player 2 ran out of hearts! Player 1 WINS!" << std::endl;
        menuSelection = 0;
        state = GameState::GAME_OVER;
        return;
    }
    
    if (board->isFull()) {
        if (xMatches > oMatches) {
            std::cout << "[Round] Player 1 WINS with " << xMatches << " matches!" << std::endl;
            player1RoundWins++;  
            player2Hearts--;
            if (player2Hearts <= 0) {
                menuSelection = 0;
                state = GameState::GAME_OVER;
            } else {
                nextRound();
            }
        } else if (oMatches > xMatches) {
            std::cout << "[Round] Player 2 WINS with " << oMatches << " matches!" << std::endl;
            player2RoundWins++;
            player1Hearts--;
            if (player1Hearts <= 0) {
                menuSelection = 0;
                state = GameState::GAME_OVER;
            } else {
                nextRound();
            }
        } else {
            std::cout << "[Round] DRAW! Both have " << xMatches << " matches. Replay!" << std::endl;

            board->reset();
            placePowerups();
            currentPlayer = player1;
            moveCount = 0;
            player1Matches = 0;
            player2Matches = 0;
        }
    }
}

std::string GameEngine::getText(const std::string& key) {
    static std::map<std::string, std::map<Language, std::string>> translations = {
        {"title", {
            {Language::ENGLISH, "TIC TAC TOE"},
            {Language::TURKISH, "TIC TAC TOE"},
            {Language::POLISH, "KOLKO I KRZYZYK"}
        }},
        {"cyber_challenge", {
            {Language::ENGLISH, "CYBER BATTLE"},
            {Language::TURKISH, "SIBER MUCADELE"},
            {Language::POLISH, "CYBER BITWA"}
        }},
        {"start_game", {
            {Language::ENGLISH, "START GAME"},
            {Language::TURKISH, "OYUNA BASLA"},
            {Language::POLISH, "START GRY"}
        }},
        {"options", {
            {Language::ENGLISH, "OPTIONS"},
            {Language::TURKISH, "SECENEKLER"},
            {Language::POLISH, "OPCJE"}
        }},
        {"exit_game", {
            {Language::ENGLISH, "EXIT GAME"},
            {Language::TURKISH, "CIKIS"},
            {Language::POLISH, "WYJSCIE"}
        }},
        {"language", {
            {Language::ENGLISH, "LANGUAGE"},
            {Language::TURKISH, "DIL"},
            {Language::POLISH, "JEZYK"}
        }},
        {"music", {
            {Language::ENGLISH, "MUSIC"},
            {Language::TURKISH, "MUZIK"},
            {Language::POLISH, "MUZYKA"}
        }},
        {"on", {
            {Language::ENGLISH, "ON"},
            {Language::TURKISH, "ACIK"},
            {Language::POLISH, "WLACZONA"}
        }},
        {"off", {
            {Language::ENGLISH, "OFF"},
            {Language::TURKISH, "KAPALI"},
            {Language::POLISH, "WYLACZONA"}
        }},
        {"back", {
            {Language::ENGLISH, "BACK"},
            {Language::TURKISH, "GERI"},
            {Language::POLISH, "POWROT"}
        }},
        {"select_mode", {
            {Language::ENGLISH, "SELECT GAME MODE"},
            {Language::TURKISH, "OYUN MODUNU SEC"},
            {Language::POLISH, "WYBIERZ TRYB GRY"}
        }},
        {"vs_ai", {
            {Language::ENGLISH, "VS AI"},
            {Language::TURKISH, "AI'YE KARSI"},
            {Language::POLISH, "PRZECIW AI"}
        }},
        {"vs_player", {
            {Language::ENGLISH, "VS PLAYER"},
            {Language::TURKISH, "2 OYUNCU"},
            {Language::POLISH, "2 GRACZY"}
        }},
        {"player_1", {
            {Language::ENGLISH, "PLAYER 1"},
            {Language::TURKISH, "OYUNCU 1"},
            {Language::POLISH, "GRACZ 1"}
        }},
        {"player_2", {
            {Language::ENGLISH, "PLAYER 2"},
            {Language::TURKISH, "OYUNCU 2"},
            {Language::POLISH, "GRACZ 2"}
        }},
        {"vs", {
            {Language::ENGLISH, "VS"},
            {Language::TURKISH, "VS"},
            {Language::POLISH, "VS"}
        }},
        {"fight", {
            {Language::ENGLISH, "FIGHT!"},
            {Language::TURKISH, "SAVAS!"},
            {Language::POLISH, "WALCZ!"}
        }},
        {"level", {
            {Language::ENGLISH, "LEVEL"},
            {Language::TURKISH, "SEVIYE"},
            {Language::POLISH, "POZIOM"}
        }},
        {"matches", {
            {Language::ENGLISH, "MATCHES"},
            {Language::TURKISH, "ESLESME"},
            {Language::POLISH, "DOPASOWANIA"}
        }},
        {"game_over", {
            {Language::ENGLISH, "GAME OVER"},
            {Language::TURKISH, "OYUN BITTI"},
            {Language::POLISH, "KONIEC GRY"}
        }},
        {"continue", {
            {Language::ENGLISH, "CONTINUE?"},
            {Language::TURKISH, "DEVAM?"},
            {Language::POLISH, "KONTYNUOWAC?"}
        }},
        {"yes", {
            {Language::ENGLISH, "YES"},
            {Language::TURKISH, "EVET"},
            {Language::POLISH, "TAK"}
        }},
        {"no", {
            {Language::ENGLISH, "NO"},
            {Language::TURKISH, "HAYIR"},
            {Language::POLISH, "NIE"}
        }},
        {"wins", {
            {Language::ENGLISH, "WINS!"},
            {Language::TURKISH, "KAZANDI!"},
            {Language::POLISH, "WYGRAL!"}
        }},
        {"draw", {
            {Language::ENGLISH, "DRAW!"},
            {Language::TURKISH, "BERABERE!"},
            {Language::POLISH, "REMIS!"}
        }}
    };
    
    auto keyIt = translations.find(key);
    if (keyIt != translations.end()) {
        auto langIt = keyIt->second.find(currentLanguage);
        if (langIt != keyIt->second.end()) {
            return langIt->second;
        }
    }
    return key;
}

void GameEngine::renderHearts() {
    int heartY = 20;
    int heartSpacing = 70;
    
    SDL_Color p1Color = {255, 100, 150, 255};
    for (int i = 0; i < MAX_HEARTS; i++) {
        bool isFilled = (i < player1Hearts);
        renderPixelHeart(50 + i * heartSpacing, heartY, isFilled, p1Color);
    }
    
    SDL_Color labelColor = {150, 220, 255, 255};
    renderText(getText("player_1"), 50, heartY + 70, labelColor, smallFont);
    
    SDL_Color p2Color = {100, 200, 255, 255};
    for (int i = 0; i < MAX_HEARTS; i++) {
        bool isFilled = (i < player2Hearts);
        renderPixelHeart(1280 - 240 + i * heartSpacing, heartY, isFilled, p2Color);
    }
    
    std::string p2Label = (gameMode == GameMode::SINGLE_PLAYER) ? "AI" : getText("player_2");
    int p2TextW, p2TextH;
    TTF_SizeUTF8(smallFont, p2Label.c_str(), &p2TextW, &p2TextH);
    renderText(p2Label, 1280 - 240, heartY + 70, labelColor, smallFont);
}

void GameEngine::renderScoreboard() {
    std::string levelText = getText("level") + " " + std::to_string(currentLevel);
    std::string boardSizeText = std::to_string(getBoardSize()) + "x" + std::to_string(getBoardSize());
    
    SDL_Color infoColor = {100, 255, 255, 255};
    renderText(levelText, 1050, 150, infoColor, smallFont);
    renderText(boardSizeText, 1050, 185, infoColor, smallFont);
    
    std::string scoreText = "SCORE: " + std::to_string(player1RoundWins) + 
                            " - " + std::to_string(player2RoundWins);
    SDL_Color scoreColor = {255, 255, 100, 255};
    renderCenteredText(scoreText, 20, scoreColor, normalFont);
    
    std::string xMatchText = "X " + getText("matches") + ": " + std::to_string(player1Matches);
    std::string oMatchText = "O " + getText("matches") + ": " + std::to_string(player2Matches);
    
    int matchTextX = (currentLanguage == Language::POLISH) ? 15 : 30;
    
    SDL_Color xColor = {255, 80, 80, 255};
    SDL_Color oColor = {80, 200, 255, 255};
    renderText(xMatchText, matchTextX, 220, xColor, smallFont);
    renderText(oMatchText, matchTextX, 255, oColor, smallFont);
}

void GameEngine::renderGame() {
    renderCyberpunkBackground();
    
    renderHearts();
    renderScoreboard();
    
    int size = getBoardSize();
    int maxBoardSize = 600;
    int boardWidth = std::min(maxBoardSize, 1280 - 400);
    int boardHeight = boardWidth;
    int boardOffsetX = (1280 - boardWidth) / 2;
    int boardOffsetY = (720 - boardHeight) / 2 + 20;
    
    SDL_SetRenderDrawColor(renderer, 20, 30, 50, 200);
    SDL_Rect boardBg = {boardOffsetX - 15, boardOffsetY - 15, boardWidth + 30, boardHeight + 30};
    SDL_RenderFillRect(renderer, &boardBg);
    
    SDL_SetRenderDrawColor(renderer, 100, 200, 255, 180);
    for (int i = 0; i < 4; i++) {
        SDL_Rect border = {boardOffsetX - 15 - i, boardOffsetY - 15 - i, 
                           boardWidth + 30 + 2*i, boardHeight + 30 + 2*i};
        SDL_RenderDrawRect(renderer, &border);
    }
    
    board->render(boardOffsetX, boardOffsetY);

    if (player1Matches > 0) {
        board->renderWinningLines(boardOffsetX, boardOffsetY, CellState::X);
    }
    if (player2Matches > 0) {
        board->renderWinningLines(boardOffsetX, boardOffsetY, CellState::O);
    }
    std::string fpsText = "FPS: " + std::to_string(currentFPS);
    SDL_Color fpsColor = {100, 255, 100, 255};
    renderText(fpsText, 1150, 640, fpsColor, smallFont);
}

void GameEngine::renderVSScreen() {
    renderCyberpunkBackground();
    
    SDL_Color p1Color = {255, 100, 150, 255};
    int p1HeartStartX = 200; 
    for (int i = 0; i < player1Hearts; i++) {
        renderPixelHeart(p1HeartStartX + i * 80, 320, true, p1Color);
    }
    
    SDL_Color p2Color = {100, 200, 255, 255};
    int p2HeartStartX = 1280 - 440;  
    for (int i = 0; i < player2Hearts; i++) {
        renderPixelHeart(p2HeartStartX + i * 80, 320, true, p2Color);
    }
    
    SDL_Color labelColor = {255, 255, 255, 255};
    renderText(getText("player_1"), p1HeartStartX, 250, labelColor, normalFont);
    
    SDL_Color vsColor = {255, 255, 100, 255};
    renderCenteredText(getText("vs"), 360, vsColor, titleFont);
    
    std::string p2Text = (gameMode == GameMode::SINGLE_PLAYER) ? "AI" : getText("player_2");
    renderText(p2Text, p2HeartStartX, 250, labelColor, normalFont);
}

void GameEngine::nextRound() {
    currentLevel++;
    int newSize = getBoardSize();
    board = std::make_unique<Board>(newSize, renderer);
    
    if (gameMode == GameMode::SINGLE_PLAYER) {
        std::static_pointer_cast<AIPlayer>(player2)->setBoard(board.get());
        std::static_pointer_cast<AIPlayer>(player2)->setDifficulty(getLevelDifficulty());
    }
    
    currentPlayer = player1;
    moveCount = 0;
    player1Matches = 0;
    player2Matches = 0;
    placePowerups();
    state = GameState::ROUND_INTRO;
    roundIntroTimer = 0;
}

void GameEngine::resetGame() {
    currentLevel = 1;
    player1Hearts = MAX_HEARTS;
    player2Hearts = MAX_HEARTS;
    player1Matches = 0;
    player2Matches = 0;
    player1RoundWins = 0;  
    player2RoundWins = 0;  
    nextRound();
}

void GameEngine::startNewGame() {
    currentLevel = 1;
    player1Hearts = MAX_HEARTS;
    player2Hearts = MAX_HEARTS;
    player1Matches = 0;
    player2Matches = 0;
    player1RoundWins = 0; 
    player2RoundWins = 0;
    
    int boardSize = getBoardSize();
    board = std::make_unique<Board>(boardSize, renderer);
    
    if (gameMode == GameMode::SINGLE_PLAYER) {
        player2 = std::make_shared<AIPlayer>("AI", CellState::O, Difficulty::MEDIUM);
        std::static_pointer_cast<AIPlayer>(player2)->setBoard(board.get());
    } else {
        player2 = std::make_shared<HumanPlayer>("PLAYER 2", CellState::O);
    }
    
    currentPlayer = player1;
    placePowerups();
    state = GameState::VS_SCREEN;
}

Difficulty GameEngine::getLevelDifficulty() {
    if (currentLevel <= 3) return Difficulty::EASY;
    if (currentLevel <= 7) return Difficulty::MEDIUM;
    if (currentLevel <= 15) return Difficulty::HARD;
    return Difficulty::EXPERT;
}

void GameEngine::switchPlayer() {
    currentPlayer = (currentPlayer == player1) ? player2 : std::static_pointer_cast<Player>(player1);
}

void GameEngine::run() {
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    
    std::cout << "\n[Game] Main loop started!" << std::endl;
    
    while (running) {
        Uint32 frameStart = SDL_GetTicks();
        
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        fpsCounter++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            currentFPS = fpsCounter;
            fpsCounter = 0;
            fpsTimer = 0;
        }
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F11) {
                    isFullscreen = !isFullscreen;
                    SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    updateMouseScale();
                }
                
                if (event.key.keysym.sym == SDLK_ESCAPE && state != GameState::PLAYING) {
                    if (state == GameState::OPTIONS || state == GameState::MODE_SELECT) {
                        menuSelection = 0;
                        state = GameState::MENU;
                    }
                }
            }
            
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                updateMouseScale();
            }
            
            switch (state) {
                case GameState::LANGUAGE_SELECT:
                    handleLanguageInput(event);
                    break;
                case GameState::MENU:
                    handleMenuInput(event);
                    break;
                case GameState::OPTIONS:
                    handleOptionsInput(event);
                    break;
                case GameState::MODE_SELECT:
                    handleModeSelectionInput(event);
                    break;
                case GameState::PLAYING:
                    handleGameInput(event);
                    break;
                case GameState::GAME_OVER:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT) {
                            menuSelection = 1 - menuSelection;
                        } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                            if (menuSelection == 0) {
                                resetGame();
                            } else {
                                menuSelection = 0;
                                state = GameState::MENU;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        
        updateStarField(deltaTime);
        //updateGridFloor(deltaTime);
        updateNeonGlow(deltaTime);
        starAnimTime += deltaTime;
        //gridAnimTime += deltaTime;
        
        if (state == GameState::VS_SCREEN) {
            static float vsTimer = 0;
            vsTimer += deltaTime;
            if (vsTimer >= 2.5f) {
                vsTimer = 0;
                state = GameState::ROUND_INTRO;
                roundIntroTimer = 0;
            }
        } else if (state == GameState::ROUND_INTRO) {
            roundIntroTimer += deltaTime;
            if (roundIntroTimer >= 2.0f) {
                state = GameState::PLAYING;
            }
        } else if (state == GameState::PLAYING) {
            // AI turn
            if (gameMode == GameMode::SINGLE_PLAYER && currentPlayer != player1) {
                auto aiPlayer = std::static_pointer_cast<AIPlayer>(player2);
                auto move = aiPlayer->makeMove();
                if (move.first != -1) {
                    SDL_Delay(300);
                    
                    if (board->placeMark(move.first, move.second, currentPlayer->getSymbol())) {
                        checkForPowerup(move.first, move.second);
                        moveCount++;
                
                        moveCount++;
                        
                        player1Matches = board->countMatches(CellState::X);
                        player2Matches = board->countMatches(CellState::O);
                        
                        checkMatchesAndUpdateScore();
                        
                        if (state == GameState::PLAYING) { 
                            switchPlayer();
                        }
                    }
                }
            } else {

                auto move = currentPlayer->makeMove();
                if (move.first != -1) {
                    if (board->placeMark(move.first, move.second, currentPlayer->getSymbol())) {
                        checkForPowerup(move.first, move.second);
                        moveCount++;
                        
                        player1Matches = board->countMatches(CellState::X);
                        player2Matches = board->countMatches(CellState::O);
                        
                        checkMatchesAndUpdateScore();
                        
                        if (state == GameState::PLAYING) { 
                            switchPlayer();
                        }
                    }
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        switch (state) {
            case GameState::LANGUAGE_SELECT:
                renderLanguageSelection();
                break;
            case GameState::MENU:
                renderMenu();
                break;
            case GameState::OPTIONS:
                renderOptions();
                break;
            case GameState::MODE_SELECT:
                renderModeSelection();
                break;
            case GameState::VS_SCREEN:
                renderVSScreen();
                break;
            case GameState::ROUND_INTRO:
                renderRoundIntro();
                break;
            case GameState::PLAYING:
                renderGame();
                break;
            case GameState::GAME_OVER:
                renderGameOver();
                break;
            default:
                break;
        }
        
        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay((Uint32)(FRAME_DELAY - frameTime));
        }
    }
    
    std::cout << "[Game] Main loop ended" << std::endl;
}

void GameEngine::createStarField() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(0, 1280);
    std::uniform_real_distribution<float> yDist(0, 720);
    
    stars.clear();
    for (int i = 0; i < 400; i++) {
        StarParticle s;
        s.x = xDist(gen);
        s.y = yDist(gen);
        s.size = 1.0f + (gen() % 30) / 10.0f;
        s.brightness = 0.3f + (gen() % 70) / 100.0f;
        s.twinkleSpeed = 0.5f + (gen() % 30) / 10.0f;
        stars.push_back(s);
    }
}

/*
void GameEngine::createGridFloor() {
    for (int i = 0; i < 30; i++) {
        GridLine line;
        line.y = 500 + i * 20;
        line.speed = 10 + (i % 5) * 5;
        line.alpha = 50 + (i * 5);
        gridLines.push_back(line);
    }
}
*/

void GameEngine::updateStarField(float dt) {
    starAnimTime += dt;
    for (auto& s : stars) {
        float twinkle = sin(starAnimTime * s.twinkleSpeed);
        s.brightness = 0.4f + 0.6f * ((twinkle + 1.0f) / 2.0f);
    }
}

/*
void GameEngine::updateGridFloor(float dt) {
    for (auto& line : gridLines) {
        line.y += line.speed * dt * 30;
        if (line.y > 720) {
            line.y = 500;
        }
    }
}
*/

void GameEngine::updateNeonGlow(float dt) {
    glowEffect.pulse += dt * 4.0f;
    glowEffect.intensity = 200 + 55 * sin(glowEffect.pulse);
}

void GameEngine::renderCyberpunkBackground() {
    for (int y = 0; y < 720; y++) {
        float ratio = (float)y / 720.0f;
        Uint8 r = 5 + (int)(ratio * 10);
        Uint8 g = 10 + (int)(ratio * 25);
        Uint8 b = 20 + (int)(ratio * 40);
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, 1280, y);
    }
    
    for (const auto& s : stars) {
        Uint8 brightness = (Uint8)(s.brightness * 255);
        
        if (s.size > 2.5f) {
            SDL_SetRenderDrawColor(renderer, brightness/2, brightness/2, brightness, brightness/3);
            for (int dx = -3; dx <= 3; dx++) {
                for (int dy = -3; dy <= 3; dy++) {
                    if (dx*dx + dy*dy <= 9) {
                        SDL_RenderDrawPoint(renderer, (int)s.x + dx, (int)s.y + dy);
                    }
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, brightness, brightness, 255, brightness);
        int starSize = (int)s.size;
        for (int dx = -starSize; dx <= starSize; dx++) {
            for (int dy = -starSize; dy <= starSize; dy++) {
                SDL_RenderDrawPoint(renderer, (int)s.x + dx, (int)s.y + dy);
            }
        }
    }
    
    /*
    SDL_SetRenderDrawColor(renderer, 0, 180, 255, 100);
    int gridStartY = 480;
    
    for (size_t i = 0; i < gridLines.size(); i++) {
        if (gridLines[i].y < gridStartY) continue;
        
        float perspective = (gridLines[i].y - gridStartY) / (720.0f - gridStartY);
        int lineWidth = (int)(1280 * (0.3f + perspective * 0.7f));
        int offsetX = (1280 - lineWidth) / 2;
        
        SDL_SetRenderDrawColor(renderer, 0, 140 + (int)(perspective * 80), 255, gridLines[i].alpha);
        SDL_RenderDrawLine(renderer, offsetX, (int)gridLines[i].y, 
                          offsetX + lineWidth, (int)gridLines[i].y);
    }
    
    for (int x = 0; x <= 12; x++) {
        float ratio = x / 12.0f;
        int topX = 320 + (int)(ratio * 640);
        int bottomX = 640;
        
        SDL_SetRenderDrawColor(renderer, 0, 140, 255, 70);
        SDL_RenderDrawLine(renderer, topX, gridStartY, bottomX, 720);
    }
    */
}

void GameEngine::renderText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* font) {
    if (!font || !renderer) return;
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void GameEngine::renderCenteredText(const std::string& text, int y, SDL_Color color, TTF_Font* font) {
    if (!font || !renderer) return;
    
    int textW, textH;
    TTF_SizeUTF8(font, text.c_str(), &textW, &textH);
    int x = (1280 - textW) / 2;
    renderText(text, x, y, color, font);
}

void GameEngine::renderPixelHeart(int x, int y, bool filled, SDL_Color color) {
    static const int heart[7][7] = {
        {0,1,1,0,1,1,0},
        {1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1},
        {0,1,1,1,1,1,0},
        {0,0,1,1,1,0,0},
        {0,0,0,1,0,0,0}
    };
    
    int pixelSize = 8;
    
    if (filled) {

        SDL_SetRenderDrawColor(renderer, color.r / 3, color.g / 3, color.b / 3, 80);
        SDL_Rect glowRect = {x - 10, y - 10, 7 * pixelSize + 20, 7 * pixelSize + 20};
        SDL_RenderFillRect(renderer, &glowRect);
        
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 7; col++) {
                if (heart[row][col]) {
                    SDL_Rect pixel = {x + col * pixelSize, y + row * pixelSize, pixelSize - 1, pixelSize - 1};
                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    } else {

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 7; col++) {
                if (heart[row][col]) {
                    SDL_Rect pixel = {x + col * pixelSize, y + row * pixelSize, pixelSize - 1, pixelSize - 1};
                    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 200);
                    SDL_RenderDrawRect(renderer, &pixel);
                }
            }
        }
    }
}

void GameEngine::renderRoundIntro() {

    for (int y = 0; y < 720; y++) {
        float intensity = sin(roundIntroTimer * 3.0f + y * 0.01f) * 0.5f + 0.5f;
        Uint8 r = 120 + 135 * intensity;
        Uint8 g = 0;
        Uint8 b = 0;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, 1280, y);
    }
    
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 100);
    int centerX = 640, centerY = 360;
    for (int angle = 0; angle < 360; angle += 15) {
        float rad = angle * 3.14159f / 180.0f;
        int x = centerX + 800 * cos(rad);
        int y = centerY + 800 * sin(rad);
        SDL_RenderDrawLine(renderer, centerX, centerY, x, y);
    }
    
    float scale = 1.0f + 0.2f * sin(roundIntroTimer * 8.0f);
    SDL_Color fightGlow = {255, 50, 0, static_cast<Uint8>(180 * scale)};
    for (int i = 25; i > 0; i -= 3) {
        renderCenteredText(getText("fight"), 300 - i, fightGlow, titleFont);
    }
    
    SDL_Color fightColor = {255, 255, 255, 255};
    renderCenteredText(getText("fight"), 300, fightColor, titleFont);
}

void GameEngine::renderGameOver() {
    renderCyberpunkBackground();
    
    SDL_Color glowColor = {255, 0, 0, 120};
    for (int i = 20; i > 0; i -= 2) {
        renderCenteredText(getText("game_over"), 180 - i, glowColor, titleFont);
    }
    
    SDL_Color titleColor = {255, 100, 100, 255};
    renderCenteredText(getText("game_over"), 180, titleColor, titleFont);
    
    std::string winnerText;
    if (player1Hearts > player2Hearts || player2Hearts <= 0) {
        winnerText = getText("player_1") + " " + getText("wins");
    } else {
        winnerText = (gameMode == GameMode::SINGLE_PLAYER ? "AI " : getText("player_2") + " ") + getText("wins");
    }
    
    SDL_Color winColor = {150, 255, 150, 255};
    renderCenteredText(winnerText, 320, winColor, normalFont);
    
    renderCenteredText(getText("continue"), 480, {255, 255, 255, 255}, normalFont);
    
    SDL_Color selectedColor = {100, 255, 255, 255};
    SDL_Color normalColor = {150, 150, 200, 255};
    
    std::string yesText = getText("yes");
    std::string noText = getText("no");
    
    SDL_Color yesColor = (menuSelection == 0) ? selectedColor : normalColor;
    SDL_Color noColor = (menuSelection == 1) ? selectedColor : normalColor;
    
    if (menuSelection == 0) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
        SDL_Rect box = {400, 560, 200, 60};
        SDL_RenderFillRect(renderer, &box);
    }
    renderText(yesText, 450, 570, yesColor, normalFont);
    
    if (menuSelection == 1) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
        SDL_Rect box = {700, 560, 200, 60};
        SDL_RenderFillRect(renderer, &box);
    }
    renderText(noText, 750, 570, noColor, normalFont);
}

void GameEngine::updateMouseScale() {
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    
    int renderW = 1280;
    int renderH = 720;
    
    scaleX = (float)renderW / windowW;
    scaleY = (float)renderH / windowH;
}

std::pair<int, int> GameEngine::transformMouseCoords(int mouseX, int mouseY) {
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    
    int renderW = 1280;
    int renderH = 720;
    
    float windowAspect = (float)windowW / windowH;
    float renderAspect = (float)renderW / renderH;
    
    int vpX = 0, vpY = 0, vpW = windowW, vpH = windowH;
    
    if (windowAspect > renderAspect) {
        vpW = windowH * renderAspect;
        vpX = (windowW - vpW) / 2;
    } else {
        vpH = windowW / renderAspect;
        vpY = (windowH - vpH) / 2;
    }
    
    if (mouseX < vpX || mouseX > vpX + vpW || mouseY < vpY || mouseY > vpY + vpH) {
        return {-1, -1};
    }
    
    int logicalX = (mouseX - vpX) * renderW / vpW;
    int logicalY = (mouseY - vpY) * renderH / vpH;
    
    return {logicalX, logicalY};
}

void GameEngine::renderLanguageSelection() {
    renderCyberpunkBackground();
    
    SDL_Color glowColor = {0, 200, 255, 80};
    for (int offset = 20; offset > 0; offset -= 3) {
        renderCenteredText("SELECT LANGUAGE", 120 - offset, glowColor, titleFont);
    }
    
    SDL_Color titleColor = {150, 240, 255, 255};
    renderCenteredText("SELECT LANGUAGE", 120, titleColor, titleFont);
    
    SDL_Color optionColor = {200, 230, 255, 255};
    
    std::vector<std::string> options = {"1 - English", "2 - Turkish", "3 - Polish"};
    for (size_t i = 0; i < options.size(); i++) {
        int y = 320 + i * 90;
        
        SDL_SetRenderDrawColor(renderer, 0, 150, 220, 60);
        SDL_Rect hoverBox = {340, y - 15, 600, 70};
        SDL_RenderFillRect(renderer, &hoverBox);
        
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 180);
        SDL_RenderDrawRect(renderer, &hoverBox);
        
        renderCenteredText(options[i], y, optionColor, normalFont);
    }
}

void GameEngine::renderMenu() {
    renderCyberpunkBackground();
    
    SDL_Color glowColor = {0, 200, 255, 100};
    for (int offset = 15; offset > 0; offset -= 2) {
        renderCenteredText(getText("title"), 80 - offset, glowColor, titleFont);
    }
    
    SDL_Color titleColor = {100, 255, 255, 255};
    renderCenteredText(getText("title"), 80, titleColor, titleFont);
    
    SDL_Color subtitleColor = {150, 200, 255, 255};
    renderCenteredText(getText("cyber_challenge"), 180, subtitleColor, normalFont);
    
    std::vector<std::string> menuOptions = {"start_game", "options", "exit_game"};
    SDL_Color selectedColor = {100, 255, 255, 255};
    SDL_Color normalColor = {150, 150, 200, 255};
    
    for (size_t i = 0; i < menuOptions.size(); i++) {
        SDL_Color color = (i == menuSelection) ? selectedColor : normalColor;
        int y = 320 + i * 80;
        
        if (i == menuSelection) {
            SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
            SDL_Rect selectionBox = {390, y - 10, 500, 60};
            SDL_RenderFillRect(renderer, &selectionBox);
            
            SDL_SetRenderDrawColor(renderer, 100, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &selectionBox);
        }
        
        renderCenteredText(getText(menuOptions[i]), y, color, normalFont);
    }
    
    SDL_Color hintColor = {100, 100, 150, 255};
    renderCenteredText("Use Arrow Keys and Enter", 600, hintColor, smallFont);
}

void GameEngine::renderOptions() {
    renderCyberpunkBackground();
    
    SDL_Color titleColor = {100, 255, 255, 255};
    renderCenteredText(getText("options"), 100, titleColor, titleFont);
    
    std::string langText = getText("language") + ": ";
    if (currentLanguage == Language::ENGLISH) langText += "English";
    else if (currentLanguage == Language::TURKISH) langText += "Turkish";
    else langText += "Polish";
    
    SDL_Color selectedColor = {100, 255, 255, 255};
    SDL_Color normalColor = {150, 150, 200, 255};
    
    SDL_Color langColor = (optionsSelection == 0) ? selectedColor : normalColor;
    if (optionsSelection == 0) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
        SDL_Rect box = {340, 240, 600, 60};
        SDL_RenderFillRect(renderer, &box);
    }
    renderCenteredText(langText, 250, langColor, normalFont);
    
    std::string musicText = getText("music") + ": " + getText("off");
    SDL_Color musicColor = (optionsSelection == 1) ? selectedColor : normalColor;
    if (optionsSelection == 1) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
        SDL_Rect box = {340, 330, 600, 60};
        SDL_RenderFillRect(renderer, &box);
    }
    renderCenteredText(musicText, 340, musicColor, normalFont);
    
    SDL_Color backColor = (optionsSelection == 2) ? selectedColor : normalColor;
    if (optionsSelection == 2) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
        SDL_Rect box = {340, 420, 600, 60};
        SDL_RenderFillRect(renderer, &box);
    }
    renderCenteredText(getText("back"), 430, backColor, normalFont);
}

void GameEngine::renderModeSelection() {
    renderCyberpunkBackground();
    
    SDL_Color titleColor = {100, 255, 255, 255};
    renderCenteredText(getText("select_mode"), 100, titleColor, titleFont);
    
    std::vector<std::string> modes = {"vs_ai", "vs_player"};
    SDL_Color selectedColor = {100, 255, 255, 255};
    SDL_Color normalColor = {150, 150, 200, 255};
    
    for (size_t i = 0; i < modes.size(); i++) {
        SDL_Color color = (i == menuSelection) ? selectedColor : normalColor;
        int y = 300 + i * 100;
        
        if (i == menuSelection) {
            SDL_SetRenderDrawColor(renderer, 0, 150, 200, 100);
            SDL_Rect box = {340, y - 10, 600, 70};
            SDL_RenderFillRect(renderer, &box);
            SDL_SetRenderDrawColor(renderer, 100, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &box);
        }
        
        renderCenteredText(getText(modes[i]), y, color, normalFont);
    }
}

void GameEngine::handleLanguageInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_1:
                currentLanguage = Language::ENGLISH;
                state = GameState::MENU;
                break;
            case SDLK_2:
                currentLanguage = Language::TURKISH;
                state = GameState::MENU;
                break;
            case SDLK_3:
                currentLanguage = Language::POLISH;
                state = GameState::MENU;
                break;
        }
    }
}

void GameEngine::handleMenuInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                menuSelection = (menuSelection - 1 + 3) % 3;
                break;
            case SDLK_DOWN:
                menuSelection = (menuSelection + 1) % 3;
                break;
            case SDLK_RETURN:
            case SDLK_SPACE:
                if (menuSelection == 0) {
                    menuSelection = 0;
                    state = GameState::MODE_SELECT;
                } else if (menuSelection == 1) {
                    optionsSelection = 0;
                    state = GameState::OPTIONS;
                } else {
                    running = false;
                }
                break;
        }
    }
}

void GameEngine::handleOptionsInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                optionsSelection = (optionsSelection - 1 + 3) % 3;
                break;
            case SDLK_DOWN:
                optionsSelection = (optionsSelection + 1) % 3;
                break;
            case SDLK_LEFT:
            case SDLK_RIGHT:
                if (optionsSelection == 0) {
                    int lang = (int)currentLanguage;
                    if (event.key.keysym.sym == SDLK_RIGHT) {
                        lang = (lang + 1) % 3;
                    } else {
                        lang = (lang - 1 + 3) % 3;
                    }
                    currentLanguage = (Language)lang;
                }
                break;
            case SDLK_RETURN:
            case SDLK_SPACE:
                if (optionsSelection == 2) {
                    menuSelection = 0;
                    state = GameState::MENU;
                }
                break;
            case SDLK_ESCAPE:
                menuSelection = 0;
                state = GameState::MENU;
                break;
        }  
    } 
}

void GameEngine::handleModeSelectionInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                menuSelection = (menuSelection - 1 + 2) % 2;
                break;
            case SDLK_DOWN:
                menuSelection = (menuSelection + 1) % 2;
                break;
            case SDLK_RETURN:
            case SDLK_SPACE:
                if (menuSelection == 0) {
                    gameMode = GameMode::SINGLE_PLAYER;
                } else {
                    gameMode = GameMode::TWO_PLAYER;
                }
                startNewGame();
                break;
            case SDLK_ESCAPE:
                menuSelection = 0;
                state = GameState::MENU;
                break;
        }
    }
}

void GameEngine::handleGameInput(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (dynamic_cast<HumanPlayer*>(currentPlayer.get())) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            
            auto logicalCoords = transformMouseCoords(mouseX, mouseY);
            if (logicalCoords.first == -1) return;
            
            int size = getBoardSize();
            int maxBoardSize = 600;
            int boardWidth = std::min(maxBoardSize, 1280 - 400);
            int boardHeight = boardWidth;
            int boardOffsetX = (1280 - boardWidth) / 2;
            int boardOffsetY = (720 - boardHeight) / 2 + 20;
            
            auto cell = board->getClickedCell(logicalCoords.first, logicalCoords.second, 
                                              boardOffsetX, boardOffsetY);
            if (cell.first != -1) {
                std::static_pointer_cast<HumanPlayer>(currentPlayer)->setClick(cell.first, cell.second);
                

            }
        }
    }
}
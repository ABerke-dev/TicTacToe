#ifndef GAMEENGINE_HPP
#define GAMEENGINE_HPP

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "Board.hpp"
#include "Player.hpp"
#include "HumanPlayer.hpp"
#include "AIPlayer.hpp"
#include "GameState.hpp"

enum class Language {
    ENGLISH,
    TURKISH,
    POLISH
};

enum class GameMode {
    SINGLE_PLAYER,
    TWO_PLAYER
};

enum class CellPowerup {
    NONE,
    HEART,
    DOUBLE_HEART,  
    BOMB
};

class GameEngine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* normalFont;
    TTF_Font* smallFont;
    
    std::unique_ptr<Board> board;
    std::shared_ptr<HumanPlayer> player1;
    std::shared_ptr<Player> player2;
    std::shared_ptr<Player> currentPlayer;
    
    GameState state;
    Language currentLanguage;
    GameMode gameMode;
    int currentLevel;
    int moveCount;
    float roundIntroTimer;
    float levelWinTimer;
    float fightAnimTimer;
    bool running;
    bool isFullscreen;

    const int TARGET_FPS = 60;
    const float FRAME_DELAY = 1000.0f / 60.0f;

    float fpsTimer;
    int fpsCounter; 
    int currentFPS;
    
    int player1Hearts;
    int player2Hearts;
    int player1Matches; 
    int player2Matches;  
    const int MAX_HEARTS = 3;

    int player1RoundWins;  
    int player2RoundWins;  
    
    std::map<std::pair<int, int>, CellPowerup> cellPowerups;  
    
    float scaleX, scaleY;
    int viewportX, viewportY;
    
    struct StarParticle {
        float x, y;
        float size;
        float brightness;
        float twinkleSpeed;
    };
    std::vector<StarParticle> stars;

    /*    
    struct GridLine {
        float y;
        float speed;
        Uint8 alpha;
    };
    std::vector<GridLine> gridLines;
    */
    
    struct NeonGlowEffect {
        float pulse;
        Uint8 intensity;
    };
    NeonGlowEffect glowEffect;
    
    float starAnimTime;
    // float gridAnimTime;
    
    int menuSelection;
    int optionsSelection;
    
    void initSDL();
    void cleanupSDL();
    void createStarField();
    void createGridFloor();
    void updateStarField(float dt);
    void updateGridFloor(float dt);
    void updateNeonGlow(float dt);
    void renderCyberpunkBackground();
    void renderLanguageSelection();
    void renderMenu();
    void renderOptions();
    void renderModeSelection();
    void renderRoundIntro();
    void renderGame();
    void renderHearts();
    void renderScoreboard();
    void renderLevelWin();
    void renderGameOver();
    void renderVSScreen();
    void renderText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* font);
    void renderCenteredText(const std::string& text, int y, SDL_Color color, TTF_Font* font);
    void renderPixelHeart(int x, int y, bool filled, SDL_Color color);
    
    void handleLanguageInput(SDL_Event& event);
    void handleMenuInput(SDL_Event& event);
    void handleOptionsInput(SDL_Event& event);
    void handleModeSelectionInput(SDL_Event& event);
    void handleGameInput(SDL_Event& event);
    void updateMouseScale();
    std::pair<int, int> transformMouseCoords(int mouseX, int mouseY);
    
    void checkMatchesAndUpdateScore();  // NEW
    void switchPlayer();
    void nextRound();
    void resetGame();
    void startNewGame();
    void placePowerups();  // NEW
    void checkForPowerup(int row, int col);  // NEW
    int getBoardSize() const;
    Difficulty getLevelDifficulty();
    std::string getText(const std::string& key);
    
public:
    GameEngine();
    ~GameEngine();
    
    void run();
};

#endif
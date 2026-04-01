#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

enum class GameState {
    LANGUAGE_SELECT,
    MENU,
    OPTIONS,
    MODE_SELECT,
    VS_SCREEN,
    ROUND_INTRO,
    PLAYING,
    ROUND_WIN,
    PLAYER_X_WON,
    PLAYER_O_WON,
    DRAW,
    GAME_OVER
};

#endif
// main.cpp
#include "Game.h"
#include <SDL.h> // Include SDL for SDL_main

int main(int argc, char* argv[]) {
    Game game;
    game.initialize();
    game.run();
    game.cleanup();
    return 0;
}

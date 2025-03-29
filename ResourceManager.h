#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    void initialize();
    void cleanup();

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::vector<SDL_Texture*> playerTextures;
    std::vector<SDL_Texture*> bossTextures;
    SDL_Texture* tileTexture = nullptr;
    SDL_Texture* enemyTexture = nullptr;
    SDL_Texture* bulletTexture = nullptr;
    SDL_Texture* nenTexture = nullptr;
    SDL_Texture* startScreenTexture = nullptr;
    SDL_Texture* startButtonTexture = nullptr;
    SDL_Texture* startButton1Texture = nullptr;
    SDL_Texture* gameOverTexture = nullptr;
    SDL_Texture* restartButtonTexture = nullptr;
    SDL_Texture* quitButtonTexture = nullptr;
    SDL_Texture* restartButton1Texture = nullptr;
    SDL_Texture* quitButton1Texture = nullptr;
    TTF_Font* font = nullptr;
    Mix_Music* backgroundMusic = nullptr;
    Mix_Chunk* shootSound = nullptr;
};

#endif

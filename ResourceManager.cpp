#include "ResourceManager.h"
#include <iostream>

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    TTF_Init();

    window = SDL_CreateWindow("Platformer Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player1.png")));
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player2.png")));
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player3.png")));
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player4.png")));
    bossTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("boss1.png")));
    bossTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("boss2.png")));
    bossTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("boss3.png")));
    bossTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("boss4.png")));
    nenTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("nen.png"));
    tileTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("tile.png"));
    enemyTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("enemy1.png"));
    bulletTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("bullet.png"));
    gameOverTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("game_over.png"));
    quitButtonTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("quit_button.png"));
    quitButton1Texture = SDL_CreateTextureFromSurface(renderer, IMG_Load("quit_button1.png"));
    startScreenTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_screen.png"));
    startButtonTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_button.png"));
    startButton1Texture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_button1.png"));
    restartButtonTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("restart_button.png"));
    restartButton1Texture = SDL_CreateTextureFromSurface(renderer, IMG_Load("restart_button1.png"));
    backgroundMusic = Mix_LoadMUS("background.mp3");
    shootSound = Mix_LoadWAV("shoot.wav");

    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    } else {
        std::cerr << "Failed to load background music: " << Mix_GetError() << std::endl;
    }

    font = TTF_OpenFont("ARLRDBD.ttf", 20);
}

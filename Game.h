// Game.h
#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <sstream>
#include "Player.h"
#include "Tile.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Boss.h"

class Game {
public:
    Game();
    ~Game();

    void initialize();
    void run();
    void handleInput(SDL_Event& event);
    void update();
    void render();
    void cleanup();
    void resetGame();

private:
    static const int SCREEN_WIDTH = 5000;
    static const int SCREEN_HEIGHT = 800;
    static const int TILE_SIZE = 45;
    static const int CAMERA_WIDTH = 900;
    static const int CAMERA_HEIGHT = 600;
    //SDL
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    //texture
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
    TTF_Font* largeFont = nullptr;
    Mix_Music* backgroundMusic = nullptr;
    Mix_Chunk* shootSound = nullptr;

    // Các đối tượng trong game
    std::vector<Tile> tiles;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    Player player;
    Boss boss;
    SDL_Rect camera = {0, 0, 900, 600};

    // Trạng thái game
    bool moveLeft;
    bool moveRight;
    bool jump;
    bool shoot;
    bool enemySpawnActive;
    Uint32 lastEnemySpawnTime;
    bool bossSpawned;
    int bossesKilled;
    int cnt;
    bool gameStarted;
    bool gameOver;
    bool isHover;
    bool isHover1;
    bool isHover2;
    Uint32 startTime;
    Uint32 finalTime;

    // Frame animation
    int currentFrame = 0;
    Uint32 lastFrameTime = 0;
    const int FRAME_DELAY = 100;
    int currentBossFrame = 0;
    Uint32 lastBossFrameTime = 0;
    const int BOSS_FRAME_DELAY = 250;


    // Các hàm hỗ trợ
    void spawnEnemy();
    void updateEnemies();
    void spawnBullet();
    void updateBullets();
    void spawnBoss();
    void updateBoss();
    SDL_Texture* createTextTexture(const std::string& text, SDL_Color color, TTF_Font* useFont);
};

#endif

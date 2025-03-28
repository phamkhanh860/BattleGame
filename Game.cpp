//Game.cpp
#include "Game.h"
#include "Constants.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

Game::Game() :
    moveLeft(false),
    moveRight(false),
    jump(false),
    shoot(false),
    enemySpawnActive(false),
    lastEnemySpawnTime(0),
    bossSpawned(false),
    bossesKilled(0),
    cnt(0),
    gameStarted(false),
    gameOver(false),
    startTime(0),
    currentFrame(0),
    lastFrameTime(0),
    currentBossFrame(0),
    lastBossFrameTime(0)
{
}

Game::~Game() {
}
SDL_Texture* Game::createTextTexture(const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
void Game::initialize() {
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
    startScreenTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_screen.png"));
    startButtonTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_button.png"));
    restartButtonTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("restart_button.png"));
    backgroundMusic = Mix_LoadMUS("background.mp3");
    shootSound = Mix_LoadWAV("shoot.wav");

    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    } else {
        std::cerr << "Failed to load background music: " << Mix_GetError() << std::endl;
    }

    font = TTF_OpenFont("ARLRDBD.ttf", 20);

    player.x = 80;
    player.y = 500;
    player.vx = 0;
    player.vy = 0;
    player.onGround = false;
    player.lastDirection = 1;
    player.gravity = 0.35f;

    tiles = {
        {0, 0, 5000, 110},{0, 480, 220, 300},{4600, 0, 1000, 1000},{370, 420, 50, 400},{560, 380, 50, 400}, {780, 520, 430, 340},{1050, 480, 100, 80},
        {780, 410, 100, 30},{780, 520, 430, 340},{1300, 410, 50, 270},{1540, 530, 70, 240},{1540, 320, 50, 135},
        {1710, 590, 3700, 180},{1710, 105, 50, 430},{1710, 230, 400, 40},{2700, 230, 400, 40},{2080, 360, 650, 30},
        {2080, 350, 20, 30},{1890, 520, 200, 100},{1940, 440, 100, 80},{0, 740, 5000, 500},{2750, 250, 40, 40},
        {3100, 585, 40, 20},{3100, 0, 70, 400},{3100, 400, 70, 190},
    };

    srand(static_cast<unsigned int>(time(0)));
    startTime = SDL_GetTicks();
}

void Game::run() {
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            handleInput(event);
        }
        update();
        render();
        SDL_Delay(16);
    }
}

void Game::resetGame() {
    tiles = {
        {0, 0, 5000, 110},{0, 480, 220, 300},{4600, 0, 1000, 1000},{370, 420, 50, 400},{560, 380, 50, 400}, {780, 520, 430, 340},{1050, 480, 100, 80},
        {780, 410, 100, 30},{780, 520, 430, 340},{1300, 410, 50, 270},{1540, 530, 70, 240},{1540, 320, 50, 135},
        {1710, 590, 3700, 180},{1710, 105, 50, 430},{1710, 230, 400, 40},{2700, 230, 400, 40},{2080, 360, 650, 30},
        {2080, 350, 20, 30},{1890, 520, 200, 100},{1940, 440, 100, 80},{0, 740, 5000, 500},{2750, 250, 40, 40},
        {3100, 585, 40, 20},{3100, 0, 70, 400},{3100, 400, 70, 190},
    };
    enemies.clear();
    bullets.clear();
    player.x = 80;
    player.y = 500;
    player.vx = 0;
    player.vy = 0;
    player.onGround = false;
    player.lastDirection = 1;
    player.gravity = 0.35f;
    boss.active = false;
    bossSpawned = false;
    bossesKilled = 0;
    cnt = 0;
    moveLeft = false;
    moveRight = false;
    jump = false;
    shoot = false;
    enemySpawnActive = false;
    startTime = SDL_GetTicks(); // Reset time
    gameOver = false;
}

void Game::handleInput(SDL_Event& event) {
    if (!gameStarted) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_Rect startButtonRect = {CAMERA_WIDTH / 2 - 50, CAMERA_HEIGHT / 2 - 25, 100, 50};
            if (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h) {
                gameStarted = true;
                startTime = SDL_GetTicks(); // Bắt đầu tính thời gian khi game bắt đầu
            }
        }
        return; // Không xử lý các input khác nếu game chưa bắt đầu
    }
    if (gameOver) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_Rect restartButtonRect = {CAMERA_WIDTH / 2 - 150, CAMERA_HEIGHT / 2 + 50, 120, 60};
            if (mouseX >= restartButtonRect.x && mouseX <= restartButtonRect.x + restartButtonRect.w &&
                mouseY >= restartButtonRect.y && mouseY <= restartButtonRect.y + restartButtonRect.h) {
                resetGame();
                gameStarted = true;
            }
            SDL_Rect quitButtonRect = {CAMERA_WIDTH / 2 + 30, CAMERA_HEIGHT / 2 + 50, 120, 60};
            if (mouseX >= quitButtonRect.x && mouseX <= quitButtonRect.x + quitButtonRect.w &&
                mouseY >= quitButtonRect.y && mouseY <= quitButtonRect.y + quitButtonRect.h) {
                exit(0);
            }
        }
        return; // Không xử lý input khác nếu game over
    }
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT: moveLeft = true; player.lastDirection = -1; break;
            case SDLK_RIGHT: moveRight = true; player.lastDirection = 1; break;
            case SDLK_UP: jump = true; break;
            case SDLK_SPACE: shoot = true; break;
        }
    } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT: moveLeft = false; break;
            case SDLK_RIGHT: moveRight = false; break;
            case SDLK_UP: jump = false; break;
            case SDLK_SPACE: shoot = false; break;
        }
    }
}

void Game::spawnEnemy() {
    if (!enemySpawnActive) return;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastEnemySpawnTime > 1700) {
        enemies.push_back({3100 - TILE_SIZE, 180, -3.5, true});
        enemies.push_back({1770, 180, 3.5, true});
        lastEnemySpawnTime = currentTime;
    }
}

void Game::updateEnemies() {
    for (auto& enemy : enemies) {
        if (!enemy.active) continue;
        bool onGround = false;
        for (auto& tile : tiles) {
            SDL_Rect enemyRect = {static_cast<int>(enemy.x), static_cast<int>(enemy.y), TILE_SIZE, TILE_SIZE};
            SDL_Rect tileRect = {tile.x, tile.y, tile.w, tile.h};
            if (enemyRect.x < tileRect.x + tileRect.w &&
                enemyRect.x + enemyRect.w > tileRect.x &&
                enemyRect.y + enemyRect.h <= tileRect.y + 1 &&
                enemyRect.y + enemyRect.h >= tileRect.y) {
                onGround = true;
                break;
            }
        }
        if (!onGround) {
            enemy.y += 5;
        }
        enemy.x += enemy.vx;
        if (enemy.x < 0 || enemy.x + TILE_SIZE > SCREEN_WIDTH) {
            enemy.vx = -enemy.vx;
        }
        for (auto& tile : tiles) {
            SDL_Rect enemyRect = {static_cast<int>(enemy.x), static_cast<int>(enemy.y), TILE_SIZE, TILE_SIZE};
            SDL_Rect tileRect = {tile.x, tile.y, tile.w, tile.h};
            if (SDL_HasIntersection(&enemyRect, &tileRect)) {
                if (enemy.vx > 0) {
                    enemy.x = tile.x - TILE_SIZE;
                } else if (enemy.vx < 0) {
                    enemy.x = tile.x + tile.w;
                }
                enemy.vx = -enemy.vx;
            }
        }
        // Kiểm tra va chạm với người chơi
        SDL_Rect playerRect = {static_cast<int>(player.x), static_cast<int>(player.y), TILE_SIZE, TILE_SIZE};
        SDL_Rect enemyRect = {static_cast<int>(enemy.x), static_cast<int>(enemy.y), TILE_SIZE, TILE_SIZE};
        if (SDL_HasIntersection(&playerRect, &enemyRect)) {
            std::cout << "Game Over!" << std::endl;
            gameOver = true;
            return;
        }
    }
}

void Game::spawnBullet() {
    if (shoot) {
        bullets.push_back({player.x + TILE_SIZE / 10, player.y + TILE_SIZE / 10, player.lastDirection * 10, true});
        shoot = false; // Reset trạng thái bắn
        if (shootSound) {
            Mix_PlayChannel(-1, shootSound, 0);
        }
    }
}

void Game::updateBullets() {
    for (auto& bullet : bullets) {
        if (!bullet.active) continue;
        bullet.x += bullet.vx;
        // TƯỜNG
        if (bullet.x < 0 || bullet.x + TILE_SIZE > SCREEN_WIDTH) {
            bullet.active = false;
            continue;
        }
        // ENEMY
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            SDL_Rect bulletRect = {static_cast<int>(bullet.x), static_cast<int>(bullet.y), TILE_SIZE, TILE_SIZE};
            SDL_Rect enemyRect = {static_cast<int>(enemy.x), static_cast<int>(enemy.y), TILE_SIZE, TILE_SIZE};
            if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
                cnt++;
                bullet.active = false;
                enemy.active = false;
                break;
            }
        }
        if (cnt == 12) {
            tiles.pop_back();
            cnt = -99999;
        }
        // BOSS
        if (bossSpawned && boss.active) {
            SDL_Rect bulletRect = {static_cast<int>(bullet.x), static_cast<int>(bullet.y), TILE_SIZE, TILE_SIZE};
            SDL_Rect bossRect = {static_cast<int>(boss.x), static_cast<int>(boss.y), 150, 150};
            if (SDL_HasIntersection(&bulletRect, &bossRect)) {
                bullet.active = false;
                boss.health--;
                if (boss.health <= 0) {
                    boss.active = false;
                    bossSpawned = false;
                    bossesKilled++; // Tăng số lượng boss đã bị tiêu diệt
                    if (bossesKilled >= 4) {
                        std::cout << "You Win!" << std::endl;
                        gameOver = true;
                        return;
                    }
                }
                break;
            }
        }
    }
}

void Game::spawnBoss() {
    if (!bossSpawned) {
        boss.x = SCREEN_WIDTH - TILE_SIZE;
        boss.y = 440;
        boss.vx = 6;
        boss.active = true;
        boss.health = 15;
        bossSpawned = true;
        player.gravity = 0.13f;
    }
}

void Game::updateBoss() {
    if (!bossSpawned || !boss.active) return;
    boss.x += boss.vx;
    if (boss.x < 3170) {
        boss.x = 3170;
        boss.vx = -boss.vx;
    }
    if (boss.x + TILE_SIZE > 4500) {
        boss.x = 4500 - TILE_SIZE;
        boss.vx = -boss.vx;
    }
    // PLAYER
    SDL_Rect playerRect = {static_cast<int>(player.x), static_cast<int>(player.y), TILE_SIZE, TILE_SIZE};
    SDL_Rect bossRect = {static_cast<int>(boss.x), static_cast<int>(boss.y), 150, 150};

    if (SDL_HasIntersection(&playerRect, &bossRect)) {
        std::cout << "Game Over! Boss touched you." << std::endl;
        gameOver = true;
        return;
    }
    // BOSS ANIMATION
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastBossFrameTime > BOSS_FRAME_DELAY) {
        currentBossFrame = (currentBossFrame + 1) % 4;
        lastBossFrameTime = currentTime;
    }
}

void Game::update() {
    if (!gameStarted || gameOver) return; // Không update nếu game chưa bắt đầu hoặc đã kết thúc
    if (moveLeft) player.vx = -4;
    if (moveRight) player.vx = 4;
    if (!moveLeft && !moveRight) player.vx = 0;
    if (jump && player.onGround) {
        player.vy = -9;
        player.onGround = false;
    }
    player.vy += player.gravity;
    player.y += player.vy;
    player.onGround = false;
    for (auto& tile : tiles) {
        SDL_Rect playerRect = {static_cast<int>(player.x), static_cast<int>(player.y), TILE_SIZE, TILE_SIZE};
        SDL_Rect tileRect = {tile.x, tile.y, tile.w, tile.h};
        if (SDL_HasIntersection(&playerRect, &tileRect)) {
            if (player.vy > 0) {
                player.y = tile.y - TILE_SIZE;
                player.vy = 0;
                player.onGround = true;
            } else if (player.vy < 0) {
                player.y = tile.y + tile.h;
                player.vy = 0;
            }
        }
    }
    player.x += player.vx;
    for (auto& tile : tiles) {
        SDL_Rect playerRect = {static_cast<int>(player.x), static_cast<int>(player.y), TILE_SIZE, TILE_SIZE};
        SDL_Rect tileRect = {tile.x, tile.y, tile.w, tile.h};
        if (SDL_HasIntersection(&playerRect, &tileRect)) {
            if (player.vx > 0) {
                player.x = tile.x - TILE_SIZE;
                player.vx = 0;
            } else if (player.vx < 0) {
                player.x = tile.x + tile.w;
                player.vx = 0;
            }
        }
    }
    if (player.x < 0) player.x = 0;
    if (player.x + TILE_SIZE > SCREEN_WIDTH) player.x = SCREEN_WIDTH - TILE_SIZE;
    if (player.y < 0) player.y = 0;
    if (player.y + TILE_SIZE > SCREEN_HEIGHT) player.y = SCREEN_HEIGHT - TILE_SIZE;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFrameTime > FRAME_DELAY) {
        currentFrame = (currentFrame + 1) % 4;
        lastFrameTime = currentTime;
    }
    camera.x = (player.x + TILE_SIZE / 2) - CAMERA_WIDTH / 2;
    camera.y = (player.y + TILE_SIZE / 2) - CAMERA_HEIGHT / 2;
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x + CAMERA_WIDTH > SCREEN_WIDTH) camera.x = SCREEN_WIDTH - CAMERA_WIDTH;
    if (camera.y + CAMERA_HEIGHT > SCREEN_HEIGHT) camera.y = SCREEN_HEIGHT - CAMERA_HEIGHT;
    if (player.y == 695) {
        gameOver = true;
        return;
    }
    if (player.x >= 1710 && player.y <= 590) {
        enemySpawnActive = true;
    }
    if (player.x >= 3100) {
        spawnBoss();
    }
    spawnEnemy();
    spawnBullet();
    updateBullets();
    updateEnemies();
    updateBoss();
}

void Game::render() {
    SDL_RenderClear(renderer);
    if (!gameStarted) {
        SDL_Rect startScreenRect = {0, 0, CAMERA_WIDTH, CAMERA_HEIGHT};
        SDL_RenderCopy(renderer, startScreenTexture, nullptr, &startScreenRect);
        SDL_Rect startButtonRect = {CAMERA_WIDTH / 2 - 60, CAMERA_HEIGHT / 2 - 25, 120, 60};
        SDL_RenderCopy(renderer, startButtonTexture, nullptr, &startButtonRect);
    } else if (gameOver) {
        SDL_Rect gameOverRect = {0, 0, CAMERA_WIDTH, CAMERA_HEIGHT};
        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
        SDL_Rect restartButtonRect = {CAMERA_WIDTH / 2 - 150, CAMERA_HEIGHT / 2 + 50, 120, 60};
        SDL_RenderCopy(renderer, restartButtonTexture, nullptr, &restartButtonRect);
        SDL_Rect quitButtonRect = {CAMERA_WIDTH / 2 + 30, CAMERA_HEIGHT / 2 + 50, 120, 60};
        SDL_RenderCopy(renderer, quitButtonTexture, nullptr, &quitButtonRect);
    } else {
        SDL_Rect nenRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, nenTexture, nullptr, &nenRect);
        for (auto& tile : tiles) {
            SDL_Rect rect = {tile.x - camera.x, tile.y - camera.y, tile.w, tile.h};
            SDL_RenderCopy(renderer, tileTexture, nullptr, &rect);
        }
        // PLAYER FRAME
        SDL_Rect playerRect = {static_cast<int>(player.x - camera.x), static_cast<int>(player.y - camera.y), TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, playerTextures[currentFrame], nullptr, &playerRect);
        for (auto& enemy : enemies) {
            if (enemy.active) {
                SDL_Rect enemyRect = {static_cast<int>(enemy.x - camera.x), static_cast<int>(enemy.y - camera.y), TILE_SIZE, TILE_SIZE};
                SDL_RenderCopy(renderer, enemyTexture, nullptr, &enemyRect);
            }
        }
        for (auto& bullet : bullets) {
            if (bullet.active) {
                SDL_Rect bulletRect = {static_cast<int>(bullet.x - camera.x), static_cast<int>(bullet.y - camera.y), TILE_SIZE, TILE_SIZE};
                SDL_RenderCopy(renderer, bulletTexture, nullptr, &bulletRect);
            }
        }
        // BOSS FRAME
        if (bossSpawned && boss.active) {
            SDL_Rect bossRect = {static_cast<int>(boss.x - camera.x), static_cast<int>(boss.y - camera.y), 150, 150};
            SDL_RenderCopy(renderer, bossTextures[currentBossFrame], nullptr, &bossRect);
        }
        Uint32 currentTime = SDL_GetTicks() - startTime; // Tính thời gian đã trôi qua
        Uint32 seconds = currentTime / 1000; // Chuyển đổi sang giây
        std::stringstream timeText;
        timeText << "Time: " << seconds << "s";
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Texture* timeTexture = createTextTexture(timeText.str(), textColor);
        if (timeTexture) {
            int textWidth = 0, textHeight = 0;
            SDL_QueryTexture(timeTexture, nullptr, nullptr, &textWidth, &textHeight);
            SDL_Rect timeRect = {CAMERA_WIDTH - textWidth - 10, 10, textWidth, textHeight}; // Góc trên bên phải
            SDL_RenderCopy(renderer, timeTexture, nullptr, &timeRect);
            SDL_DestroyTexture(timeTexture);
        }
    }
    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    for (auto& texture : playerTextures) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyTexture(tileTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(bulletTexture);
    for (auto& texture : bossTextures) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyTexture(nenTexture);
    SDL_DestroyTexture(startScreenTexture);
    SDL_DestroyTexture(startButtonTexture);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(restartButtonTexture);
    SDL_DestroyTexture(quitButtonTexture);
    SDL_DestroyRenderer(renderer);
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(shootSound);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

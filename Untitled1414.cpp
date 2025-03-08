#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <SDL.h>
#include <SDL_mixer.h>

const int SCREEN_WIDTH = 1500;
const int SCREEN_HEIGHT = 900;
const int TANK_WIDTH = 85;
const int TANK_HEIGHT = 90;
const int TANK_SPEED = 10;
const int BULLET_WIDTH = 20;
const int BULLET_HEIGHT = 20;
const int BULLET_SPEED = 15;
const int ENEMY_WIDTH = 80;
const int ENEMY_HEIGHT = 80;
const int ENEMY_SPEED = 3;
const int ENEMY_SPAWN_RATE = 120;
const int RECOIL_DISTANCE = 0;
const float PI = 3.14159265358979323846;

struct Tank {
    float x, y;
    float angle;
};

struct Bullet {
    float x, y;
    float angle;
};

struct Enemy {
    float x, y;
};

bool init(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    *window = SDL_CreateWindow("Tank Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    SDL_SetRenderDrawColor(*renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    return true;
}

void renderTank(SDL_Renderer* renderer, const Tank& tank) {
    SDL_Rect tankRect = {
        (int)tank.x - TANK_WIDTH / 2,
        (int)tank.y - TANK_HEIGHT / 2,
        TANK_WIDTH,
        TANK_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF); // Màu xanh dương cho tank
    SDL_RenderFillRect(renderer, &tankRect);

    // Vẽ hướng của tank (một đường thẳng đại diện cho hướng)
    double angle = tank.angle + PI / 2;
    int endX = tank.x + cos(angle) * TANK_WIDTH / 2;
    int endY = tank.y + sin(angle) * TANK_HEIGHT / 2;
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF); // Màu đỏ cho hướng
    SDL_RenderDrawLine(renderer, tank.x, tank.y, endX, endY);
}

void renderBullet(SDL_Renderer* renderer, const Bullet& bullet) {
    SDL_Rect bulletRect = {
        static_cast<int>(bullet.x - BULLET_WIDTH / 2),
        static_cast<int>(bullet.y - BULLET_HEIGHT / 2),
        BULLET_WIDTH,
        BULLET_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF); // Màu đỏ cho đạn
    SDL_RenderFillRect(renderer, &bulletRect);
}

void renderEnemy(SDL_Renderer* renderer, const Enemy& enemy) {
    SDL_Rect enemyRect = {
        (int)enemy.x - ENEMY_WIDTH / 2,
        (int)enemy.y - ENEMY_HEIGHT / 2,
        ENEMY_WIDTH,
        ENEMY_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF); // Màu xanh lá cho enemy
    SDL_RenderFillRect(renderer, &enemyRect);
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!init(&window, &renderer)) {
        return 1;
    }

    int cnt = 0;
    bool gameOver = false;
    bool gameStarted = false;

    Tank playerTank;
    playerTank.x = SCREEN_WIDTH / 2;
    playerTank.y = SCREEN_HEIGHT / 2;
    playerTank.angle = 0;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    bool quit = false;
    SDL_Event e;
    bool up = false, down = false, left = false, right = false;
    int frameCount = 0;
    bool isRecoiling = false;
    int recoilTimer = 0;
    float lastAngle = 0;

    Mix_Music* backgroundMusic = Mix_LoadMUS("background.mp3");
    if (backgroundMusic == nullptr) {
        std::cout << "Failed to load background music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return 1;
    }
    Mix_PlayMusic(backgroundMusic, -1);

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (gameOver) {
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = e.button.x;
                    int mouseY = e.button.y;
                    if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 100 &&
                        mouseY >= SCREEN_HEIGHT / 2 + 50 && mouseY <= SCREEN_HEIGHT / 2 + 150) {
                        // Restart game
                        gameOver = false;
                        playerTank = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0 };
                        bullets.clear();
                        enemies.clear();
                        cnt = 0;
                    }
                    else if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 100 &&
                        mouseY >= SCREEN_HEIGHT / 2 + 200 && mouseY <= SCREEN_HEIGHT / 2 + 300) {
                        // Quit game
                        quit = true;
                    }
                }
            }
            if (!gameStarted) {
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    gameStarted = true;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_UP: up = true; lastAngle = -PI / 2; break;
                case SDLK_DOWN: down = true; lastAngle = PI / 2; break;
                case SDLK_LEFT: left = true; lastAngle = PI; break;
                case SDLK_RIGHT: right = true; lastAngle = 0; break;
                case SDLK_SPACE: {
                    Bullet newBullet;
                    newBullet.x = playerTank.x;
                    newBullet.y = playerTank.y;
                    newBullet.angle = lastAngle;
                    bullets.push_back(newBullet);
                    isRecoiling = true;
                    recoilTimer = 5;
                    break;
                }
                }
            }
            else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                case SDLK_UP: up = false; break;
                case SDLK_DOWN: down = false; break;
                case SDLK_LEFT: left = false; break;
                case SDLK_RIGHT: right = false; break;
                }
            }
        }

        if (gameStarted && !gameOver) {
            // Update tank position
            if (!isRecoiling) {
                if (up) playerTank.y -= TANK_SPEED;
                if (down) playerTank.y += TANK_SPEED;
                if (left) playerTank.x -= TANK_SPEED;
                if (right) playerTank.x += TANK_SPEED;

                // Keep tank within screen bounds
                playerTank.x = std::max(TANK_WIDTH / 2.0f, std::min(playerTank.x, SCREEN_WIDTH - TANK_WIDTH / 2.0f));
                playerTank.y = std::max(TANK_HEIGHT / 2.0f, std::min(playerTank.y, SCREEN_HEIGHT - TANK_HEIGHT / 2.0f));
            }
            else {
                if (recoilTimer > 0) {
                    playerTank.x -= RECOIL_DISTANCE * cos(lastAngle);
                    playerTank.y -= RECOIL_DISTANCE * sin(lastAngle);
                    recoilTimer--;
                }
                else {
                    isRecoiling = false;
                }
            }

            // Update bullet positions
            for (auto& bullet : bullets) {
                bullet.x += BULLET_SPEED * cos(bullet.angle);
                bullet.y += BULLET_SPEED * sin(bullet.angle);
            }

            // Spawn enemies
            frameCount+= 4;
            if (frameCount % ENEMY_SPAWN_RATE == 0) {
                Enemy newEnemy;
                newEnemy.x = rand() % (SCREEN_WIDTH - ENEMY_WIDTH) + ENEMY_WIDTH / 2;
                newEnemy.y = rand() % (SCREEN_HEIGHT - ENEMY_HEIGHT) + ENEMY_HEIGHT / 2;
                enemies.push_back(newEnemy);
            }

            // Move enemies towards the player
            for (auto& enemy : enemies) {
                if (enemy.x < playerTank.x) enemy.x += ENEMY_SPEED;
                if (enemy.x > playerTank.x) enemy.x -= ENEMY_SPEED;
                if (enemy.y < playerTank.y) enemy.y += ENEMY_SPEED;
                if (enemy.y > playerTank.y) enemy.y -= ENEMY_SPEED;
            }

            // Check bullet-enemy collisions
            for (int i = 0; i < bullets.size(); ++i) {
                for (int j = 0; j < enemies.size(); ++j) {
                    float dx = bullets[i].x - enemies[j].x;
                    float dy = bullets[i].y - enemies[j].y;
                    float distance = sqrt(dx * dx + dy * dy);
                    if (distance < (BULLET_WIDTH / 2 + ENEMY_WIDTH / 2)) {
                        cnt++;
                        bullets.erase(bullets.begin() + i);
                        enemies.erase(enemies.begin() + j);
                        i--;
                        break;
                    }
                }
            }

            // Check player-enemy collisions
            for (const auto& enemy : enemies) {
                float dx = playerTank.x - enemy.x;
                float dy = playerTank.y - enemy.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < (TANK_WIDTH / 2 + ENEMY_WIDTH / 2)) {
                    gameOver = true;
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        if (!gameStarted) {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(renderer);
            SDL_Rect startButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 100 };
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &startButton);
        }
        else if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(renderer);
            SDL_Rect restartButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 200, 100 };
            SDL_Rect quitButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 200, 200, 100 };
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &restartButton);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &quitButton);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(renderer);

            renderTank(renderer, playerTank);
            for (const auto& bullet : bullets) {
                renderBullet(renderer, bullet);
            }
            for (const auto& enemy : enemies) {
                renderEnemy(renderer, enemy);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    Mix_FreeMusic(backgroundMusic);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

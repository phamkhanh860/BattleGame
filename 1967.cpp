#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <sstream>

const int SCREEN_WIDTH = 4400;
const int SCREEN_HEIGHT = 800;
const int TILE_SIZE = 45;
const int CAMERA_WIDTH = 900;
const int CAMERA_HEIGHT = 600;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// Texture cho người chơi (4 frame)
std::vector<SDL_Texture*> playerTextures; // Thay thế playerTexture bằng vector
SDL_Texture* tileTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* bulletTexture = nullptr;
SDL_Texture* bossTexture = nullptr;
SDL_Texture* nenTexture = nullptr;
SDL_Texture* startScreenTexture = nullptr; // Texture cho màn hình bắt đầu
SDL_Texture* startButtonTexture = nullptr; // Texture cho nút bắt đầu
SDL_Texture* gameOverTexture = nullptr;    // Texture cho màn hình game over

TTF_Font* font = nullptr; // Font để hiển thị thời gian
SDL_Texture* timeTexture = nullptr; // Texture để hiển thị thời gian
Uint32 startTime = 0; // Thời gian bắt đầu

Mix_Music* backgroundMusic = nullptr; // Âm thanh nền
Mix_Chunk* shootSound = nullptr;

// Biến quản lý animation
int currentFrame = 0; // Frame hiện tại
Uint32 lastFrameTime = 0; // Thời gian của frame cuối cùng
const int FRAME_DELAY = 100; // Thời gian chuyển đổi giữa các frame (ms)

struct Player {
    float x, y;
    float vx, vy;
    bool onGround;
    int lastDirection; // 1: right, -1: left
    float gravity;     // Thêm thuộc tính trọng lực cho người chơi
};

struct Tile {
    int x, y, w, h;
};

struct Enemy {
    float x, y;
    float vx;
    bool active;
};

struct Bullet {
    float x, y;
    float vx;
    bool active;
};

struct Boss {
    float x, y;
    float vx;
    bool active;
    int health; // Số viên đạn cần để tiêu diệt boss
};

std::vector<Tile> tiles;
std::vector<Enemy> enemies;
std::vector<Bullet> bullets;
Player player;
Boss boss; // Thêm boss
bool moveLeft = false, moveRight = false, jump = false, shoot = false;
bool enemySpawnActive = false;
Uint32 lastEnemySpawnTime = 0;
bool bossSpawned = false; // Biến kiểm tra xem boss đã được spawn chưa
int bossesKilled = 0;      // Đếm số lượng boss đã bị tiêu diệt
bool gameStarted = false;  // Biến để kiểm tra xem game đã bắt đầu chưa
bool gameOver = false;     // Biến để kiểm tra xem game đã kết thúc chưa

SDL_Rect camera = {0, 0, CAMERA_WIDTH, CAMERA_HEIGHT};

SDL_Texture* createTextTexture(const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Thêm SDL_INIT_AUDIO
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3); // Khởi tạo SDL_mixer
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // Mở audio device
    TTF_Init();
    window = SDL_CreateWindow("Platformer Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CAMERA_WIDTH, CAMERA_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Tải các frame của người chơi
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player1.png")));
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player2.png")));
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player3.png")));
    playerTextures.push_back(SDL_CreateTextureFromSurface(renderer, IMG_Load("player4.png")));

    // Tải các texture từ file PNG
    tileTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("tile.png"));
    enemyTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("enemy1.png"));
    bulletTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("bullet.png"));
    bossTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("boss1.png"));
    nenTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("nen.png"));
    startScreenTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_screen.png")); // Tải texture màn hình bắt đầu
    startButtonTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("start_button.png")); // Tải texture nút bắt đầu
    gameOverTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("game_over.png"));      // Tải texture màn hình game over

    backgroundMusic = Mix_LoadMUS("background.mp3");
    // Tải âm thanh khi bắn đạn
    shootSound = Mix_LoadWAV("shoot.wav");
    // Phát âm thanh nền
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1); // -1 để lặp vô hạn
    }
    font = TTF_OpenFont("ARLRDBD.ttf", 20);
    // Khởi tạo người chơi
    player.x = 80;
    player.y = 500;
    player.vx = 0;
    player.vy = 0;
    player.onGround = false;
    player.lastDirection = 1; // Mặc định hướng ban đầu là sang phải
    player.gravity = 0.4f;     // Khởi tạo trọng lực của người chơi

    // Khởi tạo các tile
    tiles = {
        {0, 0, 4400, 110},{0, 480, 220, 300},{370, 420, 50, 400},{560, 380, 50, 400}, {780, 520, 430, 340},{1050, 480, 100, 80},
        {780, 410, 100, 30},{780, 520, 430, 340},{1300, 410, 50, 270},{1540, 530, 70, 240},{1540, 320, 50, 135},
        {1710, 590, 2800, 180},{1710, 105, 50, 430},{1710, 230, 400, 40},{2700, 230, 400, 40},{2080, 360, 650, 30},
        {2080, 350, 20, 30},{1890, 520, 200, 100},{1940, 440, 100, 80},{0, 740, 4400, 500},{2750, 250, 40, 40},
        {3100, 585, 40, 20},{3100, 0, 70, 400},{3100, 400, 70, 190},
    };

    srand(static_cast<unsigned int>(time(0)));
    startTime = SDL_GetTicks();
}

void handleInput(SDL_Event& event) {
    if (!gameStarted) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Kiểm tra xem click chuột có nằm trong vùng nút start hay không
            SDL_Rect startButtonRect = {CAMERA_WIDTH / 2 - 50, CAMERA_HEIGHT / 2 - 25, 100, 50}; // Vị trí và kích thước nút start
            if (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h) {
                gameStarted = true;
                startTime = SDL_GetTicks(); // Bắt đầu tính thời gian khi game bắt đầu
            }
        }
        return; // Không xử lý các input khác nếu game chưa bắt đầu
    }
    if (gameOver) {
        // Không xử lý input khi game over
        return;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT: moveLeft = true; player.lastDirection = -1; break;
            case SDLK_RIGHT: moveRight = true; player.lastDirection = 1; break;
            case SDLK_UP: jump = true; break;
            case SDLK_SPACE: shoot = true; break; // Nhấn phím Space để bắn
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

void spawnEnemy() {
    if (!enemySpawnActive) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastEnemySpawnTime > 1500) {
        enemies.push_back({3100 - TILE_SIZE, 180, -4, true});
        enemies.push_back({1770, 180, 4, true});

        lastEnemySpawnTime = currentTime;
    }
}

void spawnBullet() {
    if (shoot) {
        bullets.push_back({player.x + TILE_SIZE / 10, player.y + TILE_SIZE / 10, player.lastDirection * 10, true});
        shoot = false; // Reset trạng thái bắn
        if (shootSound) {
            Mix_PlayChannel(-1, shootSound, 0);
        }
    }
}

void updateBullets() {
    static int cnt = 0;
    for (auto& bullet : bullets) {
        if (!bullet.active) continue;

        // Di chuyển đạn
        bullet.x += bullet.vx;

        // Kiểm tra va chạm với tường
        if (bullet.x < 0 || bullet.x + TILE_SIZE > SCREEN_WIDTH) {
            bullet.active = false;
            continue;
        }
        // Kiểm tra va chạm với enemy
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
        if (cnt == 10) {
            tiles.pop_back();
            cnt = -99999;
        }

        // Kiểm tra va chạm với boss
        if (bossSpawned && boss.active) {
            SDL_Rect bulletRect = {static_cast<int>(bullet.x), static_cast<int>(bullet.y), TILE_SIZE, TILE_SIZE};
            SDL_Rect bossRect = {static_cast<int>(boss.x), static_cast<int>(boss.y), 200, 200};

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
                        //exit(0);
                    }
                }
                break;
            }
        }
    }
}

void updateEnemies() {
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
            //exit(0);
            return;
        }
    }
}

void spawnBoss() {
    if (!bossSpawned) {
        boss.x = SCREEN_WIDTH - TILE_SIZE;
        boss.y = 410;
        boss.vx = 6; // Tốc độ di chuyển của boss
        boss.active = true;
        boss.health = 20; // Boss cần 20 viên đạn để chết
        bossSpawned = true;

        // Giảm trọng lực khi boss xuất hiện
        player.gravity = 0.1f;
    }
}

void updateBoss() {
    if (!bossSpawned || !boss.active) return;

    // Di chuyển boss
    boss.x += boss.vx;

    // Giới hạn di chuyển của boss trong khoảng từ 3170 đến SCREEN_WIDTH
    if (boss.x < 3170) {
        boss.x = 3170;
        boss.vx = -boss.vx; // Đổi hướng nếu chạm biên trái
    }
    if (boss.x + TILE_SIZE > SCREEN_WIDTH) {
        boss.x = SCREEN_WIDTH - TILE_SIZE;
        boss.vx = -boss.vx; // Đổi hướng nếu chạm biên phải
    }

    // Kiểm tra va chạm với người chơi
    SDL_Rect playerRect = {static_cast<int>(player.x), static_cast<int>(player.y), TILE_SIZE, TILE_SIZE};
    SDL_Rect bossRect = {static_cast<int>(boss.x), static_cast<int>(boss.y), 200, 200};

    if (SDL_HasIntersection(&playerRect, &bossRect)) {
        std::cout << "Game Over! Boss touched you." << std::endl;
        gameOver = true;
        //exit(0);
        return;
    }
}

void update() {
    if (!gameStarted || gameOver) return; // Không update nếu game chưa bắt đầu hoặc đã kết thúc

    if (moveLeft) player.vx = -4;
    if (moveRight) player.vx = 4;
    if (!moveLeft && !moveRight) player.vx = 0;

    if (jump && player.onGround) {
        player.vy = -9;
        player.onGround = false;
    }

    player.vy += player.gravity; // Sử dụng trọng lực của người chơi
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

    // Cập nhật animation
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFrameTime > FRAME_DELAY) {
        currentFrame = (currentFrame + 1) % 4; // Chuyển đổi giữa 4 frame
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
        //exit(0);
    }

    if (player.x >= 1710 && player.y <= 590) {
        enemySpawnActive = true;
    }

    if (player.x >= 3100) { // Điều kiện để spawn boss
        spawnBoss();
    }

    spawnEnemy();
    spawnBullet();
    updateBullets();
    updateEnemies();
    updateBoss();
}

void render() {
    SDL_RenderClear(renderer);

    if (!gameStarted) {
        // Hiển thị màn hình bắt đầu
        SDL_Rect startScreenRect = {0, 0, CAMERA_WIDTH, CAMERA_HEIGHT};
        SDL_RenderCopy(renderer, startScreenTexture, nullptr, &startScreenRect);

        // Hiển thị nút bắt đầu
        SDL_Rect startButtonRect = {CAMERA_WIDTH / 2 - 50, CAMERA_HEIGHT / 2 - 25, 100, 50};
        SDL_RenderCopy(renderer, startButtonTexture, nullptr, &startButtonRect);
    } else if (gameOver) {
        // Hiển thị màn hình game over
        SDL_Rect gameOverRect = {0, 0, CAMERA_WIDTH, CAMERA_HEIGHT};
        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
    } else {
        // Hiển thị game
        SDL_Rect nenRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; // Kích thước toàn màn hình
        SDL_RenderCopy(renderer, nenTexture, nullptr, &nenRect);

        for (auto& tile : tiles) {
            SDL_Rect rect = {tile.x - camera.x, tile.y - camera.y, tile.w, tile.h};
            SDL_RenderCopy(renderer, tileTexture, nullptr, &rect);
        }

        // Hiển thị frame hiện tại của người chơi
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

        // Render boss
        if (bossSpawned && boss.active) {
            SDL_Rect bossRect = {static_cast<int>(boss.x - camera.x), static_cast<int>(boss.y - camera.y), 300, 200};
            SDL_RenderCopy(renderer, bossTexture, nullptr, &bossRect);
        }

        Uint32 currentTime = SDL_GetTicks() - startTime; // Tính thời gian đã trôi qua
        Uint32 seconds = currentTime / 1000; // Chuyển đổi sang giây
        std::stringstream timeText;
        timeText << "Thang ngu nay duoc: " << seconds << "s";
        SDL_Color textColor = {255, 255, 255, 255}; // Màu trắng
        SDL_Texture* timeTexture = createTextTexture(timeText.str(), textColor);
        if (timeTexture) {
            int textWidth = 0, textHeight = 0;
            SDL_QueryTexture(timeTexture, nullptr, nullptr, &textWidth, &textHeight);
            SDL_Rect timeRect = {CAMERA_WIDTH - textWidth - 10, 10, textWidth, textHeight}; // Góc trên bên phải
            SDL_RenderCopy(renderer, timeTexture, nullptr, &timeRect);
            SDL_DestroyTexture(timeTexture); // Giải phóng texture sau khi sử dụng
        }
    }

    SDL_RenderPresent(renderer);
}

void cleanup() {
    // Giải phóng các frame của người chơi
    for (auto& texture : playerTextures) {
        SDL_DestroyTexture(texture);
    }

    SDL_DestroyTexture(tileTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(bossTexture);
    SDL_DestroyTexture(nenTexture);
    SDL_DestroyTexture(startScreenTexture); // Giải phóng texture màn hình bắt đầu
    SDL_DestroyTexture(startButtonTexture); // Giải phóng texture nút bắt đầu
    SDL_DestroyTexture(gameOverTexture);    // Giải phóng texture màn hình game over
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

int main(int argc, char* argv[]) {
    initialize();

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

        if (gameOver && gameStarted) {
            SDL_Delay(3000); // Hiển thị màn hình game over trong 3 giây
            running = false;
        }
    }

    cleanup();
    return 0;
}

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
const int SCREEN_WIDTH = 1500;
const int SCREEN_HEIGHT = 900;
const int TANK_WIDTH = 85;
const int TANK_HEIGHT = 90;
const int TANK_SPEED = 10;
const int BULLET_WIDTH = 110;
const int BULLET_HEIGHT = 45;
const int BULLET_SPEED = 15;
const int ENEMY_WIDTH = 80;
const int ENEMY_HEIGHT = 130;
const int ENEMY_SPEED = 3;
const int ENEMY_SPAWN_RATE = 120;
const int RECOIL_DISTANCE = 0;
const float PI = 3.14159265358979323846;
struct Tank{
    float x, y;
    float angle;
};
struct Bullet{
    float x, y;
    float angle;
};
struct Enemy {
    float x, y;
    int currentFrame;      // Frame hiện tại (0 hoặc 1)
    Uint32 lastFrameTime;  // Thời điểm cuối cùng chuyển frame
    std::vector<SDL_Texture*> frames; // Danh sách các frame
};
bool init(SDL_Window** window, SDL_Renderer** renderer) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
        std::cout << "SDL_image could not initialize! Error: " << IMG_GetError() << std::endl;
        return false;
    }
    *window = SDL_CreateWindow("Tank Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(*window == nullptr){
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if(*renderer == nullptr){
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    SDL_SetRenderDrawColor(*renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    return true;
}
void renderTank(SDL_Renderer* renderer, SDL_Texture* tankTexture, const Tank& tank){
    SDL_Rect dstRect = {
        (int)tank.x - TANK_WIDTH / 2,
        (int)tank.y - TANK_HEIGHT / 2,
        TANK_WIDTH,
        TANK_HEIGHT
    };
    double degrees = (tank.angle + PI/2) * (180.0 / PI);
    SDL_RenderCopyEx(renderer, tankTexture, nullptr, &dstRect, degrees, nullptr, SDL_FLIP_NONE);
}
void renderBullet(SDL_Renderer* renderer, const Bullet& bullet, SDL_Texture* bulletTexture){
    SDL_Rect renderRect = {
        static_cast<int>(bullet.x - BULLET_WIDTH/2),
        static_cast<int>(bullet.y - BULLET_HEIGHT/2),
        BULLET_WIDTH,
        BULLET_HEIGHT
    };
    double degrees = bullet.angle * (180.0 / PI);
    // Vẽ đạn với góc quay
    SDL_RenderCopyEx(renderer, bulletTexture, nullptr, &renderRect, degrees, nullptr, SDL_FLIP_NONE);
    //SDL_RenderCopy(renderer, bulletTexture, nullptr, &renderRect);
}
void renderEnemy(SDL_Renderer* renderer, Enemy& enemy){
    if (enemy.frames.empty()) return;
    SDL_Rect dstRect = {
        (int)enemy.x - ENEMY_WIDTH/2,
        (int)enemy.y - ENEMY_HEIGHT/2,
        ENEMY_WIDTH,
        ENEMY_HEIGHT
    };
    // Lấy frame hiện tại
    SDL_Texture* currentTexture = enemy.frames[enemy.currentFrame];
    SDL_RenderCopy(renderer, currentTexture, nullptr, &dstRect);
}

int main(int argc, char* args[]){
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!init(&window, &renderer)){
        return 1;
    }
    int cnt = 0;
    bool gameOver = false; // Thêm dòng này cùng với các biến khác như gameStarted

    SDL_Texture* gameOverTexture = IMG_LoadTexture(renderer, "gameOver.png");
    SDL_Texture* bulletTexture = IMG_LoadTexture(renderer, "bullet.png");
    SDL_Texture* backgroundTexture = IMG_LoadTexture(renderer, "picture.png");
    SDL_Texture* tankTexture = IMG_LoadTexture(renderer, "tank.png");
    std::vector<SDL_Texture*> enemyFrames = {
        IMG_LoadTexture(renderer, "enemy_frame1.png"),
        IMG_LoadTexture(renderer, "enemy_frame2.png"),
        IMG_LoadTexture(renderer, "enemy_frame3.png"),
        IMG_LoadTexture(renderer, "enemy_frame4.png"),
    };
    SDL_Texture* gameStartTexture = IMG_LoadTexture(renderer, "gameStart.png");
    SDL_Texture* startButtonTexture = IMG_LoadTexture(renderer, "startButton.png"); // Load the start button image
    SDL_Texture* restartButtonTexture = IMG_LoadTexture(renderer, "restartButton.png");
    SDL_Texture* quitButtonTexture = IMG_LoadTexture(renderer, "quitButton.png");

    Tank playerTank;
    playerTank.x = SCREEN_WIDTH / 2;
    playerTank.y = SCREEN_HEIGHT / 2;
    playerTank.angle = 80;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    bool quit = false;
    SDL_Event e;
    bool up = false, down = false, left = false, right = false;
    int frameCount = 0;
    bool isRecoiling = false;
    int recoilTimer = 0;
    float lastAngle = 0;
    bool gameStarted = false;
    // Define the 'start button' area based on the startButtonTexture size and position
    SDL_Rect startButtonRect;
    SDL_QueryTexture(startButtonTexture, NULL, NULL, &startButtonRect.w, &startButtonRect.h); // Get the width and height of the start button texture
    SDL_Rect restartButton, quitButton;
    const int BUTTON_WIDTH = 200;
    const int BUTTON_HEIGHT = 120;
    restartButton = {
        SCREEN_WIDTH/2 - BUTTON_WIDTH - 50,
        SCREEN_HEIGHT/2 + 150,
        BUTTON_WIDTH,
        BUTTON_HEIGHT
    };
    quitButton = {
        SCREEN_WIDTH/2 + 50,
        SCREEN_HEIGHT/2 + 150,
        BUTTON_WIDTH,
        BUTTON_HEIGHT
    };
    float scaleFactor = 0.5f; // Adjust this value to change the button size
    startButtonRect.w = static_cast<int>(startButtonRect.w * scaleFactor);
    startButtonRect.h = static_cast<int>(startButtonRect.h * scaleFactor);
    // Center the (smaller) start button texture on the screen (adjust as needed)
    startButtonRect.x = SCREEN_WIDTH / 2 - startButtonRect.w / 2;
    startButtonRect.y = SCREEN_HEIGHT / 2 - startButtonRect.h / 2;

    Mix_Music* backgroundMusic = Mix_LoadMUS("background.mp3");
    if (backgroundMusic == nullptr) {
        std::cout << "Failed to load background music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return 1;
    }
    // Phát nhạc nền lặp lại vô hạn
    Mix_PlayMusic(backgroundMusic, -1);

    while(!quit){
        while(SDL_PollEvent(&e) != 0){
            if(e.type == SDL_QUIT){
                quit = true;
            }
            if(gameOver){
                if(e.type == SDL_MOUSEBUTTONDOWN){
                    int mouseX = e.button.x;
                    int mouseY = e.button.y;
                    if(mouseX >= restartButton.x && mouseX <= restartButton.x + restartButton.w &&
                        mouseY >= restartButton.y && mouseY <= restartButton.y + restartButton.h){
                        // Reset game state
                        gameOver = false;
                        playerTank = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 0};
                        playerTank.angle = 80;
                        bullets.clear();
                        enemies.clear();
                    }
                    else if(mouseX >= quitButton.x && mouseX <= quitButton.x + quitButton.w &&
                             mouseY >= quitButton.y && mouseY <= quitButton.y + quitButton.h){
                        std::cout << "SCORE: " << cnt;
                        quit = true;
                    }
                }
            }
             if(!gameStarted){
                if (e.type == SDL_MOUSEBUTTONDOWN){
                    if (e.button.button == SDL_BUTTON_LEFT){
                        int mouseX = e.button.x;
                        int mouseY = e.button.y;

                        // Check if the click is inside the 'start button' area
                        if (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                            mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h) {
                            gameStarted = true; // Start the game if the button is clicked
                        }
                    }
                }
            }
            else if(e.type == SDL_KEYDOWN){
                switch (e.key.keysym.sym){
                    case SDLK_UP: up = true; lastAngle = -PI / 2; break;
                    case SDLK_DOWN: down = true; lastAngle = PI / 2; break;
                    case SDLK_LEFT: left = true; lastAngle = PI; break; // Sửa thành PI để xoay 180 độ khi sang trái
                    case SDLK_RIGHT: right = true; lastAngle = 0; break;
                    case SDLK_SPACE:{
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
            } else if(e.type == SDL_KEYUP){
                switch(e.key.keysym.sym){
                    case SDLK_UP: up = false; break;
                    case SDLK_DOWN: down = false; break;
                    case SDLK_LEFT: left = false; break;
                    case SDLK_RIGHT: right = false; break;
                }
            }
        }

        // Update game logic only if the game has started
        if(gameStarted){
            // Cập nhật vị trí xe tăng
        if(!isRecoiling){
            if (up) playerTank.y -= TANK_SPEED;
            if (down) playerTank.y += TANK_SPEED;
            if (left) playerTank.x -= TANK_SPEED;
            if (right) playerTank.x += TANK_SPEED;

            // Giới hạn trong màn hình
            playerTank.x = std::max(TANK_WIDTH / 2.0f, std::min(playerTank.x, SCREEN_WIDTH - TANK_WIDTH / 2.0f));
            playerTank.y = std::max(TANK_HEIGHT / 2.0f, std::min(playerTank.y, SCREEN_HEIGHT - TANK_HEIGHT / 2.0f));
        } else{
            if(recoilTimer > 0){
                playerTank.x -= RECOIL_DISTANCE * cos(lastAngle);
                playerTank.y -= RECOIL_DISTANCE * sin(lastAngle);
                recoilTimer--;
            } else{
                isRecoiling = false;
            }
        }
        // Di chuyển đạn
        for(auto& bullet : bullets){
            bullet.x += BULLET_SPEED * cos(bullet.angle);
            bullet.y += BULLET_SPEED * sin(bullet.angle);
        }

        // Tạo kẻ thù ngẫu nhiên
        frameCount+=4;
        if(frameCount % ENEMY_SPAWN_RATE == 0){
            Enemy newEnemy;
            newEnemy.x = rand() % (SCREEN_WIDTH - ENEMY_WIDTH) + ENEMY_WIDTH / 2;
            newEnemy.y = rand() % (SCREEN_HEIGHT - ENEMY_HEIGHT) + ENEMY_HEIGHT / 2;
            newEnemy.currentFrame = 0;                       // Thêm dòng này
            newEnemy.lastFrameTime = SDL_GetTicks();         // Thêm dòng này
            newEnemy.frames = enemyFrames;                   // Thêm dòng này
            enemies.push_back(newEnemy);
        }

        // Di chuyển kẻ thù về phía người chơi (đơn giản)
        for(auto& enemy : enemies){
            if (enemy.x < playerTank.x) enemy.x += ENEMY_SPEED;
            if (enemy.x > playerTank.x) enemy.x -= ENEMY_SPEED;
            if (enemy.y < playerTank.y) enemy.y += ENEMY_SPEED;
            if (enemy.y > playerTank.y) enemy.y -= ENEMY_SPEED;
        }

        // Kiểm tra va chạm (đạn - kẻ thù)
        for(int i = 0; i < bullets.size(); ++i){
            for (int j = 0; j < enemies.size(); ++j) {
                float dx = bullets[i].x - enemies[j].x;
                float dy = bullets[i].y - enemies[j].y;
                float distance = sqrt(dx * dx + dy * dy);
                if(distance < (BULLET_WIDTH / 2 + ENEMY_WIDTH / 2)){
                    cnt++;
                    // Va chạm xảy ra: xóa đạn và kẻ thù
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    i--; //  Cập nhật chỉ số sau khi xóa
                    break; // Thoát khỏi vòng lặp kẻ thù vì đạn đã va chạm
                }
            }
        }
        // Kiểm tra va chạm (kẻ thù - người chơi)
        for(const auto& enemy : enemies){
             float dx = playerTank.x - enemy.x;
             float dy = playerTank.y - enemy.y;
             float distance = sqrt(dx * dx + dy * dy);
             if(distance < (TANK_WIDTH / 4 + ENEMY_WIDTH / 4)){
                gameOver = true;
             }
        }
        }
        // Vẽ
        SDL_RenderClear(renderer);
        if(!gameStarted){
            SDL_RenderCopy(renderer, gameStartTexture, nullptr, nullptr); // Render the Game Start screen
            SDL_RenderCopy(renderer, startButtonTexture, nullptr, &startButtonRect); // Render the start button on top
        }
         else if(gameOver){
        // Hiển thị màn hình Game Over và 2 nút
            SDL_RenderCopy(renderer, gameOverTexture, nullptr, nullptr);
            SDL_RenderCopy(renderer, restartButtonTexture, nullptr, &restartButton);
            SDL_RenderCopy(renderer, quitButtonTexture, nullptr, &quitButton);
        }
        else{
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
            renderTank(renderer, tankTexture, playerTank);
            for(const auto& bullet : bullets){
                renderBullet(renderer, bullet, bulletTexture);
            }
            for(auto& enemy : enemies){
                Uint32 currentTime = SDL_GetTicks();
                if(currentTime - enemy.lastFrameTime > 100){ // Chuyển frame mỗi 500ms
                    enemy.currentFrame = (enemy.currentFrame + 1) % 4;
                    enemy.lastFrameTime = currentTime;
                }
                renderEnemy(renderer, enemy); // Thêm dòng này để vẽ enemy
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Giải phóng tài nguyên
    for(auto& texture : enemyFrames){
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyTexture(restartButtonTexture);
    SDL_DestroyTexture(quitButtonTexture);
    SDL_DestroyTexture(startButtonTexture);
    SDL_DestroyTexture(gameStartTexture);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(tankTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(backgroundMusic);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

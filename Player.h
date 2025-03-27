#ifndef PLAYER_H
#define PLAYER_H

struct Player {
    float x, y;
    float vx, vy;
    bool onGround;
    int lastDirection; // 1: right, -1: left
    float gravity;
};

#endif

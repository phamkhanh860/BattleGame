#ifndef BOSS_H
#define BOSS_H

struct Boss {
    float x, y;
    float vx;
    bool active;
    int health;
    int maxHealth;
};

#endif

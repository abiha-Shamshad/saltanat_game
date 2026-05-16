#pragma once
#include <string>
#include "Constants.h"

class Entity {
public:
    int x, y;
    int hp, maxHp;
    int atk;
    std::string dir; // "up", "down", "left", "right"
    float flashTimer;
    bool alive;

    Entity(int startX, int startY, int maxHealth, int attack);
    virtual ~Entity() = default;
};

struct Inventory {
    int gold = 0;
    int potion = 0;
    int key = 0;
};

class Player : public Entity {
public:
    int lvl;
    int exp;
    int expNext;
    Inventory inventory;

    Player();
    void reset();
    void gainExp(int amount);
    void heal(int amount);
};

enum class EnemyType {
    Soldier,
    Archer,
    Djinn,
    Boss
};

class Enemy : public Entity {
public:
    EnemyType type;
    std::string name;
    float moveTimer;
    float moveDelay;
    float attackTimer;

    Enemy(EnemyType t, int startX, int startY, int floorNum);
};

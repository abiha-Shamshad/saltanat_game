#include "Entities.h"
#include <cstdlib>

Entity::Entity(int startX, int startY, int maxHealth, int attack)
    : x(startX), y(startY), hp(maxHealth), maxHp(maxHealth), atk(attack),
      dir("down"), flashTimer(0.0f), alive(true) {}

Player::Player() : Entity(0, 0, 100, 15), lvl(1), exp(0), expNext(30) {}

void Player::reset() {
    hp = 100;
    maxHp = 100;
    atk = 15;
    lvl = 1;
    exp = 0;
    expNext = 30;
    inventory = {0, 2, 1}; // start with 2 potions, 1 key
    dir = "down";
    alive = true;
}

void Player::gainExp(int amount) {
    exp += amount;
    while (exp >= expNext) {
        exp -= expNext;
        lvl++;
        atk += 5;
        maxHp += 20;
        hp = std::min(hp + 20, maxHp);
        expNext = (int)(expNext * 1.5f);
    }
}

void Player::heal(int amount) {
    hp = std::min(hp + amount, maxHp);
}

Enemy::Enemy(EnemyType t, int startX, int startY, int floorNum)
    : Entity(startX, startY, 10, 5), type(t), moveTimer(0.0f), attackTimer(0.0f) {
    
    moveDelay = 0.6f + ((rand() % 40) / 100.0f);
    
    switch (t) {
        case EnemyType::Soldier:
            name = "Mughal Soldier";
            maxHp = hp = 40 + floorNum * 10;
            atk = 8 + floorNum * 2;
            break;
        case EnemyType::Archer:
            name = "Archer";
            maxHp = hp = 25 + floorNum * 8;
            atk = 12 + floorNum * 3;
            break;
        case EnemyType::Djinn:
            name = "Djinn Guard";
            maxHp = hp = 35 + floorNum * 12;
            atk = 15 + floorNum * 3;
            break;
        case EnemyType::Boss:
            name = "Darbar-e-Khas Final Guard";
            maxHp = hp = 300;
            atk = 35;
            break;
    }
}

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "Constants.h"
#include "Map.h"
#include "Entities.h"

struct Message {
    std::string text;
    sf::Color color;
    float age;
};

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    
    void startGame();
    void nextFloor();
    void winGame();
    
    void generateLevel();
    void spawnEnemies();
    void spawnItems();
    
    void tryMove(int dx, int dy);
    void playerAttack();
    void enemyAttack(Enemy& e);
    void addMessage(const std::string& text, sf::Color color = sf::Color::Yellow);

    sf::RenderWindow window;
    GameState state;
    
    Map gameMap;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Item> items;
    
    int floor;
    int score;
    float moveTimer;
    float attackAnim;
    
    float camX, camY;

    std::map<std::string, sf::Texture> textures;
    sf::Font font;
    std::vector<Message> messages;
    void loadResources();
    sf::Vector2f cartesianToIsometric(float x, float y) const;
    void drawTile(const sf::Texture& tex, int x, int y, sf::Color tint = sf::Color::White);
    void drawSprite(const sf::Texture& tex, int x, int y, int size, bool flash = false);
    void drawHealthBar(const Entity& e);
    void drawHUD();
};

#include "Game.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

Game::Game() : window(sf::VideoMode(800, 600), "SALTANAT"), state(GameState::Menu) {
    srand(static_cast<unsigned int>(time(nullptr)));
    loadResources();
    window.setFramerateLimit(60);
}

void Game::loadResources() {
    auto loadTex = [&](const std::string& name, const std::string& path) {
        sf::Texture tex;
        if (tex.loadFromFile("assets/" + path)) {
            textures[name] = tex;
        }
    };
    loadTex("character", "charater.png");
    loadTex("enemy", "enemy.png");
    loadTex("archer", "enemy_arccher.png");
    loadTex("djinn", "enemy_jin.png");
    loadTex("boss", "boss_enemy.png");
    loadTex("floor", "floor.png");
    loadTex("wall", "wall.png");
    loadTex("door", "door.png");
    loadTex("gold", "gold_coins.png");
    loadTex("potion", "health_portion.png");
    loadTex("sword", "Talwar_Sword.png");
    
    // We assume a font file exists or we just rely on shapes
    font.loadFromFile("assets/arial.ttf"); // You might need to provide a font file
}

sf::Vector2f Game::cartesianToIsometric(float x, float y) const {
    float sx = (x - y) * TILE_WIDTH_HALF;
    float sy = (x + y) * TILE_HEIGHT_HALF;
    return sf::Vector2f(sx, sy);
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.1f);
        
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        if (state == GameState::Menu) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                startGame();
            }
        }
        else if (state == GameState::Dead || state == GameState::Win) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                state = GameState::Menu;
            }
        }
    }
}

void Game::startGame() {
    floor = 1;
    score = 0;
    player.reset();
    messages.clear();
    generateLevel();
    state = GameState::Playing;
}

void Game::generateLevel() {
    gameMap.generate(floor);
    Position spawn = gameMap.getSpawnPosition();
    player.x = spawn.x;
    player.y = spawn.y;
    spawnEnemies();
    spawnItems();
    moveTimer = 0;
    attackAnim = 0;
}

void Game::nextFloor() {
    if (floor >= 5) {
        winGame();
        return;
    }
    floor++;
    addMessage("Descending to Floor " + std::to_string(floor) + "...", sf::Color::Cyan);
    generateLevel();
}

void Game::winGame() {
    state = GameState::Win;
}

void Game::spawnEnemies() {
    enemies.clear();
    int count = (floor == 5) ? 1 : (3 + floor * 2);
    for (int i = 0; i < count; i++) {
        EnemyType t;
        if (floor == 5) t = EnemyType::Boss;
        else {
            int r = rand() % 3;
            if (r == 0) t = EnemyType::Soldier;
            else if (r == 1) t = EnemyType::Archer;
            else t = EnemyType::Djinn;
        }
        
        std::vector<Position> avoid;
        avoid.push_back({player.x, player.y});
        for (auto& e : enemies) avoid.push_back({e.x, e.y});
        
        Position p = gameMap.getSpawnPosition(avoid);
        if (std::abs(p.x - player.x) + std::abs(p.y - player.y) >= 4) {
            enemies.emplace_back(t, p.x, p.y, floor);
        }
    }
}

void Game::spawnItems() {
    items.clear();
    auto tiles = gameMap.getFloorTiles();
    int count = 5 + floor * 2;
    for (int i = 0; i < count; i++) {
        Position t = tiles[rand() % tiles.size()];
        int r = rand() % 100;
        ItemType type = ItemType::Key;
        if (r < 50) type = ItemType::Gold;
        else if (r < 80) type = ItemType::Potion;
        
        items.push_back({t.x, t.y, type, false, 10});
    }
}

void Game::tryMove(int dx, int dy) {
    int nx = player.x + dx;
    int ny = player.y + dy;
    if (nx < 0 || ny < 0 || nx >= COLS || ny >= ROWS) return;
    
    TileType t = gameMap.getTile(nx, ny);
    if (t == TileType::Wall) return;
    if (t == TileType::Door) {
        if (player.inventory.key > 0) {
            player.inventory.key--;
            gameMap.setTile(nx, ny, TileType::Floor);
            addMessage("Door unlocked!", sf::Color::Green);
        } else {
            addMessage("Need a key!", sf::Color::Red);
        }
        return;
    }
    
    for (auto& e : enemies) {
        if (e.alive && e.x == nx && e.y == ny) return;
    }
    
    player.x = nx;
    player.y = ny;
    if (dx > 0) player.dir = "right";
    else if (dx < 0) player.dir = "left";
    else if (dy < 0) player.dir = "up";
    else player.dir = "down";
    
    for (auto& item : items) {
        if (!item.collected && item.x == nx && item.y == ny) {
            item.collected = true;
            if (item.type == ItemType::Gold) {
                player.inventory.gold += item.value;
                score += item.value;
                addMessage("+" + std::to_string(item.value) + " Gold", sf::Color::Yellow);
            } else if (item.type == ItemType::Potion) {
                player.inventory.potion++;
                addMessage("+1 Potion", sf::Color::Green);
            } else if (item.type == ItemType::Key) {
                player.inventory.key++;
                addMessage("+1 Key", sf::Color::Magenta);
            }
        }
    }
    
    if (t == TileType::Exit) {
        nextFloor();
    }
}

void Game::playerAttack() {
    attackAnim = 0.3f;
    int tx = player.x, ty = player.y;
    if (player.dir == "up") ty--;
    else if (player.dir == "down") ty++;
    else if (player.dir == "left") tx--;
    else if (player.dir == "right") tx++;
    
    bool hit = false;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        int dist = std::abs(e.x - player.x) + std::abs(e.y - player.y);
        if ((e.x == tx && e.y == ty) || dist == 1) {
            int dmg = player.atk + (rand() % 8);
            e.hp -= dmg;
            e.flashTimer = 0.3f;
            addMessage("Hit " + e.name + " for " + std::to_string(dmg), sf::Color::Yellow);
            if (e.hp <= 0) {
                e.alive = false;
                int expG = (e.type == EnemyType::Boss) ? 200 : 15 + floor * 5;
                player.gainExp(expG);
                score += expG;
                addMessage(e.name + " defeated!", sf::Color(255, 150, 0));
                items.push_back({e.x, e.y, ItemType::Gold, false, (e.type == EnemyType::Boss) ? 100 : 15});
                if (e.type == EnemyType::Boss) winGame();
            }
            hit = true;
            break;
        }
    }
    if (!hit) addMessage("Missed!", sf::Color(150, 150, 150));
}

void Game::enemyAttack(Enemy& e) {
    int dmg = (int)(e.atk * (0.7f + (rand() % 60) / 100.0f));
    player.hp -= dmg;
    addMessage(e.name + " deals " + std::to_string(dmg) + " dmg!", sf::Color::Red);
    if (player.hp <= 0) {
        player.hp = 0;
        state = GameState::Dead;
    }
}

void Game::update(float dt) {
    if (state != GameState::Playing) return;
    
    if (attackAnim > 0) attackAnim -= dt;
    
    for (auto it = messages.begin(); it != messages.end(); ) {
        it->age += dt;
        if (it->age > 4.0f) it = messages.erase(it);
        else ++it;
    }
    
    if (moveTimer > 0) moveTimer -= dt;
    else {
        bool moved = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { tryMove(0, -1); moved = true; }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { tryMove(0, 1); moved = true; }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { tryMove(-1, 0); moved = true; }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { tryMove(1, 0); moved = true; }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) { playerAttack(); moved = true; }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            if (player.inventory.potion > 0) {
                player.inventory.potion--;
                player.heal(40);
                addMessage("Drank potion! +40 HP", sf::Color::Green);
                moved = true;
            }
        }
        if (moved) moveTimer = 0.12f;
    }
    
    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (e.flashTimer > 0) e.flashTimer -= dt;
        if (e.moveTimer > 0) {
            e.moveTimer -= dt;
            continue;
        }
        e.moveTimer = e.moveDelay;
        
        int dist = std::abs(e.x - player.x) + std::abs(e.y - player.y);
        if (dist == 1) {
            e.attackTimer -= e.moveDelay;
            if (e.attackTimer <= 0) {
                enemyAttack(e);
                e.attackTimer = 1.2f;
            }
        } else if (dist <= 10) {
            std::vector<Position> avoid;
            for (auto& oe : enemies) { if (oe.alive && &oe != &e) avoid.push_back({oe.x, oe.y}); }
            auto path = gameMap.findPath(e.x, e.y, player.x, player.y, avoid);
            if (!path.empty()) {
                e.x = path[0].x;
                e.y = path[0].y;
            }
        } else {
            int dirs[4][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};
            int r = rand() % 4;
            int nx = e.x + dirs[r][0];
            int ny = e.y + dirs[r][1];
            if (gameMap.getTile(nx, ny) != TileType::Wall) {
                bool occ = false;
                for (auto& oe : enemies) if (oe.alive && oe.x == nx && oe.y == ny) occ = true;
                if (!occ) {
                    e.x = nx;
                    e.y = ny;
                }
            }
        }
    }
    
    sf::Vector2f pIso = cartesianToIsometric(player.x, player.y);
    camX = pIso.x - 800 / 2.0f;
    camY = pIso.y - 600 / 2.0f;
}

void Game::addMessage(const std::string& text, sf::Color color) {
    messages.insert(messages.begin(), {text, color, 0.0f});
    if (messages.size() > 5) messages.pop_back();
}

void Game::drawTile(const sf::Texture& tex, int x, int y, sf::Color tint) {
    sf::Vector2f isoPos = cartesianToIsometric(x, y);
    isoPos.x -= camX;
    isoPos.y -= camY;

    sf::VertexArray quad(sf::Quads, 4);
    quad[0].position = sf::Vector2f(isoPos.x, isoPos.y - TILE_HEIGHT_HALF);
    quad[0].texCoords = sf::Vector2f(0, 0);
    quad[0].color = tint;

    quad[1].position = sf::Vector2f(isoPos.x + TILE_WIDTH_HALF, isoPos.y);
    quad[1].texCoords = sf::Vector2f(tex.getSize().x, 0);
    quad[1].color = tint;

    quad[2].position = sf::Vector2f(isoPos.x, isoPos.y + TILE_HEIGHT_HALF);
    quad[2].texCoords = sf::Vector2f(tex.getSize().x, tex.getSize().y);
    quad[2].color = tint;

    quad[3].position = sf::Vector2f(isoPos.x - TILE_WIDTH_HALF, isoPos.y);
    quad[3].texCoords = sf::Vector2f(0, tex.getSize().y);
    quad[3].color = tint;

    window.draw(quad, &tex);
}

void Game::drawSprite(const sf::Texture& tex, int x, int y, int size, bool flash) {
    sf::Vector2f isoPos = cartesianToIsometric(x, y);
    isoPos.x -= camX;
    isoPos.y -= camY;

    sf::Sprite spr(tex);
    float scale = (float)size / tex.getSize().x;
    spr.setScale(scale, scale);
    
    float drawX = isoPos.x - (size / 2.0f);
    float drawY = isoPos.y - size + (TILE_HEIGHT_HALF / 2.0f);

    spr.setPosition(drawX, drawY);
    if (flash) spr.setColor(sf::Color(255, 100, 100));
    window.draw(spr);
}

void Game::drawHealthBar(const Entity& e) {
    sf::Vector2f isoPos = cartesianToIsometric(e.x, e.y);
    float px = isoPos.x - camX - TILE_WIDTH_HALF / 2.0f;
    float py = isoPos.y - camY - TILE;

    sf::RectangleShape bg(sf::Vector2f(TILE, 4));
    bg.setPosition(px, py);
    bg.setFillColor(sf::Color(51, 0, 0));
    window.draw(bg);
    
    float pct = (float)e.hp / e.maxHp;
    sf::RectangleShape fg(sf::Vector2f(TILE * pct, 4));
    fg.setPosition(px, py);
    fg.setFillColor(pct > 0.5f ? sf::Color(68, 204, 34) : sf::Color(204, 34, 0));
    window.draw(fg);
}

void Game::drawHUD() {
}

void Game::render() {
    window.clear(sf::Color(26, 15, 8));
    
    if (state == GameState::Menu) {
        return;
    }
    
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            TileType t = gameMap.getTile(x, y);
            if (t == TileType::Wall && textures.count("wall")) drawTile(textures["wall"], x, y, sf::Color(150, 150, 150));
            else if (t == TileType::Floor && textures.count("floor")) drawTile(textures["floor"], x, y, sf::Color(200, 200, 200));
            else if (t == TileType::Door && textures.count("door")) drawTile(textures["door"], x, y, sf::Color::White);
            else if (t == TileType::Exit) {
                sf::Vector2f isoPos = cartesianToIsometric(x, y);
                sf::ConvexShape diamond;
                diamond.setPointCount(4);
                diamond.setPoint(0, sf::Vector2f(isoPos.x - camX, isoPos.y - camY - TILE_HEIGHT_HALF));
                diamond.setPoint(1, sf::Vector2f(isoPos.x - camX + TILE_WIDTH_HALF, isoPos.y - camY));
                diamond.setPoint(2, sf::Vector2f(isoPos.x - camX, isoPos.y - camY + TILE_HEIGHT_HALF));
                diamond.setPoint(3, sf::Vector2f(isoPos.x - camX - TILE_WIDTH_HALF, isoPos.y - camY));
                diamond.setFillColor(sf::Color::Green);
                window.draw(diamond);
            }
            
            for (auto& it : items) {
                if (!it.collected && it.x == x && it.y == y) {
                    std::string texName = it.type == ItemType::Gold ? "gold" : (it.type == ItemType::Potion ? "potion" : "");
                    if (texName != "" && textures.count(texName)) drawSprite(textures[texName], it.x, it.y, 20);
                }
            }
            
            for (auto& e : enemies) {
                if (e.alive && e.x == x && e.y == y) {
                    std::string tname = (e.type == EnemyType::Soldier) ? "enemy" : (e.type == EnemyType::Archer ? "archer" : (e.type == EnemyType::Boss ? "boss" : "djinn"));
                    if (textures.count(tname)) drawSprite(textures[tname], e.x, e.y, (e.type == EnemyType::Boss) ? 48 : 30, e.flashTimer > 0);
                    drawHealthBar(e);
                }
            }
            
            if (player.x == x && player.y == y) {
                if (textures.count("character")) drawSprite(textures["character"], player.x, player.y, TILE, false);
                
                if (attackAnim > 0) {
                    int d[2] = {0, 1};
                    if (player.dir == "up") { d[0] = 0; d[1] = -1; }
                    else if (player.dir == "left") { d[0] = -1; d[1] = 0; }
                    else if (player.dir == "right") { d[0] = 1; d[1] = 0; }
                    
                    if (textures.count("sword")) {
                        sf::Vector2f isoPos = cartesianToIsometric(player.x + d[0], player.y + d[1]);
                        sf::Sprite spr(textures["sword"]);
                        float scale = 24.0f / textures["sword"].getSize().x;
                        spr.setScale(scale, scale);
                        spr.setPosition(isoPos.x - camX - 12.0f, isoPos.y - camY - 24.0f);
                        window.draw(spr);
                    }
                }
            }
        }
    }
    
    drawHUD();
    window.display();
}

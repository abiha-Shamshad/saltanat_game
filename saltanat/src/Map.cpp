#include "Map.h"
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <set>
#include <map>

Map::Map() {
    grid.resize(ROWS, std::vector<TileType>(COLS, TileType::Wall));
}

void Map::generate(int floorNum) {
    for(int y=0; y<ROWS; y++) {
        for(int x=0; x<COLS; x++) {
            grid[y][x] = TileType::Wall;
        }
    }

    int cx = COLS / 2;
    int cy = ROWS / 2;
    int steps = 200 + floorNum * 20;
    
    grid[cy][cx] = TileType::Floor;
    int dirs[4][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};
    
    for (int i = 0; i < steps; i++) {
        int r = rand() % 4;
        cx += dirs[r][0];
        cy += dirs[r][1];
        
        cx = std::max(1, std::min(COLS - 2, cx));
        cy = std::max(1, std::min(ROWS - 2, cy));
        grid[cy][cx] = TileType::Floor;
        
        if (rand() % 100 < 20) {
            for (int ry = 0; ry <= 1; ry++) {
                for (int rx = 0; rx <= 1; rx++) {
                    if (cy + ry < ROWS - 1 && cx + rx < COLS - 1) {
                        grid[cy + ry][cx + rx] = TileType::Floor;
                    }
                }
            }
        }
    }

    auto floorTiles = getFloorTiles();
    if (floorTiles.size() > 10) {
        Position doorTile = floorTiles[floorTiles.size() / 10];
        grid[doorTile.y][doorTile.x] = TileType::Door;
        Position exitTile = floorTiles.back();
        grid[exitTile.y][exitTile.x] = TileType::Exit;
    }
}

TileType Map::getTile(int x, int y) const {
    if (x < 0 || y < 0 || x >= COLS || y >= ROWS) return TileType::Wall;
    return grid[y][x];
}

void Map::setTile(int x, int y, TileType type) {
    if (x >= 0 && y >= 0 && x < COLS && y < ROWS) {
        grid[y][x] = type;
    }
}

std::vector<Position> Map::getFloorTiles() const {
    std::vector<Position> tiles;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (grid[y][x] == TileType::Floor) {
                tiles.push_back({x, y});
            }
        }
    }
    return tiles;
}

Position Map::getSpawnPosition(const std::vector<Position>& avoid) const {
    auto tiles = getFloorTiles();
    std::vector<Position> validTiles;
    for (const auto& t : tiles) {
        bool avoidIt = false;
        for (const auto& a : avoid) {
            if (a.x == t.x && a.y == t.y) {
                avoidIt = true;
                break;
            }
        }
        if (!avoidIt) {
            validTiles.push_back(t);
        }
    }
    if (!validTiles.empty()) {
        return validTiles[rand() % validTiles.size()];
    }
    return {2, 2};
}

int Map::heuristic(int ax, int ay, int bx, int by) const {
    return std::abs(ax - bx) + std::abs(ay - by);
}

// A* Implementation
struct Node {
    int x, y;
    int g, h;
    std::shared_ptr<Node> parent;
    int f() const { return g + h; }
};

struct CompareNode {
    bool operator()(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) const {
        return a->f() > b->f();
    }
};

std::vector<Position> Map::findPath(int sx, int sy, int gx, int gy, const std::vector<Position>& blockedPositions) const {
    auto key = [](int x, int y) { return y * COLS + x; };
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CompareNode> open;
    std::set<int> closed;
    std::map<int, int> bestG;

    open.push(std::make_shared<Node>(Node{sx, sy, 0, heuristic(sx, sy, gx, gy), nullptr}));
    bestG[key(sx, sy)] = 0;

    int dirs[4][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};

    int iters = 0;
    while (!open.empty() && iters < 500) {
        iters++;
        auto cur = open.top();
        open.pop();

        if (cur->x == gx && cur->y == gy) {
            std::vector<Position> path;
            auto n = cur;
            while (n->parent) {
                path.insert(path.begin(), {n->x, n->y});
                n = n->parent;
            }
            return path;
        }

        int k = key(cur->x, cur->y);
        if (closed.count(k)) continue;
        closed.insert(k);

        for (int i = 0; i < 4; i++) {
            int nx = cur->x + dirs[i][0];
            int ny = cur->y + dirs[i][1];

            if (nx < 0 || ny < 0 || nx >= COLS || ny >= ROWS) continue;
            if (grid[ny][nx] == TileType::Wall) continue;
            
            bool blocked = false;
            for (const auto& bp : blockedPositions) {
                if (bp.x == nx && bp.y == ny && (nx != gx || ny != gy)) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) continue;

            int ng = cur->g + 1;
            int nk = key(nx, ny);

            if (closed.count(nk)) continue;
            if (bestG.count(nk) && ng >= bestG[nk]) continue;

            bestG[nk] = ng;
            open.push(std::make_shared<Node>(Node{nx, ny, ng, heuristic(nx, ny, gx, gy), cur}));
        }
    }
    return {};
}

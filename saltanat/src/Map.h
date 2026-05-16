#pragma once
#include <vector>
#include "Constants.h"

class Map {
public:
    Map();
    void generate(int floorNum);
    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType type);
    Position getSpawnPosition(const std::vector<Position>& avoid = {}) const;
    std::vector<Position> getFloorTiles() const;
    
    // A* Pathfinding
    std::vector<Position> findPath(int sx, int sy, int gx, int gy, const std::vector<Position>& blockedPositions) const;

private:
    std::vector<std::vector<TileType>> grid;
    int heuristic(int ax, int ay, int bx, int by) const;
};

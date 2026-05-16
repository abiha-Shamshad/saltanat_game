#pragma once

#include <string>

const int TILE = 32;
const int TILE_WIDTH_HALF = 32;
const int TILE_HEIGHT_HALF = 16;
const int COLS = 25;
const int ROWS = 18;
const int W = COLS * TILE;
const int H = ROWS * TILE;

enum class TileType {
    Floor = 0,
    Wall = 1,
    Door = 2,
    Exit = 3
};

enum class ItemType {
    Gold,
    Potion,
    Key
};

enum class GameState {
    Menu,
    Playing,
    Dead,
    Win
};

struct Position {
    int x, y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Item {
    int x, y;
    ItemType type;
    bool collected;
    int value; // For gold
};

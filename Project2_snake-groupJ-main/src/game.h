#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Screen and Grid setup
#define GRID_SIZE 30
#define GRID_COUNT_X 30
#define GRID_COUNT_Y 30
#define WINDOW_WIDTH (GRID_SIZE * GRID_COUNT_X)
#define WINDOW_HEIGHT (GRID_SIZE * GRID_COUNT_Y)

// Speed constraints
#define MIN_INTERVAL_MS 40
#define MAX_INTERVAL_MS 300
#define DEFAULT_INTERVAL_MS 100

// Game modes
typedef enum {
    MODE_CLASSIC = 0,
    MODE_STAGES,
    MODE_CHAOS
} GameMode;

// Game states
typedef enum {
    STATE_START = 0,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_GAME_WIN
} GameState;

// Tile types for maps
typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL,
    TILE_PORTAL
} TileType;

// Textures loaded from assets
typedef struct {
    Texture2D headTex;
    Texture2D bodyTex;
    Texture2D tailTex;
    Texture2D bgTex;
    Texture2D fruitTex;
    Texture2D wallTex;
} GameAssets;

// Shared Game Context
typedef struct {
    GameState state;
    int score;
    int highScore;
    GameAssets assets;
    bool reverseMode;
    GameMode mode;
    TileType map[GRID_COUNT_X][GRID_COUNT_Y];
    int currentLevel;
    Vector2 portals[2];
    bool devModeActive;
    int devLevelSelect;
    int moveIntervalMs; // Milliseconds per tile movement
} GameData;

// Forward declarations
typedef struct Snake Snake; 
typedef struct Food Food;

void InitGameData(GameData* game);
void UpdateGame(GameData* game, Snake* snake, Food* food);
void DrawGame(GameData* game, Snake* snake, Food* food);
void RestartGame(GameData* game, Snake* snake, Food* food);
void LoadLevel(GameData* game, int level, Snake* snake);
void InitChaosMode(GameData* game, Snake* snake);
int LoadHighScore(void);
void SaveHighScore(int score);

#endif // GAME_H

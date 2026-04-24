#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Screen and Grid setup
#define GRID_SIZE 30
#define GRID_COUNT_X 30
#define GRID_COUNT_Y 30
#define WINDOW_WIDTH (GRID_SIZE * GRID_COUNT_X)
#define TOOLBAR_HEIGHT 60
#define WINDOW_HEIGHT ((GRID_SIZE * GRID_COUNT_Y) + TOOLBAR_HEIGHT)

// Game Modes
typedef enum { MODE_CLASSIC = 0, MODE_STAGES, MODE_CHAOS } GameMode;

// Tile types for the game map (matched with stages.h)
typedef enum { TILE_EMPTY = 0, TILE_WALL } TileType;

// Game states
typedef enum {
  STATE_START = 0,
  STATE_PLAYING,
  STATE_GAME_OVER,
  STATE_GAME_WIN,
  STATE_SCOREBOARD,
  STATE_NAME_INPUT
} GameState;

typedef struct {
  char name[16];
  int score;
} ScoreEntry;

// Textures loaded from assets
typedef struct {
  Texture2D headTex;
  Texture2D bodyTex;
  Texture2D tailTex;
  Texture2D bgTex;
  Texture2D fruitTex;
  Texture2D wallTex;
  Font mainFont;
} GameAssets;

// Shared Game Context
typedef struct GameData {
  GameState state;
  GameMode mode;
  int currentLevel;
  int score;
  ScoreEntry highScores[10];
  char currentName[16];
  int nameLength;
  TileType map[GRID_COUNT_X][GRID_COUNT_Y];
  bool reverseMode;
  bool isPaused;
  int speedFPS;
  bool devModeActive;
  int devLevelSelect;
  GameAssets assets;
} GameData;

// Forward declarations
typedef struct Snake Snake;
typedef struct Food Food;

void InitGameData(GameData *game);
void UpdateGame(GameData *game, Snake *snake, Food *food);
void DrawGame(GameData *game, Snake *snake, Food *food);
void RestartGame(GameData *game, Snake *snake, Food *food);
void LoadScoreboard(GameData *game);
void SaveScoreboard(GameData *game);

#endif // GAME_H

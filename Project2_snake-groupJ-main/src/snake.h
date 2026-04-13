#ifndef SNAKE_H
#define SNAKE_H

#include "raylib.h"
#include <stdbool.h>
#include "game.h"

#define MAX_SNAKE_LENGTH (GRID_COUNT_X * GRID_COUNT_Y)

typedef struct Snake {
    Vector2 body[MAX_SNAKE_LENGTH];
    int length;
    Vector2 direction;
    
    // Input queue for smooth turning
    Vector2 inputQueue[2];
    int inputQueueLength;
    
    // Continuous Movement
    float speed; // Tiles per second
    Vector2 nextDirection; // Buffer for the next grid-aligned move
    
    // Trail for smooth following: stores points the head has visited
    // Size should handle the max length at the slowest speed
    #define TRAIL_SIZE (MAX_SNAKE_LENGTH * 40) 
    Vector2 trail[TRAIL_SIZE];
    int trailHead;   // Index of the newest point
    int trailCount;  // Total points in buffer
    float trailTimer; // Accumulator for trail sampling
    
    bool isAlive;
    bool justAteFood;
    float invulnerabilityTimer; // Seconds left of Ghost Mode
} Snake;

void InitSnake(Snake* snake);
void HandleSnakeInput(Snake* snake);
void UpdateSnakeLogic(Snake* snake, GameData* game);
void DrawSnake(Snake* snake, Texture2D headTex, Texture2D bodyTex, Texture2D tailTex);
bool CheckCollisionWithSelfOrWall(Snake* snake, GameData* game);
void ReverseSnake(Snake* snake);

#endif // SNAKE_H

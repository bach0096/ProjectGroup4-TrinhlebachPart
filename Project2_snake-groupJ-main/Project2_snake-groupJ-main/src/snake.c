#include "snake.h"
#include "game.h"
#include <math.h>

void InitSnake(Snake* snake) {
    snake->length = 3;
    // Start somewhere in the middle
    int startX = GRID_COUNT_X / 2;
    int startY = GRID_COUNT_Y / 2;
    
    // Initialize body horizontally
    for (int i = 0; i < snake->length; i++) {
        snake->body[i] = (Vector2){(float)startX, (float)(startY + i)};
    }
    
    snake->direction = (Vector2){0, -1}; // Moving UP initially
    snake->moveTimer = 0.0f;
    snake->timePerMove = 0.12f; // Smooth comfortable speed
    snake->inputQueueLength = 0;
    snake->isAlive = true;
    snake->justAteFood = false;
    snake->invulnerabilityTimer = 0.0f;
}

void HandleSnakeInput(Snake* snake) {
    Vector2 nextDir = {0};
    bool keyHit = false;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) { nextDir = (Vector2){0, -1}; keyHit = true; }
    else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { nextDir = (Vector2){0, 1}; keyHit = true; }
    else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) { nextDir = (Vector2){-1, 0}; keyHit = true; }
    else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { nextDir = (Vector2){1, 0}; keyHit = true; }

    if (keyHit) {
        // Find current effective direction (last queued or current)
        Vector2 currentDir = snake->direction;
        if (snake->inputQueueLength > 0) {
            currentDir = snake->inputQueue[snake->inputQueueLength - 1];
        }

        // Prevent reversing direction
        if ((nextDir.x != 0 && nextDir.x != -currentDir.x) ||
            (nextDir.y != 0 && nextDir.y != -currentDir.y)) {
            
            // Add to queue
            if (snake->inputQueueLength < 2) {
                snake->inputQueue[snake->inputQueueLength] = nextDir;
                snake->inputQueueLength++;
            }
        }
    }
}

void UpdateSnakeLogic(Snake* snake, GameData* game) {
    if (!snake->isAlive) return;
    
    if (snake->invulnerabilityTimer > 0) {
        snake->invulnerabilityTimer -= GetFrameTime();
        if (snake->invulnerabilityTimer < 0) snake->invulnerabilityTimer = 0;
    }

    // Pop input from queue
    if (snake->inputQueueLength > 0) {
        snake->direction = snake->inputQueue[0];
        // Shift queue
        snake->inputQueue[0] = snake->inputQueue[1];
        snake->inputQueueLength--;
    }

    // Handle growth
    if (snake->justAteFood) {
        if (snake->length < MAX_SNAKE_LENGTH) {
            snake->length++;
        }
        snake->justAteFood = false;
    }

    // Shift segments back (from tail to neck)
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    
    // Move head
    snake->body[0].x += snake->direction.x;
    snake->body[0].y += snake->direction.y;
    
    // Portal Check
    // We check bounds first to avoid segfaulting on map access
    if (snake->body[0].x >= 0 && snake->body[0].x < GRID_COUNT_X && 
        snake->body[0].y >= 0 && snake->body[0].y < GRID_COUNT_Y) {
        
        if (game->map[(int)snake->body[0].x][(int)snake->body[0].y] == TILE_PORTAL) {
            // Find which portal we hit and teleport to the other
            if (snake->body[0].x == game->portals[0].x && snake->body[0].y == game->portals[0].y) {
                snake->body[0] = game->portals[1];
            } else {
                snake->body[0] = game->portals[0];
            }
        }
    }
}

bool CheckCollisionWithSelfOrWall(Snake* snake, GameData* game) {
    Vector2 head = snake->body[0];
    
    // Bounds check
    if (head.x < 0 || head.x >= GRID_COUNT_X || head.y < 0 || head.y >= GRID_COUNT_Y) {
        snake->isAlive = false;
        return true;
    }
    
    // Map check (Obstacles)
    if (game->map[(int)head.x][(int)head.y] == TILE_WALL) {
        // Skip wall collision if in Ghost Mode
        if (snake->invulnerabilityTimer <= 0) {
            snake->isAlive = false;
            return true;
        }
    }
    
    // Self check
    for (int i = 1; i < snake->length; i++) {
        if (head.x == snake->body[i].x && head.y == snake->body[i].y) {
            snake->isAlive = false;
            return true;
        }
    }
    
    return false;
}

// Helper to draw rotated texture
void DrawTextureCenteredEx(Texture2D tex, Vector2 pos, float rotation, float scale, Color tint) {
    if (tex.id == 0) return;
    Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
    Rectangle dest = {pos.x, pos.y, tex.width * scale, tex.height * scale};
    Vector2 origin = {(tex.width * scale) / 2.0f, (tex.height * scale) / 2.0f};
    
    DrawTexturePro(tex, source, dest, origin, rotation, tint);
}

void DrawSnake(Snake* snake, Texture2D headTex, Texture2D bodyTex, Texture2D tailTex) {
    // Distance between nodes is GRID_SIZE. 
    // To achieve half overlap, the visual diameter needs to be exactly 2 * GRID_SIZE.
    float overlapDiameter = GRID_SIZE * 2.0f;
    
    // Ghost mode tint (pulsing transparency)
    Color snakeTint = WHITE;
    if (snake->invulnerabilityTimer > 0) {
        float alpha = (sinf((float)GetTime() * 15.0f) * 0.5f + 0.5f) * 150 + 50; 
        snakeTint = (Color){255, 255, 255, (unsigned char)alpha};
    }

    // HIGHEST Z-INDEX is HEAD. LOWEST Z-INDEX is TAIL.
    // So we loop backwards: Tail -> Body -> Head (so head is rendered last and stays on top)
    for (int i = snake->length - 1; i >= 0; i--) {
        Vector2 gridPos = snake->body[i];
        Vector2 center = { 
            gridPos.x * GRID_SIZE + (GRID_SIZE / 2.0f), 
            gridPos.y * GRID_SIZE + (GRID_SIZE / 2.0f) 
        };

        if (i == 0) { // HEAD
            if (headTex.id != 0) {
                float scale = overlapDiameter / (float)headTex.width;
                float rot = 0.0f;
                // Raylib rotation is in degrees. Adjust if your image points a different default way.
                // Assuming default head sprite points UP
                if (snake->direction.x == 1) rot = 90.0f;
                else if (snake->direction.x == -1) rot = 270.0f;
                else if (snake->direction.y == 1) rot = 180.0f;
                else if (snake->direction.y == -1) rot = 0.0f;
                
                rot += 90.0f; // User requested: lật sang phải 1 lần
                
                DrawTextureCenteredEx(headTex, center, rot, scale, WHITE);
            } else {
                DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE, GRID_SIZE, DARKGREEN);
            }
        } 
        else if (i == snake->length - 1) { // TAIL
            if (tailTex.id != 0) {
                float scale = overlapDiameter / (float)tailTex.width;
                // Rotation based on the segment it is attached to (i-1)
                Vector2 diff = { snake->body[i-1].x - snake->body[i].x, snake->body[i-1].y - snake->body[i].y };
                float rot = 0.0f;
                // Assuming tail points UP towards the body
                if (diff.x == 1) rot = 90.0f;
                else if (diff.x == -1) rot = 270.0f;
                else if (diff.y == 1) rot = 180.0f;
                else if (diff.y == -1) rot = 0.0f;
                
                rot += 90.0f; // User requested: lật sang phải 1 lần
                
                DrawTextureCenteredEx(tailTex, center, rot, scale, WHITE);
            } else {
                DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GREEN);
            }
        } 
        else { // BODY
            if (bodyTex.id != 0) {
                float scale = overlapDiameter / (float)bodyTex.width;
                // Body circles don't strictly need rotation
                DrawTextureCenteredEx(bodyTex, center, 0.0f, scale, snakeTint);
            } else {
                Color tint = (i % 2 == 0) ? LIME : GREEN;
                // Fallback circular overlap
                DrawCircle(center.x, center.y, overlapDiameter / 2.0f, tint);
            }
        }
    }
}

void ReverseSnake(Snake* snake) {
    // Reverse the body array up to length
    for (int i = 0; i < snake->length / 2; i++) {
        Vector2 temp = snake->body[i];
        snake->body[i] = snake->body[snake->length - 1 - i];
        snake->body[snake->length - 1 - i] = temp;
    }
    
    // Update direction based on new head and new second segment
    if (snake->length > 1) {
        snake->direction.x = snake->body[0].x - snake->body[1].x;
        snake->direction.y = snake->body[0].y - snake->body[1].y;
    }
    
    // Clear input queue as old directions are no longer valid
    snake->inputQueueLength = 0;
}

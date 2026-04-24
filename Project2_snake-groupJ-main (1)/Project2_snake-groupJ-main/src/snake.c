#include "snake.h"
#include "game.h"
#include <math.h>

void InitSnake(Snake *snake) {
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
  snake->timePerMove = 0.1f; // Slower speed (reduced by 50%)
  snake->inputQueueLength = 0;
  snake->isAlive = true;
  snake->justAteFood = false;
  snake->invincibilityTimer = 0.0f;
}

void HandleSnakeInput(Snake *snake) {
  Vector2 nextDir = {0};
  bool keyHit = false;

  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    nextDir = (Vector2){0, -1};
    keyHit = true;
  } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    nextDir = (Vector2){0, 1};
    keyHit = true;
  } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
    nextDir = (Vector2){-1, 0};
    keyHit = true;
  } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
    nextDir = (Vector2){1, 0};
    keyHit = true;
  }

  if (keyHit) {
    // Find current effective direction (last queued or current)
    Vector2 currentDir = snake->direction;
    if (snake->inputQueueLength > 0) {
      currentDir = snake->inputQueue[snake->inputQueueLength - 1];
    }

    // 1. Prevent reversing direction (U-turn suicide)
    // 2. Prevent redundant directions (Ignoring the same direction reduces
    // input lag)
    if ((nextDir.x != 0 && nextDir.x != currentDir.x &&
         nextDir.x != -currentDir.x) ||
        (nextDir.y != 0 && nextDir.y != currentDir.y &&
         nextDir.y != -currentDir.y)) {

      // Add to queue
      if (snake->inputQueueLength < 2) {
        snake->inputQueue[snake->inputQueueLength] = nextDir;
        snake->inputQueueLength++;
      }
    }
  }
}

void UpdateSnakeLogic(Snake *snake, GameData *game) {
  if (!snake->isAlive)
    return;

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

  // Grid Wrapping (Xuyên biên)
  if (snake->body[0].x < 0)
    snake->body[0].x = GRID_COUNT_X - 1;
  else if (snake->body[0].x >= GRID_COUNT_X)
    snake->body[0].x = 0;

  if (snake->body[0].y < 0)
    snake->body[0].y = GRID_COUNT_Y - 1;
  else if (snake->body[0].y >= GRID_COUNT_Y)
    snake->body[0].y = 0;

  // Portal logic removed to fix 'invisible redirection' bug
}

bool CheckCollisionWithSelfOrWall(Snake *snake, GameData *game) {
  Vector2 head = snake->body[0];

  // Grid bounds check (Đã tắt để hỗ trợ Xuyên Biên)
  /*
  if (head.x < 0 || head.x >= GRID_COUNT_X || head.y < 0 ||
      head.y >= GRID_COUNT_Y) {
    snake->isAlive = false;
    return true;
  }
  */

  // Map wall check (Skip if invincible)
  if (game->map[(int)head.x][(int)head.y] == TILE_WALL) {
    if (snake->invincibilityTimer <= 0) {
      snake->isAlive = false;
      return true;
    }
  }

  // Self check (Skip if invincible)
  if (snake->invincibilityTimer <= 0) {
    for (int i = 1; i < snake->length; i++) {
      if (head.x == snake->body[i].x && head.y == snake->body[i].y) {
        snake->isAlive = false;
        return true;
      }
    }
  }

  return false;
}

void ReverseSnake(Snake *snake) {
  for (int i = 0; i < snake->length / 2; i++) {
    Vector2 temp = snake->body[i];
    snake->body[i] = snake->body[snake->length - 1 - i];
    snake->body[snake->length - 1 - i] = temp;
  }
  if (snake->length > 1) {
    snake->direction.x = snake->body[0].x - snake->body[1].x;
    snake->direction.y = snake->body[0].y - snake->body[1].y;
  }
  snake->inputQueueLength = 0;
}

// Helper to draw rotated texture with optional flipping
void DrawTextureCenteredEx(Texture2D tex, Vector2 pos, float rotation,
                           float scale, bool flipX, Color tint) {
  if (tex.id == 0)
    return;
  float targetWidth = tex.width * scale;
  float targetHeight = tex.height * scale;

  Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
  if (flipX)
    source.width = -source.width;

  Rectangle dest = {pos.x, pos.y, targetWidth, targetHeight};
  Vector2 origin = {targetWidth / 2.0f, targetHeight / 2.0f};

  DrawTexturePro(tex, source, dest, origin, rotation, tint);
}

void DrawSnake(Snake *snake, Texture2D headTex, Texture2D bodyTex,
               Texture2D tailTex) {
  float targetSize = (float)GRID_SIZE * 1.20f; // Increased by 15% as requested

  // Pulse transparency color if invincible
  Color snakeAlphaTint = WHITE;
  if (snake->invincibilityTimer > 0) {
    float pulse = (sinf((float)GetTime() * 20.0f) + 1.0f) * 0.4f + 0.2f;
    snakeAlphaTint = (Color){255, 255, 255, (unsigned char)(255 * pulse)};
  }

  for (int i = snake->length - 1; i >= 0; i--) {
    Vector2 gridPos = snake->body[i];
    Vector2 center = {gridPos.x * GRID_SIZE + (GRID_SIZE / 2.0f),
                      gridPos.y * GRID_SIZE + (GRID_SIZE / 2.0f)};

    // Selection logic based on segments

    // Overlay layer removed as requested

    if (i == 0) { // HEAD
      if (headTex.id != 0) {
        float rot = 0.0f;
        bool flipH = false;

        // Texture head.png faces LEFT by default.
        if (snake->direction.x == 1) {
          rot = 0.0f;
          flipH = true;
        } // Moving RIGHT: Flip horizontally
        else if (snake->direction.x == -1) {
          rot = 0.0f;
          flipH = false;
        } // Moving LEFT: Default orientation
        else if (snake->direction.y == 1) {
          rot = 270.0f;
          flipH = false;
        } // Moving DOWN
        else if (snake->direction.y == -1) {
          rot = 90.0f;
          flipH = false;
        } // Moving UP

        DrawTextureCenteredEx(headTex, center, rot,
                              targetSize / (float)headTex.width, flipH,
                              snakeAlphaTint);
      } else {
        DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE,
                      GRID_SIZE, YELLOW);
      }
    } else if (i == snake->length - 1) { // TAIL
      if (tailTex.id != 0) {
        Vector2 diff = {snake->body[i - 1].x - snake->body[i].x,
                        snake->body[i - 1].y - snake->body[i].y};
        float rot = 0.0f;
        bool flipH = false;

        if (diff.x == 1) {
          rot = 0.0f;
          flipH = false;
        } else if (diff.x == -1) {
          rot = 0.0f;
          flipH = true;
        } else if (diff.y == 1) {
          rot = 90.0f;
          flipH = false;
        } else if (diff.y == -1) {
          rot = 270.0f;
          flipH = false;
        }

        DrawTextureCenteredEx(tailTex, center, rot,
                              targetSize / (float)tailTex.width, flipH,
                              snakeAlphaTint);
      } else {
        DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE,
                      GRID_SIZE, YELLOW);
      }
    } else { // BODY
      if (bodyTex.id != 0) {
        Vector2 diff = {snake->body[i - 1].x - snake->body[i].x,
                        snake->body[i - 1].y - snake->body[i].y};
        float rot = 0.0f;
        bool flipH = false;

        if (diff.x == 1) {
          rot = 0.0f;
          flipH = false;
        } else if (diff.x == -1) {
          rot = 0.0f;
          flipH = true;
        } else if (diff.y == 1) {
          rot = 90.0f;
          flipH = false;
        } else if (diff.y == -1) {
          rot = 270.0f;
          flipH = false;
        }

        DrawTextureCenteredEx(bodyTex, center, rot,
                              targetSize / (float)bodyTex.width, flipH,
                              snakeAlphaTint);
      } else {
        DrawCircle(center.x, center.y, targetSize / 2.0f, YELLOW);
      }
    }
  }
}

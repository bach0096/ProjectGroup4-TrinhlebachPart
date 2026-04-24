#include "display.h"
#include "game.h"
#include "snake.h"
#include "food.h"
#include "raylib.h"
#include <stdio.h>

void DrawGame(GameData *game, Snake *snake, Food *food) {
  BeginDrawing();
  Color arenaColor = (Color){160, 220, 160, 255}; // Màu xanh lá nhẹ
  ClearBackground(arenaColor);

  Camera2D cam = {0};
  cam.offset = (Vector2){0, TOOLBAR_HEIGHT};
  cam.zoom = 1.0f;

  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    BeginMode2D(cam);
  }

  // Draw Background if loaded
  if (game->assets.bgTex.id != 0) {
    DrawTexturePro(game->assets.bgTex,
                   (Rectangle){0, 0, (float)game->assets.bgTex.width,
                               (float)game->assets.bgTex.height},
                   (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                   (Vector2){0, 0}, 0.0f, WHITE);
  } else {
    ClearBackground(arenaColor);
  }

  // Draw Grid exactly inside the 30x30 arena
  Color gridColor = (Color){
      255, 255, 255, 100}; // White at 100 alpha (approx 50% opacity of 255)
  for (int i = 0; i <= GRID_COUNT_X; i++) {
    DrawLine(i * GRID_SIZE, 0, i * GRID_SIZE, GRID_COUNT_Y * GRID_SIZE,
             gridColor);
  }
  for (int i = 0; i <= GRID_COUNT_Y; i++) {
    DrawLine(0, i * GRID_SIZE, GRID_COUNT_X * GRID_SIZE, i * GRID_SIZE,
             gridColor);
  }

  // Draw entities
  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    // Draw Walls and Portals
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        if (game->map[x][y] == TILE_WALL) {
          if (game->assets.wallTex.id != 0) {
            DrawTexturePro(game->assets.wallTex,
                           (Rectangle){0, 0, (float)game->assets.wallTex.width,
                                       (float)game->assets.wallTex.height},
                           (Rectangle){(float)x * GRID_SIZE,
                                       (float)y * GRID_SIZE, (float)GRID_SIZE,
                                       (float)GRID_SIZE},
                           (Vector2){0, 0}, 0.0f, WHITE);
          } else {
            DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE,
                          DARKGRAY);
            DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE,
                               GRID_SIZE, GRAY);
          }
        }
      }
    }

    DrawFood(food, game->assets.fruitTex);
    DrawSnake(snake, game->assets.headTex, game->assets.bodyTex,
              game->assets.tailTex);
  }

  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    EndMode2D();

    // Draw Toolbar Space
    DrawRectangle(0, 0, WINDOW_WIDTH, TOOLBAR_HEIGHT, (Color){30, 30, 30, 255});
    DrawLine(0, TOOLBAR_HEIGHT, WINDOW_WIDTH, TOOLBAR_HEIGHT, WHITE);
  }

  // Draw UI
  if (game->state == STATE_SCOREBOARD) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "TOP 10 SCOREBOARD", 40, 2).x /
                2.0f,
        50};
    DrawTextEx(game->assets.mainFont, "TOP 10 SCOREBOARD", titlePos, 40, 2,
               (Color){230, 250, 240, 255});

    for (int i = 0; i < 10; i++) {
      char entryText[64];
      snprintf(entryText, 64, "%2d. %-15s %5d", i + 1, game->highScores[i].name,
               game->highScores[i].score);
      Vector2 entryPos = {
          WINDOW_WIDTH / 2.0f -
              MeasureTextEx(game->assets.mainFont, entryText, 25, 1).x / 2.0f,
          120.0f + i * 35.0f};
      DrawTextEx(game->assets.mainFont, entryText, entryPos, 25, 1,
                 (Color){230, 250, 240, 255});
    }

    Vector2 backTextSize =
        MeasureTextEx(game->assets.mainFont, "RETURN", 20, 1);
    Vector2 subPos = {WINDOW_WIDTH / 2.0f - backTextSize.x / 2.0f,
                      WINDOW_HEIGHT - 60.0f};

    Rectangle backRect = {subPos.x - 20, subPos.y - 10, backTextSize.x + 40,
                          backTextSize.y + 20};
    Vector2 mousePos = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePos, backRect);

    DrawRectangleRounded(backRect, 0.3f, 10,
                         isHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (isHovering)
      DrawRectangleRoundedLines(backRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});

    DrawTextEx(game->assets.mainFont, "RETURN", subPos, 20, 1,
               isHovering ? (Color){230, 250, 240, 255}
                          : (Color){200, 220, 210, 255});

  } else if (game->state == STATE_START) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "SNAKE GAME", 75, 2).x / 2.0f,
        WINDOW_HEIGHT / 3.0f - 35}; // Adjusted position for size 75
    DrawTextEx(game->assets.mainFont, "SNAKE GAME", titlePos, 75, 2,
               (Color){135, 206, 250, 255});

    Vector2 mousePos = GetMousePosition();

    // Start Button
    Vector2 startTextSize =
        MeasureTextEx(game->assets.mainFont, "START GAME", 20, 1);
    Vector2 startPos = {WINDOW_WIDTH / 2.0f - startTextSize.x / 2.0f,
                        WINDOW_HEIGHT / 2.0f};
    Rectangle startRect = {startPos.x - 20, startPos.y - 10,
                           startTextSize.x + 40, startTextSize.y + 20};
    bool startHovering = CheckCollisionPointRec(mousePos, startRect);
    DrawRectangleRounded(startRect, 0.3f, 10,
                         startHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (startHovering)
      DrawRectangleRoundedLines(startRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, "START GAME", startPos, 20, 1,
               startHovering ? BLACK : (Color){230, 250, 240, 255});

    // Draw Mode Info
    const char *modeTexts[] = {"MODE: CLASSIC", "MODE: STAGES", "MODE: CHAOS"};
    Color modeColors[] = {(Color){200, 220, 210, 255}, LIME, MAGENTA};
    Vector2 modeTextSize =
        MeasureTextEx(game->assets.mainFont, modeTexts[game->mode], 18, 1);
    Vector2 modePos = {WINDOW_WIDTH / 2.0f - modeTextSize.x / 2.0f,
                       WINDOW_HEIGHT / 2.0f + 40};
    Rectangle modeRect = {modePos.x - 20, modePos.y - 10, modeTextSize.x + 40,
                          modeTextSize.y + 20};
    bool modeHovering = CheckCollisionPointRec(mousePos, modeRect);
    DrawRectangleRounded(modeRect, 0.3f, 10,
                         modeHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (modeHovering)
      DrawRectangleRoundedLines(modeRect, 0.3f, 10, 2, modeColors[game->mode]);
    DrawTextEx(game->assets.mainFont, modeTexts[game->mode], modePos, 18, 1,
               modeHovering ? BLACK : modeColors[game->mode]);

    // Draw Reverse string
    const char *revText =
        game->reverseMode ? "REVERSE MODE: ON" : "REVERSE MODE: OFF";
    Vector2 revTextSize = MeasureTextEx(game->assets.mainFont, revText, 18, 1);
    Vector2 revPos = {WINDOW_WIDTH / 2.0f - revTextSize.x / 2.0f,
                      WINDOW_HEIGHT / 2.0f + 70};
    Rectangle revRect = {revPos.x - 20, revPos.y - 10, revTextSize.x + 40,
                         revTextSize.y + 20};
    bool revHovering = CheckCollisionPointRec(mousePos, revRect);
    DrawRectangleRounded(revRect, 0.3f, 10,
                         revHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (revHovering)
      DrawRectangleRoundedLines(
          revRect, 0.3f, 10, 2,
          game->reverseMode ? GOLD : (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, revText, revPos, 18, 1,
               revHovering
                   ? BLACK
                   : (game->reverseMode ? GOLD : (Color){230, 250, 240, 255}));

    // Speed Button
    const char *speedText = TextFormat("SPEED: %d FPS", game->speedFPS);
    Vector2 speedTextSize =
        MeasureTextEx(game->assets.mainFont, speedText, 18, 1);
    Vector2 speedPos = {WINDOW_WIDTH / 2.0f - speedTextSize.x / 2.0f,
                        WINDOW_HEIGHT / 2.0f + 100};
    Rectangle speedRect = {speedPos.x - 20, speedPos.y - 10,
                           speedTextSize.x + 40, speedTextSize.y + 20};
    bool speedHovering = CheckCollisionPointRec(mousePos, speedRect);
    DrawRectangleRounded(speedRect, 0.3f, 10,
                         speedHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (speedHovering)
      DrawRectangleRoundedLines(speedRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, speedText, speedPos, 18, 1,
               speedHovering ? BLACK : (Color){230, 250, 240, 255});

    // Scoreboard Button
    Vector2 sbTextSize =
        MeasureTextEx(game->assets.mainFont, "SCOREBOARD", 18, 1);
    Vector2 sbHintPos = {WINDOW_WIDTH / 2.0f - sbTextSize.x / 2.0f,
                         WINDOW_HEIGHT / 2.0f + 130};
    Rectangle sbRect = {sbHintPos.x - 20, sbHintPos.y - 10, sbTextSize.x + 40,
                        sbTextSize.y + 20};
    bool sbHovering = CheckCollisionPointRec(mousePos, sbRect);
    DrawRectangleRounded(sbRect, 0.3f, 10,
                         sbHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (sbHovering)
      DrawRectangleRoundedLines(sbRect, 0.3f, 10, 2, ORANGE);
    DrawTextEx(game->assets.mainFont, "SCOREBOARD", sbHintPos, 18, 1,
               sbHovering ? ORANGE : (Color){230, 250, 240, 255});

    // Hidden Dev Mode UI
    if (game->devModeActive) {
      const char *devText =
          TextFormat("DEV MODE ACTIVE - LEVEL: %d", game->devLevelSelect);
      Vector2 devPos = {
          WINDOW_WIDTH / 2.0f -
              MeasureTextEx(game->assets.mainFont, devText, 16, 1).x / 2.0f,
          WINDOW_HEIGHT / 2.0f + 160};
      DrawTextEx(game->assets.mainFont, devText, devPos, 16, 1, YELLOW);

      const char *promptText = "< LEFT / RIGHT TO CHANGE >";
      Vector2 promptPos = {
          WINDOW_WIDTH / 2.0f -
              MeasureTextEx(game->assets.mainFont, promptText, 12, 1).x / 2.0f,
          WINDOW_HEIGHT / 2.0f + 185};
      DrawTextEx(game->assets.mainFont, promptText, promptPos, 12, 1, GOLD);
    }
  } else if (game->state == STATE_PLAYING) {
    DrawTextEx(game->assets.mainFont, TextFormat("Score: %d", game->score),
               (Vector2){10, 10}, 20, 1, (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont,
               TextFormat("Best: %d", game->highScores[0].score),
               (Vector2){10, 35}, 16, 1, (Color){230, 250, 240, 255});

    Vector2 mousePos = GetMousePosition();

    if (game->isPaused) {
      // Draw semi-transparent overlay
      DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});

      Vector2 mPos = GetMousePosition();

      // PAUSE Title
      const char *pTitle = "GAME PAUSED";
      Vector2 pTxtSize = MeasureTextEx(game->assets.mainFont, pTitle, 50, 2);
      DrawTextEx(game->assets.mainFont, pTitle,
                 (Vector2){WINDOW_WIDTH / 2.0f - pTxtSize.x / 2.0f,
                           WINDOW_HEIGHT / 2.0f - 120},
                 50, 2, WHITE);

      // Resume Button Drawing
      Rectangle resumeRect = {WINDOW_WIDTH / 2.0f - 150, WINDOW_HEIGHT / 2.0f - 60, 300, 80};
      bool resumeHover = CheckCollisionPointRec(mPos, resumeRect);
      DrawRectangleRounded(resumeRect, 0.3f, 10, resumeHover ? (Color){230, 250, 240, 100} : (Color){230, 250, 240, 40});
      DrawRectangleRoundedLines(resumeRect, 0.3f, 10, 2, WHITE);
      Vector2 resTextSize = MeasureTextEx(game->assets.mainFont, "RESUME", 30, 1);
      DrawTextEx(game->assets.mainFont, "RESUME", (Vector2){resumeRect.x + (resumeRect.width - resTextSize.x) / 2.0f, resumeRect.y + (resumeRect.height - resTextSize.y) / 2.0f}, 30, 1, WHITE);

      // Back to Menu Button Drawing
      Rectangle backRect = {WINDOW_WIDTH / 2.0f - 225, WINDOW_HEIGHT / 2.0f + 40, 450, 80};
      bool backHover = CheckCollisionPointRec(mPos, backRect);
      DrawRectangleRounded(backRect, 0.3f, 10, backHover ? (Color){230, 250, 240, 100} : (Color){230, 250, 240, 40});
      DrawRectangleRoundedLines(backRect, 0.3f, 10, 2, WHITE);
      Vector2 backTextSize = MeasureTextEx(game->assets.mainFont, "BACK TO MENU", 30, 1);
      DrawTextEx(game->assets.mainFont, "BACK TO MENU", (Vector2){backRect.x + (backRect.width - backTextSize.x) / 2.0f, backRect.y + (backRect.height - backTextSize.y) / 2.0f}, 30, 1, WHITE);
    }
  } else if (game->state == STATE_GAME_OVER) {
    Vector2 overPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "GAME OVER!", 50, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "GAME OVER!", overPos, 50, 2,
               (Color){230, 250, 240, 255});

    Vector2 scorePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont,
                          TextFormat("Final Score: %d", game->score), 20, 1)
                    .x /
                2.0f,
        WINDOW_HEIGHT / 2.0f + 10};
    DrawTextEx(game->assets.mainFont,
               TextFormat("Final Score: %d", game->score), scorePos, 20, 1,
               (Color){230, 250, 240, 255});

    Vector2 mousePos = GetMousePosition();
    Rectangle reRect = {WINDOW_WIDTH / 2.0f - 80, WINDOW_HEIGHT / 2.0f + 30,
                        160, 40};
    bool reHover = CheckCollisionPointRec(mousePos, reRect);
    DrawRectangleRounded(reRect, 0.3f, 10,
                         reHover ? (Color){230, 250, 240, 60}
                                 : (Color){230, 250, 240, 20});
    DrawRectangleRoundedLines(reRect, 0.3f, 10, 2, WHITE);

    Vector2 reTextSize =
        MeasureTextEx(game->assets.mainFont, "TRY AGAIN", 20, 1);
    DrawTextEx(game->assets.mainFont, "TRY AGAIN",
               (Vector2){reRect.x + reRect.width / 2 - reTextSize.x / 2,
                         reRect.y + reRect.height / 2 - reTextSize.y / 2},
               20, 1, WHITE);
  } else if (game->state == STATE_GAME_WIN) {
    Vector2 winPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "CHAMPION!", 60, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "CHAMPION!", winPos, 60, 2,
               (Color){230, 250, 240, 255});

    Vector2 subWinPos = {WINDOW_WIDTH / 2.0f -
                             MeasureTextEx(game->assets.mainFont,
                                           "You conquered the game!", 20, 1)
                                     .x /
                                 2.0f,
                         WINDOW_HEIGHT / 2.0f + 10};
    DrawTextEx(game->assets.mainFont, "You conquered the game!", subWinPos, 20,
               1, (Color){230, 250, 240, 255});

    Vector2 mousePos = GetMousePosition();
    Rectangle reRect = {WINDOW_WIDTH / 2.0f - 80, WINDOW_HEIGHT / 2.0f + 40,
                        160, 40};
    bool reHover = CheckCollisionPointRec(mousePos, reRect);
    DrawRectangleRounded(reRect, 0.3f, 10,
                         reHover ? (Color){230, 250, 240, 60}
                                 : (Color){230, 250, 240, 20});
    DrawRectangleRoundedLines(reRect, 0.3f, 10, 2, WHITE);

    Vector2 reTextSize =
        MeasureTextEx(game->assets.mainFont, "PLAY AGAIN", 20, 1);
    DrawTextEx(game->assets.mainFont, "PLAY AGAIN",
               (Vector2){reRect.x + reRect.width / 2 - reTextSize.x / 2,
                         reRect.y + reRect.height / 2 - reTextSize.y / 2},
               20, 1, WHITE);
  } else if (game->state == STATE_NAME_INPUT) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "NEW HIGH SCORE!", 40, 2).x /
                2.0f,
        WINDOW_HEIGHT / 2.0f - 80};
    DrawTextEx(game->assets.mainFont, "NEW HIGH SCORE!", titlePos, 40, 2,
               (Color){230, 250, 240, 255});

    Vector2 promptPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "Enter your name:", 20, 1).x /
                2.0f,
        WINDOW_HEIGHT / 2.0f - 20};
    DrawTextEx(game->assets.mainFont, "Enter your name:", promptPos, 20, 1,
               (Color){230, 250, 240, 255});

    char displayName[32];
    snprintf(displayName, 32, "%s_", game->currentName);
    Vector2 namePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, displayName, 30, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 20};
    DrawTextEx(game->assets.mainFont, displayName, namePos, 30, 2,
               (Color){230, 250, 240, 255});

    Vector2 subPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "SUBMIT", 15, 1).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 80};
    DrawTextEx(game->assets.mainFont, "SUBMIT", subPos, 15, 1,
               (Color){200, 220, 210, 255});
  }

  EndDrawing();
}

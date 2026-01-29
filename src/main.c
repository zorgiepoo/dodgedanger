/*
 MIT License

 Copyright (c) 2024 Mayur Pawashe

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "app.h"
#include "platforms.h"
#include "renderer.h"
#include "window.h"
#include "font.h"
#include "text.h"
#include "zgtime.h"
#include "thread.h"
#include "quit.h"
#include "mt_random.h"
#include "renderer_projection.h"
#include "gamepad.h"
#include "defaults.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"

#define MAX_FPS_RATE 120
#define ANIMATION_TIMER_INTERVAL 0.01666 // in seconds
#define MAX_ITERATIONS (25 * ANIMATION_TIMER_INTERVAL)

#define FONT_SYSTEM_NAME "Times New Roman"
#define FONT_POINT_SIZE 144

#define TEXT_RENDERING_CACHE_COUNT 32

#if PLATFORM_OSX
#define WINDOW_TITLE ""
#else
#define WINDOW_TITLE "Dodge Danger"
#endif

#define MAX_CUBE_COUNT 4096
#define MAX_BOUNDARY_X_MAGNITUDE 8
#define MAX_BOUNDARY_RENDER_GAP 0.2
#define PLAYER_MAGNITUDE 0.05f
#define CUBE_MAGNITUDE 1.0f
#define PLAYER_INITIAL_SPEED 4.0f
#define PLAYER_SPEED_CAP 20.0f
#define PLAYER_SPEED_INCREASE 0.2f
#define CUBE_PLAYER_DIST_AWAY 100.0f
#define CUBE_PLAYER_CROSS_DIST_AWAY 40.0f
#define CUBE_PLAYER_WARN_MAX_FACTOR 8
#define CUBE_PLAYER_WARN_FUTURE_MAX_ITERATIONS 100

#define HIGH_SCORE_USER_DEFAULTS_KEY "high_score"
#define FULLSCREEN_USER_DEFAULTS_KEY "fullscreen"
#define WINDOW_WIDTH_USER_DEFAULTS_KEY "window_width"
#define WINDOW_HEIGHT_USER_DEFAULTS_KEY "window_height"
#define USER_DEFAULTS_NAME "dodgedanger"

typedef struct
{
    vec3_t position;
    color4_t color;
    bool dead;
    bool warning;
} Cube;

typedef struct
{
    vec3_t playerPosition;
    ZGFloat playerSpeed;
    uint32_t score;
    double timer;
    
    Cube *cubes;
    
    bool paused;
    bool playerLost;
    
    bool playerDirectionRight;
    bool playerDirectionLeft;
    
    bool exitOptionSelected;
    bool renderInstruction;
} Game;

// This struct only exists to add another level of indirection that makes it slightly more challenging for cheat tools to find
typedef struct
{
    uint32_t numberOfGamesPlayed;
    Game *game;
} GameSeries;

typedef struct
{
    GameSeries *gameSeries;
    Renderer renderer;
    
    BufferArrayObject cubeVertexArrayObject;
    BufferObject cubeIndicesBufferObject;
    BufferObject cubeLineIndicesBufferObject;
    
    GamepadManager *gamepadManager;
    
    double lastFrameTime;
    double cyclesLeftOver;
    
    ZGFloat playerCubeDiagonalSumDistance;
    
    uint32_t lastRunloopTime;
    
    uint32_t highScore;
    
    bool needsToDrawScene;
    bool playOptionSelected;
} AppContext;

static void drawScene(Renderer *renderer, void *context)
{
    AppContext *appContext = context;
    GameSeries *gameSeries = appContext->gameSeries;
    
    if (gameSeries == NULL)
    {
        {
            ZGFloat scale = 0.015f;
            
            color4_t color = (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
            
            mat4_t titleModelViewMatrix = m4_translation((vec3_t){0.0f, 16.0f, -70.0f});
            drawStringScaled(renderer, titleModelViewMatrix, color, scale, "Dodge Danger");
        }
        
        {
            bool playOptionSelected = appContext->playOptionSelected;
            
            ZGFloat scale = 0.01f;
            color4_t selectedColor = (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
            color4_t nonSelectedColor = (color4_t){1.0f, 1.0f, 1.0f, 0.5f};
            
            mat4_t playModelViewMatrix = m4_translation((vec3_t){0.0f, 5.0f, -70.0f});
            drawStringScaled(renderer, playModelViewMatrix, (playOptionSelected ? selectedColor : nonSelectedColor), scale, "Play");
            
            mat4_t quitModelViewMatrix = m4_mul(playModelViewMatrix, m4_translation((vec3_t){0.0f, -5.0f, 0.0f}));
            drawStringScaled(renderer, quitModelViewMatrix, (!playOptionSelected ? selectedColor : nonSelectedColor), scale, "Quit");
        }
    }
    else
    {
        Game *game = gameSeries->game;
        
        mat4_t worldRotationMatrix = m4_rotation_x(0.0f * ((ZGFloat)M_PI / 180.0f));
        
        vec3_t playerPosition = game->playerPosition;
        
        mat4_t playerModelTranslationMatrix = m4_translation((vec3_t){-playerPosition.x, -playerPosition.y, -playerPosition.z});
        
        // Draw walls boundary
        {
            mat4_t rotatedModelViewMatrix = m4_mul(worldRotationMatrix, playerModelTranslationMatrix);
            mat4_t scalingMatrix = m4_scaling(vec3((ZGFloat)(MAX_BOUNDARY_X_MAGNITUDE + MAX_BOUNDARY_RENDER_GAP), 1.0f, playerPosition.z + PROJECTION_FAR_VIEW_DISTANCE));
            mat4_t modelViewMatrix = m4_mul(rotatedModelViewMatrix, scalingMatrix);
            
            color4_t color = (color4_t){0.0f, 1.0f, 0.0f, 1.0f};
            drawVerticesFromIndices(renderer, modelViewMatrix, RENDERER_LINE_MODE, appContext->cubeVertexArrayObject, appContext->cubeLineIndicesBufferObject, 48, color, RENDERER_OPTION_NONE);
        }
        
        // Draw cubes
        Cube *cubes = game->cubes;
        for (uint32_t cubeIndex = 0; cubeIndex < MAX_CUBE_COUNT; cubeIndex++)
        {
            if (cubes[cubeIndex].dead || cubes[cubeIndex].position.z < playerPosition.z - CUBE_PLAYER_DIST_AWAY)
            {
                continue;
            }
            
            vec3_t cubePosition = cubes[cubeIndex].position;
            mat4_t cubeModelTranslationMatrix = m4_mul(playerModelTranslationMatrix, m4_translation(cubePosition));
            
            mat4_t rotatedModelViewMatrix = m4_mul(worldRotationMatrix, cubeModelTranslationMatrix);
            // Make sure cubes don't quite touch each other from rendering perspective
            mat4_t scalingMatrix = m4_scaling(vec3(0.99f, 1.0f, 1.0f));
            mat4_t modelViewMatrix = m4_mul(rotatedModelViewMatrix, scalingMatrix);
            
            color4_t cubeColor = cubes[cubeIndex].warning ? (color4_t){1.0f, 1.0f, 0.0f, 1.0f} : cubes[cubeIndex].color;
            
            {
                uint32_t indicesCount;
                if (cubes[cubeIndex].position.z < playerPosition.z - CUBE_PLAYER_CROSS_DIST_AWAY)
                {
                    indicesCount = 48;
                }
                else
                {
                    indicesCount = 52;
                }
                
                drawVerticesFromIndices(renderer, modelViewMatrix, RENDERER_LINE_MODE, appContext->cubeVertexArrayObject, appContext->cubeLineIndicesBufferObject, indicesCount, cubeColor, RENDERER_OPTION_NONE);
            }
        }
        
        // Draw score
        {
            ZGFloat scale = 0.008f;
            color4_t color = (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
            
            mat4_t scoreModelViewMatrix = m4_translation((vec3_t){-25.0f, 26.0f, -70.0f});
            
            char scoreBuffer[256] = {0};
            snprintf(scoreBuffer, sizeof(scoreBuffer) - 1, "Score: %u", game->score);
            drawStringLeftAligned(renderer, scoreModelViewMatrix, color, scale, scoreBuffer);
        }
        
        // Draw dodge!
        if (game->renderInstruction)
        {
            ZGFloat scale = 0.01f;
            color4_t color = cubes[0].warning ? (color4_t){1.0f, 1.0f, 0.0f, 1.0f} : (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
            
            mat4_t scoreModelViewMatrix = m4_translation((vec3_t){0.0f, 14.0f, -70.0f});
            
            drawStringScaled(renderer, scoreModelViewMatrix, color, scale, "Dodge!");
        }
        
        if (game->playerLost)
        {
            // Draw high score
            {
                ZGFloat scale = 0.008f;
                color4_t color = (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
                
                mat4_t scoreModelViewMatrix = m4_translation((vec3_t){-25.0f, 22.0f, -70.0f});
                
                char scoreBuffer[256] = {0};
                snprintf(scoreBuffer, sizeof(scoreBuffer) - 1, "High Score: %u", appContext->highScore);
                drawStringLeftAligned(renderer, scoreModelViewMatrix, color, scale, scoreBuffer);
            }
            
            {
                ZGFloat scale = 0.01f;
                color4_t selectedColor = (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
                color4_t nonSelectedColor = (color4_t){1.0f, 1.0f, 1.0f, 0.7f};
                
                bool exitOptionSelected = game->exitOptionSelected;
                
                mat4_t playModelViewMatrix = m4_translation((vec3_t){0.0f, 5.0f, -70.0f});
                drawStringScaled(renderer, playModelViewMatrix, !exitOptionSelected ? selectedColor : nonSelectedColor, scale, "Play Again");
                
                mat4_t exitModelViewMatrix = m4_translation((vec3_t){0.0f, -5.0f, -70.0f});
                drawStringScaled(renderer, exitModelViewMatrix, exitOptionSelected ? selectedColor : nonSelectedColor, scale, "Exit");
            }
        }
        else if (game->paused)
        {
            ZGFloat scale = 0.01f;
            color4_t selectedColor = (color4_t){1.0f, 1.0f, 1.0f, 1.0f};
            color4_t nonSelectedColor = (color4_t){1.0f, 1.0f, 1.0f, 0.7f};
            
            bool exitOptionSelected = game->exitOptionSelected;
            
            mat4_t playModelViewMatrix = m4_translation((vec3_t){0.0f, 5.0f, -70.0f});
            drawStringScaled(renderer, playModelViewMatrix, !exitOptionSelected ? selectedColor : nonSelectedColor, scale, "Resume");
            
            mat4_t exitModelViewMatrix = m4_translation((vec3_t){0.0f, -5.0f, -70.0f});
            drawStringScaled(renderer, exitModelViewMatrix, exitOptionSelected ? selectedColor : nonSelectedColor, scale, "Exit");
        }
    }
}

static void generateCubePositions(Game *game, uint32_t startingIndex)
{
    game->playerPosition = vec3(0.0f, 0.0f, 20.0f);
    
    Cube *cubes = game->cubes;
    
    cubes[0].position = vec3(0.0f, 0.0f, 0.0f);
    cubes[0].color = (color4_t){0.0f, 1.0f, 0.0f, 1.0f};
    cubes[0].dead = false;
    cubes[0].warning = false;
    
    uint32_t currentCubeIndex = startingIndex;
    ZGFloat startDepth = startingIndex > 0 ? -CUBE_MAGNITUDE * 2 * 5 : 0.0f;
    
    color4_t colors[] = {(color4_t){0.0f, 1.0f, 0.0f, 1.0f}, (color4_t){0.0f, 0.0f, 1.0f, 1.0f}, (color4_t){0.7f, 0.0f, 0.0f, 1.0f}, (color4_t){1.0f, 0.0f, 1.0f, 1.0f}, (color4_t){0.2f, 0.2f, 0.5f, 1.0f}};
    
    const uint32_t maxCountPerExpertLevel = 5;
    const uint32_t maxCountPerMediumLevel = 4;
    const uint32_t maxCountPerBeginnerLevel = 3;
    
    const uint32_t cubeCountForMediumLevelEntry = 50;
    const uint32_t cubeCountForExpertLevelEntry = 100;
    
    uint32_t *prevRandomXIndices = calloc(maxCountPerExpertLevel, sizeof(*prevRandomXIndices));
    const uint32_t maxDepthIncreaseCount = 5;
    
    const uint32_t maxCubesPerLevel = (uint32_t)((MAX_BOUNDARY_X_MAGNITUDE * 2.0f) / (CUBE_MAGNITUDE * 2.0f));
    
    while (currentCubeIndex < MAX_CUBE_COUNT)
    {
        uint32_t maxCountPerLevel;
        if (currentCubeIndex < cubeCountForMediumLevelEntry)
        {
            maxCountPerLevel = maxCountPerBeginnerLevel;
        }
        else if (currentCubeIndex < cubeCountForExpertLevelEntry)
        {
            maxCountPerLevel = maxCountPerMediumLevel;
        }
        else
        {
            maxCountPerLevel = maxCountPerExpertLevel;
        }
        
        uint32_t countPerLevel;
        if (MAX_CUBE_COUNT - currentCubeIndex < maxCountPerLevel)
        {
            countPerLevel = (uint32_t)(mt_random() % (MAX_CUBE_COUNT - currentCubeIndex)) + 1;
        }
        else
        {
            countPerLevel = (uint32_t)(mt_random() % maxCountPerLevel) + 1;
        }
        
        for (uint32_t cubeLevelIndex = 0; cubeLevelIndex < countPerLevel; cubeLevelIndex++)
        {
            uint32_t randomXIndex;
            do
            {
                uint32_t randomNumber = (uint32_t)(mt_random() % maxCubesPerLevel);
                if (cubeLevelIndex == 0)
                {
                    randomXIndex = randomNumber;
                    prevRandomXIndices[cubeLevelIndex] = randomXIndex;
                    break;
                }
                else
                {
                    bool foundPrevRandomIndex = false;
                    for (uint32_t prevIndex = 0; prevIndex < cubeLevelIndex; prevIndex++)
                    {
                        if (prevRandomXIndices[prevIndex] == randomNumber)
                        {
                            foundPrevRandomIndex = true;
                            break;
                        }
                    }
                    
                    if (!foundPrevRandomIndex)
                    {
                        randomXIndex = randomNumber;
                        prevRandomXIndices[cubeLevelIndex] = randomXIndex;
                        break;
                    }
                }
            }
            while (true);
            
            vec3_t cubePosition = vec3((ZGFloat)(-MAX_BOUNDARY_X_MAGNITUDE + CUBE_MAGNITUDE) + (ZGFloat)randomXIndex * (CUBE_MAGNITUDE * 2), 0.0f, startDepth);
            cubes[currentCubeIndex].position = cubePosition;
            
            uint32_t colorIndex = (uint32_t)(mt_random() % (sizeof(colors) / sizeof(colors[0])));
            cubes[currentCubeIndex].color = colors[colorIndex];
            cubes[currentCubeIndex].dead = false;
            cubes[currentCubeIndex].warning = false;
            
            currentCubeIndex++;
        }
        
        uint32_t depthDecrease = (uint32_t)(mt_random() % maxDepthIncreaseCount);
        startDepth -= CUBE_MAGNITUDE * 2 * (2 + depthDecrease);
    }
    
    free(prevRandomXIndices);
}

static void animate(double timeDelta, AppContext *appContext)
{
    GameSeries *gameSeries = appContext->gameSeries;
    if (gameSeries == NULL)
    {
        return;
    }
    
    Game *game = gameSeries->game;
    if (game == NULL || game->playerLost || game->paused)
    {
        return;
    }
    
    game->timer += timeDelta;
    
    if (game->timer >= 4.0f)
    {
        game->renderInstruction = false;
    }
    
    ZGFloat deltaX;
    if (game->playerDirectionRight && game->playerDirectionLeft)
    {
        deltaX = 0.0f;
    }
    else if (game->playerDirectionRight && game->playerPosition.x + PLAYER_MAGNITUDE <= (ZGFloat)MAX_BOUNDARY_X_MAGNITUDE)
    {
        deltaX = 1.0f;
    }
    else if (game->playerDirectionLeft && game->playerPosition.x - PLAYER_MAGNITUDE >= (ZGFloat)(-MAX_BOUNDARY_X_MAGNITUDE))
    {
        deltaX = -1.0f;
    }
    else
    {
        deltaX = 0.0f;
    }
    
    vec3_t deltaVector = v3_muls(v3_norm(vec3(deltaX, 0.0f, -1.0f)), (ZGFloat)(timeDelta * game->playerSpeed));
    game->playerPosition = v3_add(game->playerPosition, deltaVector);
    
    vec3_t playerPosition = game->playerPosition;
    
    bool foundAliveCube = false;
    Cube *cubes = game->cubes;
    for (uint32_t cubeIndex = 0; cubeIndex < MAX_CUBE_COUNT; cubeIndex++)
    {
        Cube cube = cubes[cubeIndex];
        if (cube.dead)
        {
            continue;
        }
        
        foundAliveCube = true;
        
        ZGFloat distance = sqrtf((cube.position.x - playerPosition.x) * (cube.position.x - playerPosition.x) + (cube.position.y - playerPosition.y) * (cube.position.y - playerPosition.y) + (cube.position.z - playerPosition.z) * (cube.position.z - playerPosition.z));
        
        if (distance <= appContext->playerCubeDiagonalSumDistance)
        {
            // Player loses here
            game->playerLost = true;
            if (game->score > appContext->highScore)
            {
                appContext->highScore = game->score;
            }
            
            gameSeries->numberOfGamesPlayed++;
            
            ZGAppSetAllowsScreenIdling(true);
            break;
        }
        else if (playerPosition.z - PLAYER_MAGNITUDE < cube.position.z + CUBE_MAGNITUDE)
        {
            cubes[cubeIndex].dead = true;
            game->score++;
            
            if (game->playerSpeed < PLAYER_SPEED_CAP)
            {
                game->playerSpeed += PLAYER_SPEED_INCREASE;
                if (game->playerSpeed > PLAYER_SPEED_CAP)
                {
                    game->playerSpeed = PLAYER_SPEED_CAP;
                }
            }
        }
        else if (distance <= appContext->playerCubeDiagonalSumDistance * CUBE_PLAYER_WARN_MAX_FACTOR)
        {
            bool foundFutureCollision = false;
            vec3_t playerPositionInFuture = v3_add(game->playerPosition, deltaVector);
            for (uint32_t collideInFutureIndex = 0; collideInFutureIndex < CUBE_PLAYER_WARN_FUTURE_MAX_ITERATIONS; collideInFutureIndex++)
            {
                playerPositionInFuture = v3_add(playerPositionInFuture, deltaVector);
                
                ZGFloat collideInFutureDistance = sqrtf((cube.position.x - playerPositionInFuture.x) * (cube.position.x - playerPositionInFuture.x) + (cube.position.y - playerPositionInFuture.y) * (cube.position.y - playerPositionInFuture.y) + (cube.position.z - playerPositionInFuture.z) * (cube.position.z - playerPositionInFuture.z));
                
                if (collideInFutureDistance <= appContext->playerCubeDiagonalSumDistance)
                {
                    foundFutureCollision = true;
                    break;
                }
                else if (playerPositionInFuture.z - PLAYER_MAGNITUDE < cube.position.z + CUBE_MAGNITUDE)
                {
                    break;
                }
            }
            
            cubes[cubeIndex].warning = foundFutureCollision;
        }
        else
        {
            cubes[cubeIndex].warning = false;
        }
    }
    
    if (!foundAliveCube)
    {
        generateCubePositions(game, 0);
    }
}

static void appTerminatedHandler(void *context)
{
    AppContext *appContext = context;
    
    Defaults userDefaults = userDefaultsForWriting(USER_DEFAULTS_NAME);
    
    writeDefaultIntKey(userDefaults, HIGH_SCORE_USER_DEFAULTS_KEY, (int)appContext->highScore);
    writeDefaultIntKey(userDefaults, FULLSCREEN_USER_DEFAULTS_KEY, (int)appContext->renderer.fullscreen);
    writeDefaultIntKey(userDefaults, WINDOW_WIDTH_USER_DEFAULTS_KEY, (int)appContext->renderer.windowWidth);
    writeDefaultIntKey(userDefaults, WINDOW_HEIGHT_USER_DEFAULTS_KEY, (int)appContext->renderer.windowHeight);
    
    closeDefaults(userDefaults);
}

static void handleWindowEvent(ZGWindowEvent event, void *context)
{
    AppContext *appContext = context;
    GameSeries *gameSeries = appContext->gameSeries;
    Game *game = (gameSeries != NULL) ? gameSeries->game : NULL;
    
    switch (event.type)
    {
        case ZGWindowEventTypeResize:
            updateViewport(&appContext->renderer, event.width, event.height);
            break;
        case ZGWindowEventTypeFocusGained:
            break;
        case ZGWindowEventTypeFocusLost:
            if (game != NULL)
            {
                game->paused = true;
                ZGAppSetAllowsScreenIdling(true);
            }
            break;
        case ZGWindowEventTypeShown:
            appContext->needsToDrawScene = true;
            appContext->lastRunloopTime = 0;
            break;
        case ZGWindowEventTypeHidden:
            appContext->needsToDrawScene = false;
            appContext->lastRunloopTime = 0;
            break;
#if PLATFORM_WINDOWS
        case ZGWindowEventDeviceConnected:
            pollNewGamepads(appContext->gamepadManager);
            break;
#endif
    }
}

static void destroyGame(AppContext *appContext)
{
    GameSeries *gameSeries = appContext->gameSeries;
    if (gameSeries != NULL)
    {
        free(gameSeries->game->cubes);
        free(gameSeries->game);
        free(gameSeries);
        appContext->gameSeries = NULL;
    }
    
    ZGAppSetAllowsScreenIdling(true);
}

static void createNewGame(AppContext *appContext)
{
    if (appContext->gameSeries == NULL)
    {
        appContext->gameSeries = calloc(1, sizeof(*appContext->gameSeries));
    }
    
    GameSeries *gameSeries = appContext->gameSeries;
    Game *oldGame = gameSeries->game;
    if (oldGame != NULL)
    {
        free(oldGame->cubes);
        free(oldGame);
    }
    
    Game *newGame = calloc(1, sizeof(*newGame));
    gameSeries->game = newGame;
    newGame->playerSpeed = PLAYER_INITIAL_SPEED;
    newGame->renderInstruction = true;
    
    newGame->cubes = calloc(MAX_CUBE_COUNT, sizeof(newGame->cubes[0]));
    
    generateCubePositions(newGame, 1);
    
    ZGAppSetAllowsScreenIdling(false);
}

static void handleKeyboardEvent(ZGKeyboardEvent event, void *context)
{
    AppContext *appContext = context;
    
    switch (event.type)
    {
        case ZGKeyboardEventTypeKeyDown:
        {
            GameSeries *gameSeries = appContext->gameSeries;
            if (gameSeries == NULL)
            {
                switch (event.keyCode)
                {
                    case ZG_KEYCODE_DOWN:
                    case ZG_KEYCODE_UP:
                        appContext->playOptionSelected = !appContext->playOptionSelected;
                        break;
                    default:
                        if (ZGTestReturnKeyCode(event.keyCode))
                        {
                            if (!appContext->playOptionSelected)
                            {
                                ZGSendQuitEvent();
                            }
                            else
                            {
                                createNewGame(appContext);
                            }
                        }
                        break;
                }
            }
            else
            {
                Game *game = gameSeries->game;
                if (game->playerLost)
                {
                    switch (event.keyCode)
                    {
                        case ZG_KEYCODE_ESCAPE:
                            destroyGame(appContext);
                            break;
                        case ZG_KEYCODE_UP:
                        case ZG_KEYCODE_DOWN:
                            game->exitOptionSelected = !game->exitOptionSelected;
                            break;
                        default:
                            if (ZGTestReturnKeyCode(event.keyCode))
                            {
                                if (!game->exitOptionSelected)
                                {
                                    createNewGame(appContext);
                                }
                                else
                                {
                                    destroyGame(appContext);
                                }
                            }
                            break;
                    }
                }
                else if (game->paused)
                {
                    switch (event.keyCode)
                    {
                        case ZG_KEYCODE_ESCAPE:
                            game->paused = false;
                            ZGAppSetAllowsScreenIdling(false);
                            break;
                        case ZG_KEYCODE_UP:
                        case ZG_KEYCODE_DOWN:
                            game->exitOptionSelected = !game->exitOptionSelected;
                            break;
                        default:
                            if (ZGTestReturnKeyCode(event.keyCode))
                            {
                                if (!game->exitOptionSelected)
                                {
                                    game->paused = false;
                                    ZGAppSetAllowsScreenIdling(false);
                                }
                                else
                                {
                                    destroyGame(appContext);
                                }
                            }
                            break;
                    }
                }
                else
                {
                    switch (event.keyCode)
                    {
                        case ZG_KEYCODE_ESCAPE:
                            game->paused = true;
                            ZGAppSetAllowsScreenIdling(true);
                            break;
                        case ZG_KEYCODE_RIGHT:
                        case ZG_KEYCODE_D:
                            game->playerDirectionRight = true;
                            break;
                        case ZG_KEYCODE_LEFT:
                        case ZG_KEYCODE_A:
                            game->playerDirectionLeft = true;
                            break;
                    }
                }
            }
            
            break;
        }
        case ZGKeyboardEventTypeKeyUp:
        {
            GameSeries *gameSeries = appContext->gameSeries;
            if (gameSeries != NULL)
            {
                Game *game = gameSeries->game;
                
                switch (event.keyCode)
                {
                    case ZG_KEYCODE_RIGHT:
                    case ZG_KEYCODE_D:
                        game->playerDirectionRight = false;
                        break;
                    case ZG_KEYCODE_LEFT:
                    case ZG_KEYCODE_A:
                        game->playerDirectionLeft = false;
                        break;
                }
            }
            
            break;
        }
        case ZGKeyboardEventTypeTextInput:
            break;
    }
}

static void pollEventHandler(void *context, void *systemEvent)
{
    AppContext *appContext = context;
    
    uint16_t gamepadEventsCount = 0;
    GamepadEvent *gamepadEvents = pollGamepadEvents(appContext->gamepadManager, systemEvent, &gamepadEventsCount);
    for (uint16_t gamepadEventIndex = 0; gamepadEventIndex < gamepadEventsCount; gamepadEventIndex++)
    {
        GameSeries *gameSeries = appContext->gameSeries;
        
        GamepadEvent *gamepadEvent = &gamepadEvents[gamepadEventIndex];
        
        switch (gamepadEvent->state)
        {
            case GAMEPAD_STATE_PRESSED:
                switch (gamepadEvent->button)
                {
                    case GAMEPAD_BUTTON_A:
                    case GAMEPAD_BUTTON_B:
                    case GAMEPAD_BUTTON_X:
                    case GAMEPAD_BUTTON_Y:
                    case GAMEPAD_BUTTON_LEFTSHOULDER:
                    case GAMEPAD_BUTTON_RIGHTSHOULDER:
                    case GAMEPAD_BUTTON_LEFTTRIGGER:
                    case GAMEPAD_BUTTON_RIGHTTRIGGER:
                    case GAMEPAD_BUTTON_START:
                        if (gameSeries == NULL)
                        {
                            if (!appContext->playOptionSelected)
                            {
                                ZGSendQuitEvent();
                            }
                            else
                            {
                                createNewGame(appContext);
                            }
                        }
                        else
                        {
                            Game *game = gameSeries->game;
                            if (game->playerLost)
                            {
                                if (!game->exitOptionSelected)
                                {
                                    createNewGame(appContext);
                                }
                                else
                                {
                                    destroyGame(appContext);
                                }
                            }
                            else if (game->paused)
                            {
                                if (!game->exitOptionSelected)
                                {
                                    game->paused = false;
                                    ZGAppSetAllowsScreenIdling(false);
                                }
                                else
                                {
                                    destroyGame(appContext);
                                }
                            }
                            else if (gamepadEvent->button == GAMEPAD_BUTTON_START)
                            {
                                game->paused = true;
                                ZGAppSetAllowsScreenIdling(true);
                            }
                        }
                        break;
                    case GAMEPAD_BUTTON_BACK:
                        if (gameSeries != NULL)
                        {
                            Game *game = gameSeries->game;
                            if (game->playerLost)
                            {
                                destroyGame(appContext);
                            }
                            else if (!game->paused)
                            {
                                game->paused = true;
                                ZGAppSetAllowsScreenIdling(true);
                            }
                            else
                            {
                                game->paused = false;
                                ZGAppSetAllowsScreenIdling(false);
                            }
                        }
                        break;
                    case GAMEPAD_BUTTON_DPAD_UP:
                    case GAMEPAD_BUTTON_DPAD_DOWN:
                        if (gameSeries == NULL)
                        {
                            appContext->playOptionSelected = !appContext->playOptionSelected;
                        }
                        else
                        {
                            Game *game = gameSeries->game;
                            if (game->playerLost || game->paused)
                            {
                                game->exitOptionSelected = !game->exitOptionSelected;
                            }
                        }
                        break;
                    case GAMEPAD_BUTTON_DPAD_LEFT:
                        if (gameSeries != NULL)
                        {
                            gameSeries->game->playerDirectionLeft = true;
                        }
                        break;
                    case GAMEPAD_BUTTON_DPAD_RIGHT:
                        if (gameSeries != NULL)
                        {
                            gameSeries->game->playerDirectionRight = true;
                        }
                        break;
                    case GAMEPAD_BUTTON_MAX:
                        break;
                }
                break;
            case GAMEPAD_STATE_RELEASED:
                switch (gamepadEvent->button)
                {
                    case GAMEPAD_BUTTON_A:
                    case GAMEPAD_BUTTON_B:
                    case GAMEPAD_BUTTON_X:
                    case GAMEPAD_BUTTON_Y:
                    case GAMEPAD_BUTTON_LEFTSHOULDER:
                    case GAMEPAD_BUTTON_RIGHTSHOULDER:
                    case GAMEPAD_BUTTON_LEFTTRIGGER:
                    case GAMEPAD_BUTTON_RIGHTTRIGGER:
                        break;
                    case GAMEPAD_BUTTON_BACK:
                        break;
                    case GAMEPAD_BUTTON_START:
                        break;
                    case GAMEPAD_BUTTON_DPAD_UP:
                    case GAMEPAD_BUTTON_DPAD_DOWN:
                        break;
                    case GAMEPAD_BUTTON_DPAD_LEFT:
                        if (gameSeries != NULL)
                        {
                            gameSeries->game->playerDirectionLeft = false;
                        }
                        break;
                    case GAMEPAD_BUTTON_DPAD_RIGHT:
                        if (gameSeries != NULL)
                        {
                            gameSeries->game->playerDirectionRight = false;
                        }
                        break;
                    case GAMEPAD_BUTTON_MAX:
                        break;
                }
                break;
        }
    }
}

static void runLoopHandler(void *context)
{
    AppContext *appContext = context;
    Renderer *renderer = &appContext->renderer;
    
    // Update game state
    // http://ludobloom.com/tutorials/timestep.html
    
    double currentTime = (double)ZGGetTicks() / 1000.0;
    
    double updateIterations = ((currentTime - appContext->lastFrameTime) + appContext->cyclesLeftOver);
    
    if (updateIterations > MAX_ITERATIONS)
    {
        updateIterations = MAX_ITERATIONS;
    }
    
    while (updateIterations > ANIMATION_TIMER_INTERVAL)
    {
        updateIterations -= ANIMATION_TIMER_INTERVAL;
        
        animate(ANIMATION_TIMER_INTERVAL, appContext);
    }
    
    appContext->cyclesLeftOver = updateIterations;
    appContext->lastFrameTime = currentTime;
    
    if (appContext->needsToDrawScene)
    {
        renderFrame(renderer, drawScene, context);
    }
    
    bool shouldCapFPS = !appContext->needsToDrawScene || !renderer->vsync;
    if (shouldCapFPS)
    {
        uint32_t timeAfterRender = ZGGetTicks();
        
        if (appContext->lastRunloopTime > 0 && timeAfterRender > appContext->lastRunloopTime)
        {
            uint32_t timeElapsed = timeAfterRender - appContext->lastRunloopTime;
            uint32_t targetTime = (uint32_t)(1000.0 / MAX_FPS_RATE);
            
            if (timeElapsed < targetTime)
            {
                ZGDelay(targetTime - timeElapsed);
            }
        }
        
        appContext->lastRunloopTime = ZGGetTicks();
    }
}

static ZGWindow *appLaunchedHandler(void *context)
{
    AppContext *appContext = context;
    
    mt_init();
    
    appContext->playOptionSelected = true;
    
    appContext->lastRunloopTime = 0;
    appContext->lastFrameTime = 0.0;
    appContext->cyclesLeftOver = 0.0;
    appContext->needsToDrawScene = true;
    
    Defaults userDefaults = userDefaultsForReading(USER_DEFAULTS_NAME);

    appContext->highScore = (uint32_t)readDefaultIntKey(userDefaults, HIGH_SCORE_USER_DEFAULTS_KEY, 0);
    bool fullscreen = readDefaultBoolKey(userDefaults, FULLSCREEN_USER_DEFAULTS_KEY, false);
    int windowWidth = readDefaultIntKey(userDefaults, WINDOW_WIDTH_USER_DEFAULTS_KEY, 800);
    int windowHeight = readDefaultIntKey(userDefaults, WINDOW_HEIGHT_USER_DEFAULTS_KEY, 500);
    
    closeDefaults(userDefaults);
    
    ZGFloat playerDiagonalDistance = sqrtf((PLAYER_MAGNITUDE * PLAYER_MAGNITUDE) + (PLAYER_MAGNITUDE * PLAYER_MAGNITUDE));
    ZGFloat cubeDiagonalDistance = sqrtf((CUBE_MAGNITUDE * CUBE_MAGNITUDE) + (CUBE_MAGNITUDE * CUBE_MAGNITUDE));
    appContext->playerCubeDiagonalSumDistance = playerDiagonalDistance + cubeDiagonalDistance;
    
    appContext->gamepadManager = initGamepadManager(NULL, NULL, NULL, NULL);
    
    Renderer *renderer = &appContext->renderer;
    
    RendererCreateOptions rendererOptions = { 0 };
    
    rendererOptions.clearColor = (color4_t){0.4f, 0.4f, 0.4f, 1.0f};
    rendererOptions.windowTitle = WINDOW_TITLE;
    rendererOptions.windowWidth = windowWidth;
    rendererOptions.windowHeight = windowHeight;
    rendererOptions.fullscreen = fullscreen;
    rendererOptions.vsync = true;
    rendererOptions.fsaa = true;

    rendererOptions.windowEventHandler = handleWindowEvent;
    rendererOptions.windowEventContext = appContext;
    rendererOptions.keyboardEventHandler = handleKeyboardEvent;
    rendererOptions.keyboardEventContext = appContext;

    createRenderer(renderer, rendererOptions);
    
    // Create vertex/index data for our cubes
    {
        const ZGFloat vertices[] =
        {
            // Bottom
            -1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f,
            
            // Left
            -1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 1.0f,
            
            // Right
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f,
            
            // Front
            -1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            
            // Back
            -1.0f, 1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f,
            
            // Top
            -1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
        };
        
        const uint16_t indices[] =
        {
            // Bottom
            0, 1, 2,
            2, 3, 0,
            
            // Left
            4, 5, 6,
            6, 7, 4,
            
            // Right
            8, 9, 10,
            10, 11, 8,
            
            // Front
            12, 13, 14,
            14, 15, 12,
            
            // Back
            16, 17, 18,
            18, 19, 16,
            
            // Top
            20, 21, 22,
            22, 23, 20,
        };
        
        const uint16_t lineIndices[] =
        {
            // Bottom
            0, 1,
            1, 2,
            2, 3,
            0, 3,
            
            // Left
            4, 5,
            5, 6,
            6, 7,
            4, 7,
            
            // Right
            8, 9,
            9, 10,
            10, 11,
            8, 11,
            
            // Front
            12, 13,
            13, 14,
            14, 15,
            12, 15,
            
            // Back
            16, 17,
            17, 18,
            18, 19,
            16, 19,
            
            // Top
            20, 21,
            21, 22,
            22, 23,
            20, 23,
            
            // Front X
            12, 14,
            13, 15
        };
        
        appContext->cubeVertexArrayObject = createVertexArrayObject(renderer, vertices, sizeof(vertices));
        appContext->cubeIndicesBufferObject = createIndexBufferObject(renderer, indices, sizeof(indices));
        appContext->cubeLineIndicesBufferObject = createIndexBufferObject(renderer, lineIndices, sizeof(lineIndices));
    }
    
    initFontWithName(FONT_SYSTEM_NAME, FONT_POINT_SIZE);
    initText(renderer, TEXT_RENDERING_CACHE_COUNT);
    
    return renderer->window;
}

int main(int argc, char *argv[])
{
    static AppContext appContext;

    ZGAppHandlers appHandlers = {.launchedHandler = appLaunchedHandler, .terminatedHandler = appTerminatedHandler, .runLoopHandler = runLoopHandler, .pollEventHandler = pollEventHandler};
    return ZGAppInit(argc, argv, &appHandlers, &appContext);
}

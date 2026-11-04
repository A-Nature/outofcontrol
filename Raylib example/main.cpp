#include "outOfControl.hpp"
#include <cmath>

int main(){
    const int SCREEN_WIDTH = 1600;
    const int SCREEN_HEIGHT = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Out of Control");
    int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
    int monitorHeight = GetMonitorHeight(GetCurrentMonitor());
    float fitScale = fminf(1.0f, fminf(monitorWidth * 0.95f / SCREEN_WIDTH, monitorHeight * 0.85f / SCREEN_HEIGHT));
    SetWindowSize(SCREEN_WIDTH * fitScale, SCREEN_HEIGHT * fitScale);
    SetWindowPosition((monitorWidth - SCREEN_WIDTH * fitScale) / 2, (monitorHeight - SCREEN_HEIGHT * fitScale) / 2 + 20);
    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    SetTargetFPS(60);

    Texture2D gameMap = LoadTexture("sprite/map/gameMap.png");
    Texture2D trooperWalk = LoadTexture("sprite/trooper/trooperwalk.png");
    Texture2D trooperAttack = LoadTexture("sprite/trooper/trooperhit.png");
    Texture2D trooperDie = LoadTexture("sprite/trooper/trooperdie.png");
    Texture2D hulkWalk = LoadTexture("sprite/hulk/hulkwalk.png");
    Texture2D hulkAttack = LoadTexture("sprite/hulk/hulkhit.png");
    Texture2D hulkDie = LoadTexture("sprite/hulk/hulkdie.png");
    Texture2D gatlingSentryFire = LoadTexture("sprite/turrets/gatlingsentry.png");
    Texture2D emptySlotSprite = LoadTexture("sprite/turrets/emptyturretblock.png");

    vector<enemy> enemies;
    vector<deadEnemy> deadEnemies;
    vector<buildSpot> buildSpots;
    vector<turret> turrets;

    buildSpots.push_back(buildSpot(193, 343));
    buildSpots.push_back(buildSpot(354, 343));
    buildSpots.push_back(buildSpot(539, 254));
    buildSpots.push_back(buildSpot(780, 270));
    buildSpots.push_back(buildSpot(990, 550));
    buildSpots.push_back(buildSpot(1170, 610));
    buildSpots.push_back(buildSpot(1230, 110));

    int health = 4000;
    int maxHealth = 4000;

    int superCredits = 100;
    int level = 1;
    float levelIntervalCurrent = 10;
    float levelProgressTimer = 0;
    float levelBreakTimer = 0;
    bool onLevelBreak = false;

    float trooperSpawnTimer = 0;
    float trooperSpawnInterval = 3;

    float hulkSpawnTimer = 0;
    float hulkSpawnInterval = 7.3; 
    bool hulksSpawn = false;

    while (!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        Vector2 mouse = GetMousePosition();

        if (IsKeyPressed(KEY_F)){
            ToggleFullscreen();
        }
        float screenScale = fminf(GetScreenWidth() / (float)SCREEN_WIDTH, GetScreenHeight() / (float)SCREEN_HEIGHT);
        float screenOffsetX = (GetScreenWidth() - SCREEN_WIDTH * screenScale) / 2;
        float screenOffsetY = (GetScreenHeight() - SCREEN_HEIGHT * screenScale) / 2;
        mouse = {(mouse.x - screenOffsetX) / screenScale, (mouse.y - screenOffsetY) / screenScale};

        if(health <= 0){
            health = 0;
        }

        if (onLevelBreak){
            levelBreakTimer += deltaTime;
            if (levelBreakTimer > 10){
                levelBreakTimer = 0;
                onLevelBreak = false;
            }
        }

        else{
            levelProgressTimer += deltaTime;
        }

        if (levelProgressTimer >= levelIntervalCurrent){
            level += 1;
            levelUpdateStats(level, levelIntervalCurrent, trooperSpawnInterval, hulksSpawn, hulkSpawnInterval);
            onLevelBreak = true;
            levelProgressTimer = 0;
        }

        if(!onLevelBreak){
            trooperSpawnTimer += deltaTime;
            hulkSpawnTimer += deltaTime;
            if (trooperSpawnTimer >= trooperSpawnInterval){
                trooperSpawnTimer = 0;
                enemies.push_back(enemy(trooperWalk, trooperAttack, trooperDie, 90, 20, 100, 10, 4, 4, 0.24, 7));
            }

            if (hulkSpawnTimer >= hulkSpawnInterval and hulksSpawn){
                hulkSpawnTimer = 0;
                enemies.push_back(enemy(hulkWalk, hulkAttack, hulkDie, 50, 70, 600, 30, 8, 10, 1.2, 5));
            }
        }

        for (int i = 0; i < enemies.size(); i ++){
            enemies[i].update(deltaTime, health);
        }

        for(int i = 0; i < turrets.size(); i++){
            turrets[i].update(deltaTime, enemies);
        }

        for(int i = 0; i < deadEnemies.size(); i++){
            deadEnemies[i].update(deltaTime);
        }

        for(int i = 0; i < deadEnemies.size(); i++){
            if(deadEnemies[i].finished){
                deadEnemies.erase(deadEnemies.begin() + i);
                i--;
            }
        }

        for(int i = 0; i < enemies.size(); i++){
            if(enemies[i].health <= 0){
                superCredits += enemies[i].coinDrop;
                deadEnemies.push_back(deadEnemy(enemies[i].dieSheet, enemies[i].position, enemies[i].rotation, enemies[i].dieFrameCount));
                enemies.erase(enemies.begin() + i);
            }
        }

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            for(int i = 0; i < buildSpots.size(); i++){
                Rectangle button = {buildSpots[i].box.x + 55, buildSpots[i].box.y, 170, 60};
                if(CheckCollisionPointRec(mouse, buildSpots[i].box) and !buildSpots[i].occupied){
                    buildSpots[i].panelOpen = !buildSpots[i].panelOpen;
                }

                if(buildSpots[i].panelOpen and CheckCollisionPointRec(mouse,button)){
                    if(superCredits >= 100){
                        superCredits -= 100;
                        turrets.push_back(turret("Gatling Sentry", gatlingSentryFire, {buildSpots[i].box.x + 24, buildSpots[i].box.y + 24}, 10, 250, 0.08f, 100));
                        buildSpots[i].occupied = true;
                        buildSpots[i].panelOpen = false;
                    }
                }
            }
        }

        BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        DrawTexture(gameMap, 0, 0, WHITE);

        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].draw();
        }

        for(int i = 0; i < turrets.size(); i++){
            turrets[i].draw();
        }

        for(int i = 0; i < deadEnemies.size(); i++){
            deadEnemies[i].draw();
        }

        DrawText(TextFormat("Super Credits: %i", superCredits), 20, 20, 30, YELLOW);
        DrawText(TextFormat("Level: %i", level), 20, 55, 30, {65, 99, 156, 255});
        DrawText(TextFormat("Time to Next Level: %0.1f", levelIntervalCurrent - levelProgressTimer), 20, 90, 30, ORANGE);

        float healthPercent = (float)health / maxHealth;
        DrawRectangle(1474, 139, 100, 20, RED);
        DrawRectangle(1474, 139, 100 * healthPercent, 20, GREEN);
        DrawText(TextFormat("%i / %i", health, maxHealth), 1460, 165, 20, RED);

        if (onLevelBreak){
            DrawText(TextFormat("Level Won!", 10 - levelBreakTimer), 625, 370, 30, {38, 168, 34, 255});
            DrawText(TextFormat("On Break For %0.1f", 10 - levelBreakTimer), 450, 410, 60, {191, 41, 21, 255});
        }

        for(int i = 0; i < buildSpots.size(); i++){
            if(!buildSpots[i].occupied){
                DrawTexture(emptySlotSprite, buildSpots[i].box.x, buildSpots[i].box.y, WHITE);
            }

            if(buildSpots[i].panelOpen){
                Rectangle button = {buildSpots[i].box.x + 55, buildSpots[i].box.y, 170, 60};
                if (superCredits >= 100){
                    DrawRectangleRec(button, DARKGRAY);
                }
                else{
                    DrawRectangleRec(button, {43, 43, 43, 255});
                }
                DrawText("GATLING SENTRY", button.x + 5, button.y + 5, 18, WHITE);
                DrawText( "100 SC", button.x + 5, button.y + 30, 18, YELLOW);
            }


        }

        if(health <= 0){
            DrawRectangle(0, 0, 1600, 900, BLACK);
            DrawText("DEMOCRACY HAS FALLEN", 420, 350, 60, RED);
            DrawText("GAME OVER", 610, 430, 50, WHITE);
        }

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture, {0, 0, (float)SCREEN_WIDTH, -(float)SCREEN_HEIGHT}, {screenOffsetX, screenOffsetY, SCREEN_WIDTH * screenScale, SCREEN_HEIGHT * screenScale}, {0, 0}, 0, WHITE);
        EndDrawing();
    }

    UnloadTexture(gameMap);
    UnloadTexture(trooperWalk);
    UnloadTexture(trooperAttack);
    UnloadTexture(gatlingSentryFire);
    UnloadTexture(emptySlotSprite);
    UnloadRenderTexture(target);

    CloseWindow();
}
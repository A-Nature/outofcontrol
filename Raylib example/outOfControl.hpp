#include "raylib.h"
#include <vector>
#include <string>
#include <cmath>

using namespace std;

vector<Vector2> enemyPath = {
    {-6.0f, 447.0f},
    {464.0f, 447.0f},
    {464.0f, 200.0f},
    {650.0f, 200.0f},
    {650.0f, 426.0f},
    {906.0f, 426.0f},
    {906.0f, 703.0f},
    {1330.0f, 703.0f},
    {1330.0f, 242.0f},
    {1400.0f, 242.0f},
};

class enemy{
    public:
        Texture2D walkSheet;
        Texture2D attackSheet;
        Texture2D dieSheet;
        Texture2D enemyIcon;
        int damage;
        float speed;
        int health;
        int maxHealth;
        int coinDrop;
        Vector2 position;
        int pathIndex;
        bool attacking;
        int dieFrameCount;
        float attackTimer;
        float attackInterval;
        float rotation;
        int currentFrame;
        float frameTimer;
        float frameSpeed;
        int walkFrameCount;
        int attackFrameCount;
        float frameWidth;
        float frameHeight;

        enemy(Texture2D walkSheetTemp, Texture2D attackSheetTemp, Texture2D dieSheetTemp, int speedTemp, int damageTemp, int healthTemp, int coinDropTemp, int walkFramesTemp, int attackFramesTemp, float attackIntervalTemp, int dieFramesTemp){
            walkSheet = walkSheetTemp;
            attackSheet = attackSheetTemp;
            dieSheet = dieSheetTemp;
            position = enemyPath[0];
            pathIndex = 1;
            attacking = false;
            rotation = 0;
            currentFrame = 0;
            frameTimer = 0.0f;
            frameSpeed = 0.12f;
            walkFrameCount = walkFramesTemp;
            attackFrameCount = attackFramesTemp;
            frameWidth = walkSheet.width / walkFrameCount;
            frameHeight = walkSheet.height;
            speed = speedTemp;
            damage = damageTemp;
            health = healthTemp;
            maxHealth = healthTemp;
            coinDrop = coinDropTemp;
            attackTimer = 0.0f;
            attackInterval = attackIntervalTemp;
            dieFrameCount = dieFramesTemp;
        }

        void update(float deltaTime, int &baseHealth) {
            int frames;
            if (attacking){
                frames = attackFrameCount;
            } 
            else{
                frames = walkFrameCount;
            }

            frameTimer += deltaTime;

            if (frameTimer >= frameSpeed){
                frameTimer = 0;
                currentFrame = (currentFrame + 1) % frames;
            }

            if (attacking){
                attackTimer += deltaTime;

                if(attackTimer >= attackInterval){
                    attackTimer = 0;
                    baseHealth -= damage;
                }

                return;
            }

            Vector2 target = enemyPath[pathIndex];
            float deltaX = target.x - position.x;
            float deltaY = target.y - position.y;
            float distance = sqrtf(deltaX * deltaX + deltaY * deltaY);
            rotation = atan2f(deltaY, deltaX) * RAD2DEG;

            if (distance <= speed * deltaTime){
                position = target;
                pathIndex++;
                if (pathIndex >= enemyPath.size()){
                    attacking = true;
                    currentFrame = 0;
                    frameTimer = 0;
                }
            } 
            
            else{
                position.x += (deltaX / distance) * speed * deltaTime;
                position.y += (deltaY / distance) * speed * deltaTime;
            }
        }

        void draw(){
            Texture2D sheet;

            if (attacking){
                sheet = attackSheet;
            }

            else{
                sheet = walkSheet;
            }

            int frames;

            if (attacking){
                frames = attackFrameCount;
            }

            else{
                frames = walkFrameCount;
            }

            Rectangle sourceRectangle = {currentFrame * frameWidth, 0.0f, frameWidth, frameHeight};
            Rectangle destinationRectangle = {position.x, position.y, frameWidth, frameHeight};
            Vector2 origin = {frameWidth / 2.0f, frameHeight / 2.0f};
            DrawTexturePro(sheet, sourceRectangle, destinationRectangle, origin, rotation - 90, WHITE);

            float healthBarWidth = frameWidth * 0.8f;
            float healthBarX = position.x - healthBarWidth / 2;
            float healthBarY = position.y - frameHeight / 2 - 10;
            DrawRectangle(healthBarX, healthBarY, healthBarWidth, 4, RED);
            DrawRectangle(healthBarX, healthBarY, healthBarWidth * ((float)health / maxHealth), 4, GREEN);
        }
};

class deadEnemy{
    public:
        Vector2 position;
        Texture2D deathSheet;
        float rotation;
        int currentFrame;
        int deathFrameCount;
        float frameSpeed;
        float frameTimer;
        float frameWidth;
        float frameHeight;
        bool finished;

        deadEnemy(Texture2D deathSheetImage, Vector2 positionTemp, float rotationTemp, int frameCountTemp){
            deathSheet = deathSheetImage;
            position = positionTemp;
            rotation = rotationTemp;

            deathFrameCount = (frameCountTemp > 0) ? frameCountTemp : 1;

            currentFrame = 0;
            frameTimer = 0.0f;
            frameSpeed = 0.08f;

            frameWidth = (float)deathSheet.width / deathFrameCount;
            frameHeight = (float)deathSheet.height;

            finished = false;
        }

        void update(float deltaTime){
            if(finished){
                return;
            }

            frameTimer += deltaTime;

            if(frameTimer >= frameSpeed){
                frameTimer = 0;
                currentFrame++;
                if(currentFrame >= deathFrameCount){
                    finished = true;
                }
            }
        }

        void draw(){
            if(finished){
                return;
            }

            int frameIndex = currentFrame;
            if(frameIndex >= deathFrameCount){
                frameIndex = deathFrameCount - 1;
            }

            Rectangle sourceRectangle = {float(frameIndex) * frameWidth, 0.0f, frameWidth, frameHeight};
            Rectangle destinationRectangle = {position.x, position.y, frameWidth, frameHeight};
            Vector2 origin = {frameWidth / 2.0f, frameHeight / 2.0f};
            DrawTexturePro(deathSheet, sourceRectangle, destinationRectangle, origin, rotation - 90, WHITE);
        }
};

class buildSpot{
    public:
        Rectangle box;
        bool occupied;
        bool panelOpen;

        buildSpot(float x, float y){
            box = {x, y, 48, 48};
            occupied = false;
            panelOpen = false;
        }
};

class turret{
    public:
        string turretName;
        Texture2D turretIcon;
        Texture2D turretSpriteSheet;
        Vector2 position;

        int damage;
        float range;
        float fireRate;
        int cost;

        float fireTimer;
        float rotation;

        int currentFrame;
        int frameCount;

        float frameTimer;
        float frameSpeed;

        bool firing;

        turret(string nameTemp, Texture2D spriteSheetTemp, Vector2 positionTemp, int damageTemp, float rangeTemp, float fireRateTemp, int costTemp){
            turretName = nameTemp;
            turretSpriteSheet = spriteSheetTemp;
            position = positionTemp;
            damage = damageTemp;
            range = rangeTemp;
            fireRate = fireRateTemp;
            cost = costTemp;
            fireTimer = 0.0f;
            rotation = 0.0f;
            currentFrame = 0;
            frameCount = 4;
            frameTimer = 0.0f;
            frameSpeed = 0.08f;
            firing = false;
        }

        void update(float deltaTime, vector<enemy>& enemies){
            enemy* target = nullptr;
            float nearestDistance = range;

            for(int i = 0; i < enemies.size(); i++){
                if(enemies[i].health <= 0){
                    continue;
                }

                float distanceX = enemies[i].position.x - position.x;
                float distanceY = enemies[i].position.y - position.y;
                float distance = sqrtf(distanceX * distanceX + distanceY * distanceY);

                if(distance < nearestDistance){
                    nearestDistance = distance;
                    target = &enemies[i];
                }
            }

            if(target != nullptr){
                float distanceX = target->position.x - position.x;
                float distanceY = target->position.y - position.y;

                rotation = atan2f(distanceY, distanceX) * RAD2DEG;
                fireTimer += deltaTime;

                if(fireTimer >= fireRate){
                    fireTimer = 0;
                    target->health -= damage;
                }

                firing = true;
            } else {
                firing = false;
            }

            if(firing){
                frameTimer += deltaTime;
                if(frameTimer >= frameSpeed){
                    frameTimer = 0;
                    currentFrame++;
                    if(currentFrame >= frameCount){
                        currentFrame = 0;
                    }
                }
            } else {
                frameTimer = 0;
                currentFrame = 0;
            }
        }

        void draw(){
            Rectangle sourceRectangle = {(float)currentFrame * 32, 0, 32, 32};
            Rectangle destinationRectangle = {position.x, position.y, 48, 48};
            Vector2 origin = {24, 24};

            DrawTexturePro(turretSpriteSheet, sourceRectangle, destinationRectangle, origin, rotation - 90, WHITE);
        }
};

void levelUpdateStats(int currentLevel, float &nextLevelInterval, float &trooperSpawmIntervalTemp, bool &hulksShouldSpawn, float &hulkSpawnIntervalTemp){
    if (currentLevel == 2){
        trooperSpawmIntervalTemp = 2;
        nextLevelInterval = 15;
    }

    if (currentLevel == 3){
        trooperSpawmIntervalTemp = 1.5;
        hulksShouldSpawn = true;
        hulkSpawnIntervalTemp = 7.3;
        nextLevelInterval = 20;
    }

    if (currentLevel == 4){
        trooperSpawmIntervalTemp = 0.8;
        hulkSpawnIntervalTemp = 6.4;
        nextLevelInterval = 20;
    }

    if (currentLevel == 5){
        trooperSpawmIntervalTemp = 0.6;
        hulkSpawnIntervalTemp = 5;
        nextLevelInterval = 20;
    }

    if (currentLevel == 6){
        trooperSpawmIntervalTemp = 0.4;
        hulkSpawnIntervalTemp = 3;
        nextLevelInterval = 15;
    }

    if (currentLevel == 7){
        trooperSpawmIntervalTemp = 0.4;
        hulkSpawnIntervalTemp = 2.5;
        nextLevelInterval = 20;
    }
}
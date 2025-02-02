#include "iostream"
#include "cmath"
#include "string"
#include "random"
#include "algorithm"
#include "raylib.h"
#include "raymath.h"

std::string pathToImages = "./assets/images/";
std::string pathToSounds = "./assets/sounds/";
int imageSize = 75;
int tileRowCount = 4;
float animationSpeed = 8.5f;


class Tile {
public:
    int posIndex = 0;
    int index = 0;
    int size = 0;
    Texture2D texture;
    bool inProgress = false;

    Vector2 position{ 0, 0 };
    Vector2 realPosition{0, 0};

    int emptySpacePosIndex = 0;


    Tile() {

    }

    Tile(int indexProp, int posIndex, int sizeProp) {
        this->index = indexProp;
        this->posIndex = posIndex;
        this->size = sizeProp;
        if (this->index != 0) {
            std::string texturePath = pathToImages + "puzzle-" + std::to_string(index) + ".jpg";
            this->texture = LoadTexture(texturePath.c_str());
        }
    }

    void Unload() {
        UnloadTexture(this->texture);
    }

    void Draw() {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        this->realPosition = this->position;
        this->realPosition.x += (screenWidth / 2) - (2 * this->size);
        this->realPosition.y += (screenHeight / 2) - (2 * this->size);
        DrawTextureEx(this->texture, this->realPosition, 0.0f, (this->size / (float)imageSize), WHITE);
    }

    void Update() {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        if (this->inProgress) {
            float deltaTime = GetFrameTime();

            Vector2 emptySpacePos = this->CalculatePosIndex(this->emptySpacePosIndex);

            this->position.x += (emptySpacePos.x - this->position.x) * deltaTime * animationSpeed;
            this->position.y += (emptySpacePos.y - this->position.y) * deltaTime * animationSpeed;

            if (Vector2Distance(this->position, emptySpacePos) < 0.2) {
                this->inProgress = false;
                this->posIndex = this->emptySpacePosIndex;
            }
        }
        else {
            this->SetPosIndex(this->posIndex);
        }
        
        this->Draw();
    }

    Vector2 CalculatePosIndex(int index) {
        float positionX = (((index) % tileRowCount)) * this->size;
        float positionY = ((int)((index) / tileRowCount)) * this->size;

        return Vector2{ positionX, positionY };
    }

    void SetPosIndex(int index) {
        this->position = this->CalculatePosIndex(index);
    }
};

class Tiles {
public:
    Tile tiles[16];
    int tileSize = 0;
    int animTileIndex = -1;
    Sound stoneSlidingSound;

    Tiles() {

    }

    Tiles(int tileSizeProp) {
        std::string soundPath = pathToSounds + "stone-sliding.mp3";
        this->stoneSlidingSound = LoadSound(soundPath.c_str());

        const int sizeOfTiles = (sizeof(this->tiles) / sizeof(Tile));
        
        //Shuffling cards
        int randNumbers[] = { 0, 4, 8, 7, 3, 13, 5, 14, 11, 10, 2, 12, 1, 15, 6, 9 };

        //Generating tiles
        this->tileSize = tileSizeProp;
        for (int i = 0; i < sizeOfTiles; i++) {
            int randPosIndex = randNumbers[i];
            this->tiles[i] = Tile(i, randPosIndex, this->tileSize);
        }
    }

    void Unload() {
        for (int i = 0; i < (sizeof(this->tiles) / sizeof(Tile)); i++) {
            this->tiles[i].Unload();
        }

        UnloadSound(this->stoneSlidingSound);
    }

    void Update(){
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        for (int i = 0; i < (sizeof(this->tiles) / sizeof(Tile)); i++) {
            this->tiles[i].Update();

            if (this->tiles[i].index != 0) {
                Vector2 touchPos = GetTouchPosition(0);
                Vector2 mousePos = GetMousePosition();
                Vector2 inputPos = (touchPos.x > 0.0f) ? touchPos : mousePos;

                Rectangle textureRect = Rectangle{ this->tiles[i].realPosition.x, this->tiles[i].realPosition.y, (float)this->tiles[i].size, (float)this->tiles[i].size };

                if (this->animTileIndex != -1 && this->tiles[this->animTileIndex].inProgress == false) {
                    this->animTileIndex = -1;
                }else if (this->animTileIndex == -1 && CheckCollisionPointRec(inputPos, textureRect) && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsGestureDetected(GESTURE_TAP))) {
                    float distance = Vector2Distance(this->tiles[i].realPosition, this->tiles[0].realPosition);
                    if (distance <= this->tiles[i].size) {
                        this->animTileIndex = i;
                        this->tiles[i].inProgress = true;
                        this->tiles[i].emptySpacePosIndex = this->tiles[0].posIndex;

                        this->tiles[0].realPosition = this->tiles[i].realPosition;
                        this->tiles[0].posIndex = this->tiles[i].posIndex;

                        PlaySound(this->stoneSlidingSound);
                    }
                }
            }
        }
    }

    void ChangeSize(int size) {
        this->tileSize = size;
        for (int i = 0; i < (sizeof(this->tiles) / sizeof(Tile)); i++) {
            this->tiles[i].size = this->tileSize;
        }
    };

    bool CheckFinished() {
        for (int i = 0; i < (sizeof(this->tiles) / sizeof(Tile)); i++) {
            if (this->tiles[i].index != this->tiles[i].posIndex) {
                return false;
            }
        }

        return true;
    }
};

class Puzzle {
public:
    Tiles tilesClass;
    int screenWidth = 0;
    int screenHeight = 0;
    Sound backgroundSound;

    Puzzle() {
        int tileSize = 0;

        this->tilesClass = Tiles(tileSize);

        std::string pathToBackgroundSound = pathToSounds + "background.mp3";
        this->backgroundSound = LoadSound(pathToBackgroundSound.c_str());
    }

    ~Puzzle() {
        this->tilesClass.Unload();
        UnloadSound(this->backgroundSound);
    }

    void Update() {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        float puzzleWidth = (float)std::min(std::min(screenWidth, screenHeight), 900);

        float tileSize = puzzleWidth / tileRowCount;
        
        this->tilesClass.ChangeSize(tileSize);

        this->tilesClass.Update();

        if (!IsSoundPlaying(this->backgroundSound)) {
            PlaySound(this->backgroundSound);
        }
    }
};

int main() {

    int windowWidth = 800;
    int windowHeight = 800;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitAudioDevice();
    InitWindow(windowWidth, windowHeight, "Puzzle");
    SetTargetFPS(60);

    Puzzle puzzle = Puzzle();

    while ((!WindowShouldClose()) && !puzzle.tilesClass.CheckFinished()) {
        BeginDrawing();
        ClearBackground(BLACK);

        puzzle.Update();

        EndDrawing();
    }

    CloseAudioDevice();

    CloseWindow();

    return 0;
}
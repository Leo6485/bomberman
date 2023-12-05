#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define TILESIZE 40
#define NUMTILES_H 15
#define NUMTILES_W 15

#define MAINMENU 0
#define GAMESCREEN 1
#define GAMEOVERSCREEN 9


//------------------------------------------------------------------------------------
// Variáveis globais
//------------------------------------------------------------------------------------
const short int sceneWidth = TILESIZE * NUMTILES_W;
const short int sceneHeight = TILESIZE * NUMTILES_H;

const Vector2 scene = {(SCREEN_W - sceneWidth) / 2, (SCREEN_H - sceneHeight) / 2};

int currentScreen = MAINMENU;
int currentMap = 0;
int gameOverScreenTime = 0;

int lastKeyP1 = KEY_NULL;
int lastKeyP2 = KEY_NULL;


typedef struct Bomb{
    Rectangle pos;
    Rectangle explosion_right;
    Rectangle explosion_left;
    Rectangle explosion_up;
    Rectangle explosion_down;
    float indexTop;
    float indexRight;
    float indexBottom;
    float indexLeft;
    int isActive;
    int collisionActive;
    int time;
} Bomb;

typedef struct {
    Vector2 pos;
    float size;
    int life;
    float velX;
    float velY;
    float acc;
    float fric;
    int put_bomb;
    int num_bombs;
    int bomb_distance;
    float bomb_vel;
    Bomb bombs[10];
    char name[16];
} PlayerObj;

typedef struct {
    int x;
    int y;
} Vector2Int;

typedef struct {
    PlayerObj player;
    PlayerObj player2;
} game;



//------------------------------------------------------------------------------------
// Funções relacionadas ao mapa
//------------------------------------------------------------------------------------
void initRandomMap(short int mapa[][NUMTILES_W]);
void drawMap(short int mapa[][NUMTILES_W]);
Vector2 getCoords(int x, int y);
Vector2Int getIndex(float x, float y);

//------------------------------------------------------------------------------------
// Funções relacionadas ao jogador
//------------------------------------------------------------------------------------
void initPlayer(PlayerObj *player, Vector2 pos, char name[]);
void getLastKey(int key[], int* lastKey);
void updateMove(PlayerObj *player, int key[], int lastKeyP1);
void prevCollision(PlayerObj *player, short int mapa[][NUMTILES_W]);

//------------------------------------------------------------------------------------
// Funções do jogo
//------------------------------------------------------------------------------------
void updateGame(game *g, short int mapa[][NUMTILES_W]);
void gameOver();
void mainMenu(game *g);
void editText(char* text, Rectangle position);
void updateMainMenu(game *g);
void drawMainMenu();
void debug(game *g);

//------------------------------------------------------------------------------------
// Funções da bomba
//------------------------------------------------------------------------------------
void initBombs(PlayerObj *player, int n);
void draw_bomb(PlayerObj *player);
void update_bomb(game *g, PlayerObj *player, short int mapa[][NUMTILES_W], int key);
void bombDamage(game *g);
void bombCollision(PlayerObj *player, short int mapa[][NUMTILES_W]);
void bombToBombCollision(game *g);



//Converte índices para coordenadas
Vector2 getCoords(int x, int y) {
    float coordX = x * TILESIZE + scene.x;
    float coordY = y * TILESIZE + scene.y;
    Vector2 coords = {coordX, coordY};
    return coords;
}

//Converte coordenadas para índices
Vector2Int getIndex(float x, float y) {
    int indexX = ((x + TILESIZE / 2) - scene.x) / TILESIZE;
    int indexY = ((y + TILESIZE / 2) - scene.y) / TILESIZE;
    Vector2Int index = {indexX, indexY};
    return index;
}


void initPlayer(PlayerObj *player, Vector2 pos, char name[]){
    player->pos = pos;
    player->size = TILESIZE;
    player->life = 1; 
    player->velX = 0.0;
    player->velY = 0.0;
    player->acc = 2.5;
    player->fric = 0.5;
    player->put_bomb = 0;
    player->num_bombs = 5;
    player->bomb_distance = 2;
    player->bomb_vel = 0.25;
    //player->bombs = {};
    strcpy(player->name, name);
}



int main() {
    game g;

    initPlayer(&g.player, (Vector2){scene.x, scene.y}, "PLAYER 1\0");
    initPlayer(&g.player2, (Vector2){scene.x + (NUMTILES_W - 1) * TILESIZE, scene.y + (NUMTILES_H - 1) * TILESIZE}, "PLAYER 2\0");

    initBombs(&g.player, 10);
    initBombs(&g.player2, 10);

    short int mapa[2][NUMTILES_H][NUMTILES_W] = {
                                          {
                                          {0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {2, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 2},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 2, 0, 0},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2},
                                          {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
                                          {0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0},
                                         },
                                          {
                                          {0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 2, 0, 0, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                          {0, 0, 2, 0, 0, 0, 2, 1, 0, 0, 2, 0, 2, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 2, 0, 2},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0},
                                          {1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
                                          {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                          {2, 0, 2, 0, 2, 0, 2, 1, 0, 0, 2, 0, 0, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                          {2, 0, 0, 0, 2, 0, 0, 1, 2, 0, 0, 0, 0, 0, 2},
                                          {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                          {0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 2, 0, 0, 0, 0},
                                         }
                                        };

    InitWindow(SCREEN_W, SCREEN_H, "Projeto");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if(currentScreen == MAINMENU){
            mainMenu(&g);

        } else if(currentScreen == GAMEOVERSCREEN) {
            gameOver(&g);

        } else if(currentScreen == GAMESCREEN) {
            updateGame(&g, mapa[currentMap]);

        }
    }

    CloseWindow();

    return 0;
}

//Desenha o mapa, inteirando sobre a matriz
void drawMap(short int mapa[][NUMTILES_W]) {
    for (int i = 0; i < NUMTILES_H; i++) {
        for (int j = 0; j < NUMTILES_W; j++) {
            if (mapa[i][j] == 1) {
                short int tileX = scene.x + j * TILESIZE;
                short int tileY = scene.y + i * TILESIZE;
                DrawRectangle(tileX, tileY, TILESIZE, TILESIZE, DARKGRAY);
            } else if (mapa[i][j] == 2) {
                short int tileX = scene.x + j * TILESIZE;
                short int tileY = scene.y + i * TILESIZE;
                DrawRectangle(tileX, tileY, TILESIZE, TILESIZE, WHITE);
            }
        }
    }
}

//Atualiza o game
void updateGame(game *g, short int mapa[][NUMTILES_W]) {

    if(!g->player.life || !g->player2.life) {
        currentScreen = GAMEOVERSCREEN;
        gameOverScreenTime = GetTime();
    }

    getLastKey((int []){KEY_W, KEY_S, KEY_A, KEY_D}, &lastKeyP1);
    getLastKey((int []){KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT}, &lastKeyP2);

    updateMove(&g->player2, (int []){KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT}, lastKeyP2);
    updateMove(&g->player, (int []){KEY_W, KEY_S, KEY_A, KEY_D}, lastKeyP1);


    ClearBackground(BLACK);
    BeginDrawing();

    DrawRectangle(scene.x, scene.y, sceneWidth, sceneHeight, GREEN);


    drawMap(mapa);



    //Desenha uma grade na cena
    for (int x = 0; x <= sceneWidth; x += TILESIZE) {
        DrawLine(scene.x + x, scene.y, scene.x + x, SCREEN_H / 2 + sceneHeight / 2, DARKGRAY);
    }

    for (int y = 0; y <= sceneHeight; y += TILESIZE) {
        DrawLine(scene.x, scene.y + y, SCREEN_W / 2 + sceneWidth / 2, scene.y + y, DARKGRAY);
    }


    prevCollision(&g->player, mapa);
    prevCollision(&g->player2, mapa);

    update_bomb(g, &g->player, mapa, KEY_SPACE);
    update_bomb(g, &g->player2, mapa, KEY_ENTER);

    //Desenha o player
    DrawRectangle(g->player.pos.x, g->player.pos.y, g->player.size, g->player.size, BLUE);
    DrawText(g->player.name, 0, 0, 25, WHITE);

    
    DrawRectangle(g->player2.pos.x, g->player2.pos.y, g->player2.size, g->player2.size, YELLOW);
    DrawText(g->player2.name, 0, 50, 25, WHITE);

    EndDrawing();
}

//Move o player como uma física baseada em aceleração e atrito
void updateMove(PlayerObj *player, int key[], int lastKey) {
    if (lastKey == key[0] && player->pos.y > scene.y)
        player->velY -= player->acc;
    else if (player->pos.y <= scene.y)
        player->pos.y = scene.y;

    if (lastKey == key[1] && player->pos.y < sceneHeight + scene.y - player->size)
        player->velY += player->acc;
    else if (player->pos.y >= sceneHeight + scene.y - player->size)
        player->pos.y = sceneHeight + scene.y - player->size;

    if (lastKey == key[2] && player->pos.x >= scene.x)
        player->velX -= player->acc;
    else if (player->pos.x < scene.x)
        player->pos.x = 0 + scene.x;

    if (lastKey == key[3] && player->pos.x <= sceneWidth + scene.x - player->size)
        player->velX += player->acc;
    else if (player->pos.x >= sceneWidth + scene.x - player->size)
        player->pos.x = sceneWidth + scene.x - player->size;

    player->pos.x += player->velX;
    player->pos.y += player->velY;

    player->velX *= player->fric;
    player->velY *= player->fric;
}

//Inicia as bombas com os campos "isActive" e "collisionActive" igual a 0, para não pegar lixo de memória
void initBombs(PlayerObj *player, int n) {
    for(int i = 0; i < n; i++) {
        player->bombs[i].isActive = 0;
        player->bombs[i].collisionActive = 0;
    }
}

//Desenha as bombas ativas
void draw_bomb(PlayerObj *player){
    for(int i = 0; i < player->num_bombs; i++) { 
        if(player->bombs[i].isActive == 1) {
            DrawRectangleRec(player->bombs[i].pos, RED);
            DrawRectangleRec(player->bombs[i].explosion_right, RED);
            DrawRectangleRec(player->bombs[i].explosion_left, RED);
            DrawRectangleRec(player->bombs[i].explosion_up, RED);
            DrawRectangleRec(player->bombs[i].explosion_down, RED);
        }
    }
}

//Causa dano ao player caso seja atingido pela bomba
void bombDamage(game *g) {
    short int CollisionP1 = 0;
    short int CollisionP2 = 0;

    Rectangle playerRec = {g->player.pos.x, g->player.pos.y, g->player.size, g->player.size};
    Rectangle player2Rec = {g->player2.pos.x, g->player2.pos.y, g->player2.size, g->player2.size};

    for(int i = 0; i < g->player.num_bombs; i++) {
        if(g->player.bombs[i].isActive == 1) {
            
            if (fabs(g->player.bombs[i].time - GetTime()) > 3 && fabs(g->player.bombs[i].time - GetTime()) < 5) {

                CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_right, playerRec);
                CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_left, playerRec);
                CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_up, playerRec);
                CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_down, playerRec);

                CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_right, player2Rec);
                CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_left, player2Rec);
                CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_up, player2Rec);
                CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_down, player2Rec);
            }
        }
    }
    
    for(int i = 0; i < g->player2.num_bombs; i++) {
        if(g->player2.bombs[i].isActive == 1) {
            
            if (fabs(g->player2.bombs[i].time - GetTime()) > 3 && fabs(g->player2.bombs[i].time - GetTime()) < 5) {

                CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_right, playerRec);
                CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_left, playerRec);
                CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_up, playerRec);
                CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_down, playerRec);

                CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_right, player2Rec);
                CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_left, player2Rec);
                CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_up, player2Rec);
                CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_down, player2Rec);

            }
        }
    }

    if(CollisionP1) {
        g->player.life = 0;
    }
    if(CollisionP2) {
        g->player2.life = 0;
    }
}

//Ativa a colisão de entidades com a bomba, após o player sair de "dentro" dela
void bombCollision(PlayerObj *player, short int mapa[][NUMTILES_W]) {
    for(int i = 0; i < player->num_bombs; i++) { 
        if(player->bombs[i].isActive == 1) {
            if(player->bombs[i].collisionActive == 0) {

                //Se a colisão estiver desativada e o player ter saído de "dentro" da bomba, ative a colisão colocando um bloco "3" na matriz do mapa
                if(!CheckCollisionRecs(player->bombs[i].pos, (Rectangle){player->pos.x, player->pos.y, player->size, player->size})) {
                    Vector2Int bombIndex = getIndex(player->bombs[i].pos.x, player->bombs[i].pos.y);
                    mapa[bombIndex.y][bombIndex.x] = 3;
                }
            }
        }
    }
}


void bombToBombCollision(game *g) {



    for(int i = 0; i < g->player.num_bombs; i++) {
        for(int j = 0; j < g->player.num_bombs; j++) {
            if(g->player.bombs[i].isActive == 1) {
                
                if (fabs(g->player.bombs[i].time - GetTime()) > 3 && fabs(g->player.bombs[i].time - GetTime()) < 5) {

                    short int CollisionP1 = 0;
                    short int CollisionP2 = 0;

                    Rectangle bombP1 = g->player.bombs[j].pos;
                    Rectangle bombP2 = g->player2.bombs[j].pos;

                    CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_right, bombP1);
                    CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_left, bombP1);
                    CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_up, bombP1);
                    CollisionP1 += CheckCollisionRecs(g->player.bombs[i].explosion_down, bombP1);
                    
                    CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_right, bombP2);
                    CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_left, bombP2);
                    CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_up, bombP2);
                    CollisionP2 += CheckCollisionRecs(g->player.bombs[i].explosion_down, bombP2);


                    if(CollisionP1 && (GetTime() - g->player.bombs[j].time) < 2.8 ) {
                        g->player.bombs[j].time = GetTime() - 3;
                    }

                    if(CollisionP2 && (GetTime() - g->player2.bombs[j].time) < 2.8 ) {
                        g->player2.bombs[j].time = GetTime() - 3;
                    }
                }
            }
        }
    }

    for(int i = 0; i < g->player2.num_bombs; i++) {
        for(int j = 0; j < g->player2.num_bombs; j++) {
            if(g->player2.bombs[i].isActive == 1) {

                if (fabs(g->player2.bombs[i].time - GetTime()) > 3 && fabs(g->player2.bombs[i].time - GetTime()) < 5) {

                    short int CollisionP1 = 0;
                    short int CollisionP2 = 0;
                    
                    Rectangle bombP1 = g->player.bombs[j].pos;
                    Rectangle bombP2 = g->player2.bombs[j].pos;

                    CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_right, bombP1);
                    CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_left, bombP1);
                    CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_up, bombP1);
                    CollisionP1 += CheckCollisionRecs(g->player2.bombs[i].explosion_down, bombP1);
                    
                    CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_right, bombP2);
                    CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_left, bombP2);
                    CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_up, bombP2);
                    CollisionP2 += CheckCollisionRecs(g->player2.bombs[i].explosion_down, bombP2);


                    if(CollisionP1 && (GetTime() - g->player.bombs[j].time) < 2.8 ) {
                        g->player.bombs[j].time = GetTime() - 3;
                    }

                    if(CollisionP2 && (GetTime() - g->player2.bombs[j].time) < 2.8 ) {
                        g->player2.bombs[j].time = GetTime() - 3;
                    }
                }
            }
        }
    }

}


//Spawna, explode e desativa as bombas
void update_bomb(game *g, PlayerObj *player, short int mapa[][NUMTILES_W], int key) {
    
    draw_bomb(&g->player);
    draw_bomb(&g->player2);
    bombDamage(g);
    bombCollision(&g->player, mapa);
    bombCollision(&g->player2, mapa);
    bombToBombCollision(g);

    
    if (IsKeyPressed(key)) {
        player->put_bomb = 1;
    }

    //Adiciona uma bomba no mesmo índice do player
    if (player->put_bomb == 1) {
        for (int i = 0; i < player->num_bombs; i++) {
            if (player->bombs[i].isActive == 0) {
                player->bombs[i].isActive = 1;
                
                Vector2Int index = getIndex(player->pos.x, player->pos.y);
                Vector2 coords = getCoords(index.x, index.y);

                player->bombs[i].pos = (Rectangle){coords.x, coords.y, TILESIZE, TILESIZE};
                player->bombs[i].explosion_right = player->bombs[i].pos;
                player->bombs[i].explosion_left = player->bombs[i].pos;
                player->bombs[i].explosion_down = player->bombs[i].pos;
                player->bombs[i].explosion_up = player->bombs[i].pos;

                player->bombs[i].time = GetTime();

                Vector2Int bombIndex = getIndex(player->bombs[i].pos.x, player->bombs[i].pos.y);

                player->bombs[i].indexBottom = bombIndex.y;
                player->bombs[i].indexRight = bombIndex.x;
                player->bombs[i].indexTop = bombIndex.y;
                player->bombs[i].indexLeft = bombIndex.x;

                break;
            }
        }
    }

    for (int i = 0; i < player->num_bombs; i++) {
        if (player->bombs[i].isActive == 1) {
            if (fabs(player->bombs[i].time - GetTime()) > 3 && fabs(player->bombs[i].time - GetTime()) < 5) {
                int grow_tax = player->bomb_distance;

                Vector2Int bombIndex = getIndex(player->bombs[i].pos.x, player->bombs[i].pos.y);

                // Right
                if (player->bombs[i].explosion_right.width < (player->bomb_distance + 1) * TILESIZE) { // Analisa se a largura da bomba é menor que a distância máxima
                    float indexRight = player->bombs[i].indexRight;

                    int prevBlock = mapa[(int)bombIndex.y][(int)(indexRight)]; //Um bloco atrás da explosão

                     /*
                     Extremidade da explosão, exemplo (int)(4.99 + 1) será 5 ao converter para inteiro,
                     quero que pare no bloco 5, ou seja, só irá parar ao atingir o bloco (int)(5 + 1) ou 6
                     */
                    int currentBlock = mapa[(int)bombIndex.y][(int)(indexRight + 1)];

                    if (prevBlock != 2 && currentBlock != 1 && (indexRight + 1) < NUMTILES_W) {
                        player->bombs[i].indexRight += player->bomb_vel; // aumenta o índice
                        player->bombs[i].explosion_right.width += TILESIZE * player->bomb_vel; // aumenta o tamanho do retângulo da explosão
                    }
                }

                // Left
                if (player->bombs[i].explosion_left.width < (player->bomb_distance + 1) * TILESIZE) {
                    float indexLeft = player->bombs[i].indexLeft;

                    int prevBlock = mapa[(int)bombIndex.y][(int)(indexLeft - player->bomb_vel + 1)];
                    int currentBlock = mapa[(int)bombIndex.y][(int)(indexLeft - player->bomb_vel)];


                    if (prevBlock != 2 && currentBlock != 1 && indexLeft - player->bomb_vel >= 0) {
                        player->bombs[i].indexLeft -= player->bomb_vel;
                        player->bombs[i].explosion_left.width += TILESIZE * player->bomb_vel;
                        player->bombs[i].explosion_left.x -= TILESIZE * player->bomb_vel;
                    }
                }

                // Top
                if (player->bombs[i].explosion_up.height < (player->bomb_distance + 1) * TILESIZE) {
                    float indexTop = player->bombs[i].indexTop;

                    int prevBlock = mapa[(int)(indexTop - player->bomb_vel + 1)][(int)bombIndex.x];
                    int currentBlock = mapa[(int)(indexTop - player->bomb_vel)][(int)bombIndex.x];

                    if (prevBlock != 2 && currentBlock != 1 && indexTop - player->bomb_vel >= 0) {
                        player->bombs[i].indexTop -= player->bomb_vel;
                        player->bombs[i].explosion_up.height += TILESIZE * player->bomb_vel;
                        player->bombs[i].explosion_up.y -= TILESIZE * player->bomb_vel;
                    }
                }
                // Bottom
                if (player->bombs[i].explosion_down.height < (player->bomb_distance + 1) * TILESIZE) {
                    float indexBottom = player->bombs[i].indexBottom;

                    int prevBlock = mapa[(int)(indexBottom + player->bomb_vel - 0.001)][(int)bombIndex.x];
                    int currentBlock = mapa[(int)(indexBottom + 1)][(int)bombIndex.x];

                    if (prevBlock != 2 && currentBlock != 1 && (indexBottom + 1) < NUMTILES_H) {
                        player->bombs[i].indexBottom += player->bomb_vel;
                        player->bombs[i].explosion_down.height += TILESIZE * player->bomb_vel;
                    }
                }

            } else if(fabs(player->bombs[i].time - GetTime()) > 3){
                
                Vector2Int bombIndex = getIndex(player->bombs[i].pos.x, player->bombs[i].pos.y);

                mapa[(int)bombIndex.y][(int)(player->bombs[i].indexRight)] = 0; //Axtremidade da explosão para um bloco vazio na matriz, quebrando os blocos atingidos
                mapa[(int)bombIndex.y][(int)(player->bombs[i].indexLeft)] = 0;
                mapa[(int)(player->bombs[i].indexTop)][(int)bombIndex.x] = 0;
                mapa[(int)(player->bombs[i].indexBottom)][(int)bombIndex.x] = 0;


                mapa[bombIndex.y][bombIndex.x] = 0; //Atualiza a bomba na matriz para 0, desativando a colisão

                player->bombs[i].collisionActive = 0;
                player->bombs[i].isActive = 0;

	    }
        }
    }
    player->put_bomb = 0;
}

//Gera um mapa aleatório (Não utilizado)
void initRandomMap(short int mapa[][NUMTILES_W]) {
    for (int i = 0; i < NUMTILES_H; i++) {
        for (int j = 0; j < NUMTILES_W; j++) {
            if (i % 2 && j % 2) {
                mapa[i][j] = 1;
            } else {
                mapa[i][j] = 0;
            }
        }
    }
    for (int i = 0; i < NUMTILES_H; i += 2) {
        int x;
        for (int j = 0; j < NUMTILES_W; j += 2) {
            x = GetRandomValue(0, 1);
            if (x && (i > 1 || j > 1)) {
                mapa[i][j] = 2;
            }
        }
    }
}

//Gerencia as colisões do player
void prevCollision(PlayerObj *player, short int mapa[][NUMTILES_W]) {
    Vector2Int playerIndex = getIndex(player->pos.x, player->pos.y);

    for (int y = playerIndex.y - 1; y <= playerIndex.y + 1; y++) {
        for (int x = playerIndex.x - 1; x <= playerIndex.x + 1; x++) { //Obtém os blocos em torno do player
            if (mapa[y][x] >= 1 && x >= 0 && x < NUMTILES_W && y >= 0 && y < NUMTILES_H) { //Blocos maiores ou iguais a 1 colidem
                Vector2 coords = getCoords(x, y);

                //Demarca os blocos em torno do player
                //DrawCircle(coords.x + TILESIZE / 2, coords.y + TILESIZE / 2, 4, RED);
                //DrawRectangleLines(coords.x, coords.y, TILESIZE, TILESIZE, RED);

                int playerLeft = player->pos.x;
                int playerRight = player->pos.x + player->size;
                int playerTop = player->pos.y;
                int playerBottom = player->pos.y + player->size;

                int tileLeft = coords.x;
                int tileRight = coords.x + TILESIZE;
                int tileTop = coords.y;
                int tileBottom = coords.y + TILESIZE;

                Rectangle playerRect = {player->pos.x, player->pos.y, player->size, player->size};
                Rectangle tileRect = {coords.x, coords.y, TILESIZE, TILESIZE};

                if (CheckCollisionRecs(playerRect, tileRect)) {
                    int tolerance = TILESIZE - 10;
                    int move_on_collision = 2.8;

                    //Top
                    int top_dist = tileTop - playerTop;
                    if (top_dist >= tolerance) {
                        player->pos.y = tileTop - player->size; //Move o player para fora do bloco caso a colisão seja muito superficial (Menor ou igual a 10), pois foi de onde a colisão partiu
                    } else if (top_dist < tolerance && top_dist > 10) {
                        player->pos.y -= move_on_collision; //Move o player ligeiramente para direção oposta á colisão, pois não foi de onde a colisão partiu
                    }

                    //Left
                    int left_dist = tileLeft - playerLeft;
                    if (left_dist >= tolerance) {
                        player->pos.x = tileLeft - player->size;
                    } else if (left_dist < tolerance && left_dist > 10) {
                        player->pos.x -= move_on_collision;
                    }

                    //Bottom
                    int bottom_dist = playerBottom - tileBottom;
                    if (bottom_dist >= tolerance) {
                        player->pos.y = tileBottom;
                    } else if (bottom_dist < tolerance && bottom_dist > 10) {
                        player->pos.y += move_on_collision;
                    }

                    //Right
                    int right_dist = playerRight - tileRight;
                    if (right_dist >= tolerance) {
                        player->pos.x = tileRight;
                    } else if (right_dist < tolerance && right_dist > 10) {
                        player->pos.x += move_on_collision;
                    }
                }
            }
        }
    }
}

//Obtém a última tecla pressionada, para não andar em dois eixos ao mesmo tempo
void getLastKey(int key[], int *lastKey) {
    if (IsKeyPressed(key[0])) *lastKey = key[0];
    if (IsKeyPressed(key[2])) *lastKey = key[2];
    if (IsKeyPressed(key[1])) *lastKey = key[1];
    if (IsKeyPressed(key[3])) *lastKey = key[3];

    if (IsKeyReleased(key[0]) || IsKeyReleased(key[2]) || IsKeyReleased(key[1]) || IsKeyReleased(key[3])) {
        if (IsKeyDown(key[3]))
            *lastKey = key[3];
        else if (IsKeyDown(key[1]))
            *lastKey = key[1];
        else if (IsKeyDown(key[2]))
            *lastKey = key[2];
        else if (IsKeyDown(key[0]))
            *lastKey = key[0];
        else
            *lastKey = KEY_NULL;
    }
}


//Tela inicial teste (Não utilizado)
void mainMenu(game *g) {
    currentMap = 0;
    updateMainMenu(g);
}




void updateMainMenu(game *g) {
    Vector2 cursorPosition = GetMousePosition();
    Color color_start_btt = BLUE;


    //Play button
    if(CheckCollisionPointRec(cursorPosition, (Rectangle){SCREEN_W/2 - 125, SCREEN_H/2 + 50, 250, 100})) {
        color_start_btt = GRAY; //hover

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentScreen = GAMESCREEN; //on click
        }

    } else {
        color_start_btt = BLUE; //no hover
    }


    editText(g->player.name, (Rectangle){125, 200, 250, 50});
    editText(g->player2.name, (Rectangle){425, 200, 250, 50});

    drawMainMenu(color_start_btt, g);

}


void editText(char* text, Rectangle position) {
    Vector2 cursorPosition = GetMousePosition();
    Color color_edit_text_1 = BLUE;
    //Player name

    if(CheckCollisionPointRec(cursorPosition, position)) {
        int key = GetKeyPressed();
        color_edit_text_1 = GRAY;
        if (key) {
            if ((key >= 32) && (key <= 125) && (strlen(text) < 15)) {

                int len = strlen(text);
                text[len] = (char)key;
                text[len + 1] = '\0';

            } else if ((key == KEY_BACKSPACE) && (strlen(text) > 0)) {
                text[strlen(text) - 1] = '\0';

            }
        }
    } else {
        color_edit_text_1 = BLUE;
    }

    DrawRectangleRec(position, color_edit_text_1);
    DrawText(text, position.x + 25, position.y + 15, 25, WHITE);

}


void drawMainMenu(Color color_start_btt, game *g) {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("Bomb SUS", SCREEN_W/2 - 125, 50, 50, BLUE);

    DrawRectangle(SCREEN_W/2 - 125, SCREEN_H/2 + 50, 250, 100, color_start_btt);
    DrawText("Play", SCREEN_W/2 - 50, SCREEN_H/2 + 75, 50, WHITE);
    EndDrawing();
}





//Tela de Game Over
void gameOver(game *g) {
    BeginDrawing();
    ClearBackground(BLACK);


    if((g->player.life && g->player2.life) || !(g->player.life || g->player2.life)) {
        DrawText("EMPATE", SCREEN_W/2 - 75, 300, 25, WHITE);
    } else if(g->player.life) {
        char text[24];
        sprintf(text, "%s GANHOU!", g->player.name);
        DrawText(text, SCREEN_W/2 - 75, 300, 25, WHITE);
    } else if(g->player2.life) {
        char text[24];
        sprintf(text, "%s GANHOU!", g->player2.name);
        DrawText(text, SCREEN_W/2 - 75, 300, 25, WHITE);
    }


    if(fabs(GetTime() - gameOverScreenTime) > 2.5) {
        g->player.pos = (Vector2){scene.x, scene.y};
        g->player.life = 1;

        g->player2.pos = (Vector2){scene.x + (NUMTILES_W - 1) * TILESIZE, scene.y + (NUMTILES_H - 1) * TILESIZE};
        g->player2.life = 1;

        currentScreen = GAMESCREEN;

        if(currentMap == 1) {

            initPlayer(&g->player, (Vector2){scene.x, scene.y}, g->player.name);
            initPlayer(&g->player2, (Vector2){scene.x + (NUMTILES_W - 1) * TILESIZE, scene.y + (NUMTILES_H - 1) * TILESIZE}, g->player2.name);
            currentScreen = MAINMENU;
        }

        if(currentMap == 0) {
            currentMap = 1;
        }
    }
    EndDrawing();
}

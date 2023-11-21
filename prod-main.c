#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define TILESIZE 40
#define NUMTILES_H 15
#define NUMTILES_W 15


//------------------------------------------------------------------------------------
//Variáveis globais
//------------------------------------------------------------------------------------
const short int sceneWidth = TILESIZE * NUMTILES_W;
const short int sceneHeight = TILESIZE * NUMTILES_H;

const Vector2 scene = {(SCREEN_W - sceneWidth) / 2, (SCREEN_H - sceneHeight) / 2};

KeyboardKey lastKey = KEY_NULL;


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
} PlayerObj;

typedef struct {
    int x;
    int y;
} Vector2Int;

typedef struct {
    PlayerObj player;
} game;



//------------------------------------------------------------------------------------
// Funções relacionadas ao mapa
//------------------------------------------------------------------------------------
void initRandomMap();
void drawMap();
Vector2 getCoords(int x, int y);
Vector2Int getIndex(float x, float y);

//------------------------------------------------------------------------------------
// Funções relacionadas ao jogador
//------------------------------------------------------------------------------------
void getLastKey();
void updateMove(game *g);
void prevCollision(game *g, short int mapa[][NUMTILES_W]);

//------------------------------------------------------------------------------------
// Funções do jogo
//------------------------------------------------------------------------------------
void updateGame(game *g);
void gameOver();
void mainMenu();
void debug(game *g);

//------------------------------------------------------------------------------------
// Funções da bomba
//------------------------------------------------------------------------------------
void draw_bomb(game *g);
void update_bomb(game *g, short int mapa[][NUMTILES_W]);
void bombDamage(game *g);
void bombCollision(game *g);
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

short int mapa[NUMTILES_H][NUMTILES_W] = {{0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0},
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
                                          {0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 2},
                                         };





int main() {
    game g = {
            .player = {
            .pos = {scene.x, scene.y}, 
            .size = TILESIZE,
            .life = 1, 
            .velX = 0.0,
            .velY = 0.0,
            .acc = 2.5,
            .fric = 0.5,
            .put_bomb = 0,
            .num_bombs = 5,
            .bomb_distance = 2,
            .bomb_vel = 0.25,
            .bombs = {}
        }
    };

    //Cria um mapa aleatório
    //initRandomMap();

    InitWindow(SCREEN_W, SCREEN_H, "Projeto");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if(IsKeyDown(KEY_ENTER)){
            mainMenu();
        } else if(!g.player.life) {
            gameOver();
        } else {
            updateGame(&g);
        }
    }

    CloseWindow();

    return 0;
}

//Desenha o mapa, inteirando sobre a matriz
void drawMap() {
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
void updateGame(game *g) {
    updateMove(g);
    DrawRectangle(scene.x, scene.y, sceneWidth, sceneHeight, GREEN);

    BeginDrawing();

    ClearBackground(BLACK);

    drawMap();



    //Desenha uma grade na cena
    for (int x = 0; x <= sceneWidth; x += TILESIZE) {
        DrawLine(scene.x + x, scene.y, scene.x + x, SCREEN_H / 2 + sceneHeight / 2, DARKGRAY);
    }

    for (int y = 0; y <= sceneHeight; y += TILESIZE) {
        DrawLine(scene.x, scene.y + y, SCREEN_W / 2 + sceneWidth / 2, scene.y + y, DARKGRAY);
    }

    update_bomb(g, mapa);

    prevCollision(g, mapa);
    DrawRectangle(g->player.pos.x, g->player.pos.y, g->player.size, g->player.size, BLUE); //Desenha o player

    EndDrawing();
}

//Move o player como uma física baseada em aceleração e atrito
void updateMove(game *g) {
    getLastKey();
    if (lastKey == KEY_W && g->player.pos.y > scene.y)
        g->player.velY -= g->player.acc;
    else if (g->player.pos.y <= scene.y)
        g->player.pos.y = scene.y;

    if (lastKey == KEY_S && g->player.pos.y < sceneHeight + scene.y - g->player.size)
        g->player.velY += g->player.acc;
    else if (g->player.pos.y >= sceneHeight + scene.y - g->player.size)
        g->player.pos.y = sceneHeight + scene.y - g->player.size;

    if (lastKey == KEY_A && g->player.pos.x >= scene.x)
        g->player.velX -= g->player.acc;
    else if (g->player.pos.x < scene.x)
        g->player.pos.x = 0 + scene.x;

    if (lastKey == KEY_D && g->player.pos.x <= sceneWidth + scene.x - g->player.size)
        g->player.velX += g->player.acc;
    else if (g->player.pos.x >= sceneWidth + scene.x - g->player.size)
        g->player.pos.x = sceneWidth + scene.x - g->player.size;

    g->player.pos.x += g->player.velX;
    g->player.pos.y += g->player.velY;

    g->player.velX *= g->player.fric;
    g->player.velY *= g->player.fric;
}

//Inicia as bombas com os campos "isActive" e "collisionActive" igual a 0, para não pegar lixo de memória
void initBombs(game *g, int n) {
    for(int i = 0; i < n; i++) {
        g->player.bombs[i].isActive = 0;
        g->player.bombs[i].collisionActive = 0;
    }
}

//Desenha as bombas ativas
void draw_bomb(game *g){
    for(int i = 0; i < g->player.num_bombs; i++) { 
        if(g->player.bombs[i].isActive == 1) {
            DrawRectangleRec(g->player.bombs[i].pos, RED);
            DrawRectangleRec(g->player.bombs[i].explosion_right, RED);
            DrawRectangleRec(g->player.bombs[i].explosion_left, RED);
            DrawRectangleRec(g->player.bombs[i].explosion_up, RED);
            DrawRectangleRec(g->player.bombs[i].explosion_down, RED);
        }
    }
}

//Causa dano ao player caso seja atingido pela bomba
void bombDamage(game *g) {
    for(int i = 0; i < g->player.num_bombs; i++) { 
        if(g->player.bombs[i].isActive == 1) {
            
            if (fabs(g->player.bombs[i].time - GetTime()) > 3 && fabs(g->player.bombs[i].time - GetTime()) < 5) {
                short int Collision = 0;

                //Se colidir com algum retângulo de explosão, a variável Collision terá um valor positivo
                Collision += CheckCollisionRecs(g->player.bombs[i].explosion_right, (Rectangle){g->player.pos.x, g->player.pos.y, g->player.size, g->player.size});
                Collision += CheckCollisionRecs(g->player.bombs[i].explosion_left, (Rectangle){g->player.pos.x, g->player.pos.y, g->player.size, g->player.size});
                Collision += CheckCollisionRecs(g->player.bombs[i].explosion_up, (Rectangle){g->player.pos.x, g->player.pos.y, g->player.size, g->player.size});
                Collision += CheckCollisionRecs(g->player.bombs[i].explosion_down, (Rectangle){g->player.pos.x, g->player.pos.y, g->player.size, g->player.size});

                if(Collision) {
                    g->player.life -= 1;
                }
            }
        }
    }
}

//Ativa a colisão de entidades com a bomba, após o player sair de "dentro" dela
void bombCollision(game *g) {
    for(int i = 0; i < g->player.num_bombs; i++) { 
        if(g->player.bombs[i].isActive == 1) {
            if(g->player.bombs[i].collisionActive == 0) {

                //Se a colisão estiver desativada e o player ter saído de "dentro" da bomba, ative a colisão colocando um bloco "3" na matriz do mapa
                if(!CheckCollisionRecs(g->player.bombs[i].pos, (Rectangle){g->player.pos.x, g->player.pos.y, g->player.size, g->player.size})) {
                    Vector2Int bombIndex = getIndex(g->player.bombs[i].pos.x, g->player.bombs[i].pos.y);
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
                    short int Collision = 0;

                    //Se colidir com algum retângulo de explosão, a variável Collision terá um valor positivo
                    Collision += CheckCollisionRecs(g->player.bombs[i].explosion_right, g->player.bombs[j].pos);
                    Collision += CheckCollisionRecs(g->player.bombs[i].explosion_left, g->player.bombs[j].pos);
                    Collision += CheckCollisionRecs(g->player.bombs[i].explosion_up, g->player.bombs[j].pos);
                    Collision += CheckCollisionRecs(g->player.bombs[i].explosion_down, g->player.bombs[j].pos);

                    if(Collision && (GetTime() - g->player.bombs[j].time) < 2.8 ) {
                        g->player.bombs[j].time = GetTime() - 3;
                    }
                }
            }
        }
    }
}


//Spawna, explode e desativa as bombas
void update_bomb(game *g, short int mapa[][NUMTILES_W]) {
    
    draw_bomb(g);
    bombDamage(g);
    bombCollision(g);
    bombToBombCollision(g);

    
    if (IsKeyPressed(KEY_SPACE)) {
        g->player.put_bomb = 1;
    }

    //Adiciona uma bomba no mesmo índice do player
    if (g->player.put_bomb == 1) {
        for (int i = 0; i < g->player.num_bombs; i++) {
            if (g->player.bombs[i].isActive == 0) {
                g->player.bombs[i].isActive = 1;
                
                Vector2Int index = getIndex(g->player.pos.x, g->player.pos.y);
                Vector2 coords = getCoords(index.x, index.y);

                g->player.bombs[i].pos = (Rectangle){coords.x, coords.y, TILESIZE, TILESIZE};
                g->player.bombs[i].explosion_right = g->player.bombs[i].pos;
                g->player.bombs[i].explosion_left = g->player.bombs[i].pos;
                g->player.bombs[i].explosion_down = g->player.bombs[i].pos;
                g->player.bombs[i].explosion_up = g->player.bombs[i].pos;

                g->player.bombs[i].time = GetTime();

                Vector2Int bombIndex = getIndex(g->player.bombs[i].pos.x, g->player.bombs[i].pos.y);

                g->player.bombs[i].indexBottom = bombIndex.y;
                g->player.bombs[i].indexRight = bombIndex.x;
                g->player.bombs[i].indexTop = bombIndex.y;
                g->player.bombs[i].indexLeft = bombIndex.x;

                break;
            }
        }
    }

    for (int i = 0; i < g->player.num_bombs; i++) {
        if (g->player.bombs[i].isActive == 1) {
            if (fabs(g->player.bombs[i].time - GetTime()) > 3 && fabs(g->player.bombs[i].time - GetTime()) < 5) {
                int grow_tax = g->player.bomb_distance;

                Vector2Int bombIndex = getIndex(g->player.bombs[i].pos.x, g->player.bombs[i].pos.y);

                // Right
                if (g->player.bombs[i].explosion_right.width < (g->player.bomb_distance + 1) * TILESIZE) { // Analisa se a largura da bomba é menor que a distância máxima
                    float indexRight = g->player.bombs[i].indexRight;

                    int prevBlock = mapa[(int)bombIndex.y][(int)(indexRight)]; //Um bloco atrás da explosão

                     /*
                     Extremidade da explosão, exemplo (int)(4.99 + 1) será 5 ao converter para inteiro,
                     quero que pare no bloco 5, ou seja, só irá parar ao atingir o bloco (int)(5 + 1) ou 6
                     */
                    int currentBlock = mapa[(int)bombIndex.y][(int)(indexRight + 1)];

                    if (prevBlock != 2 && currentBlock != 1 && (indexRight + 1) < NUMTILES_W) {
                        g->player.bombs[i].indexRight += g->player.bomb_vel; // aumenta o índice
                        g->player.bombs[i].explosion_right.width += TILESIZE * g->player.bomb_vel; // aumenta o tamanho do retângulo da explosão
                    }
                }

                // Left
                if (g->player.bombs[i].explosion_left.width < (g->player.bomb_distance + 1) * TILESIZE) {
                    float indexLeft = g->player.bombs[i].indexLeft;

                    int prevBlock = mapa[(int)bombIndex.y][(int)(indexLeft - g->player.bomb_vel + 1)];
                    int currentBlock = mapa[(int)bombIndex.y][(int)(indexLeft - g->player.bomb_vel)];


                    if (prevBlock != 2 && currentBlock != 1 && indexLeft - g->player.bomb_vel >= 0) {
                        g->player.bombs[i].indexLeft -= g->player.bomb_vel;
                        g->player.bombs[i].explosion_left.width += TILESIZE * g->player.bomb_vel;
                        g->player.bombs[i].explosion_left.x -= TILESIZE * g->player.bomb_vel;
                    }
                }

                // Top
                if (g->player.bombs[i].explosion_up.height < (g->player.bomb_distance + 1) * TILESIZE) {
                    float indexTop = g->player.bombs[i].indexTop;

                    int prevBlock = mapa[(int)(indexTop - g->player.bomb_vel + 1)][(int)bombIndex.x];
                    int currentBlock = mapa[(int)(indexTop - g->player.bomb_vel)][(int)bombIndex.x];

                    if (prevBlock != 2 && currentBlock != 1 && indexTop - g->player.bomb_vel >= 0) {
                        g->player.bombs[i].indexTop -= g->player.bomb_vel;
                        g->player.bombs[i].explosion_up.height += TILESIZE * g->player.bomb_vel;
                        g->player.bombs[i].explosion_up.y -= TILESIZE * g->player.bomb_vel;
                    }
                }
                // Bottom
                if (g->player.bombs[i].explosion_down.height < (g->player.bomb_distance + 1) * TILESIZE) {
                    float indexBottom = g->player.bombs[i].indexBottom;

                    int prevBlock = mapa[(int)(indexBottom + g->player.bomb_vel - 0.001)][(int)bombIndex.x];
                    int currentBlock = mapa[(int)(indexBottom + 1)][(int)bombIndex.x];

                    if (prevBlock != 2 && currentBlock != 1 && (indexBottom + 1) < NUMTILES_H) {
                        g->player.bombs[i].indexBottom += g->player.bomb_vel;
                        g->player.bombs[i].explosion_down.height += TILESIZE * g->player.bomb_vel;
                    }
                }

            } else if(fabs(g->player.bombs[i].time - GetTime()) > 3){
                
                Vector2Int bombIndex = getIndex(g->player.bombs[i].pos.x, g->player.bombs[i].pos.y);

                mapa[(int)bombIndex.y][(int)(g->player.bombs[i].indexRight)] = 0; //Axtremidade da explosão para um bloco vazio na matriz, quebrando os blocos atingidos
                mapa[(int)bombIndex.y][(int)(g->player.bombs[i].indexLeft)] = 0;
                mapa[(int)(g->player.bombs[i].indexTop)][(int)bombIndex.x] = 0;
                mapa[(int)(g->player.bombs[i].indexBottom)][(int)bombIndex.x] = 0;
                
                
                mapa[bombIndex.y][bombIndex.x] = 0; //Atualiza a bomba na matriz para 0, desativando a colisão

                g->player.bombs[i].collisionActive = 0;
                g->player.bombs[i].isActive = 0;

	    }
        }
    }
    g->player.put_bomb = 0;
}

//Gera um mapa aleatório (Não utilizado)
void initRandomMap() {
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
void prevCollision(game *g, short int mapa[][NUMTILES_W]) {
    Vector2Int playerIndex = getIndex(g->player.pos.x, g->player.pos.y);

    for (int y = playerIndex.y - 1; y <= playerIndex.y + 1; y++) {
        for (int x = playerIndex.x - 1; x <= playerIndex.x + 1; x++) { //Obtém os blocos em torno do player
            if (mapa[y][x] >= 1 && x >= 0 && x < NUMTILES_W && y >= 0 && y < NUMTILES_H) { //Blocos maiores ou iguais a 1 colidem
                Vector2 coords = getCoords(x, y);

                //Demarca os blocos em torno do player
                DrawCircle(coords.x + TILESIZE / 2, coords.y + TILESIZE / 2, 4, RED);
                DrawRectangleLines(coords.x, coords.y, TILESIZE, TILESIZE, RED);

                int playerLeft = g->player.pos.x;
                int playerRight = g->player.pos.x + g->player.size;
                int playerTop = g->player.pos.y;
                int playerBottom = g->player.pos.y + g->player.size;

                int tileLeft = coords.x;
                int tileRight = coords.x + TILESIZE;
                int tileTop = coords.y;
                int tileBottom = coords.y + TILESIZE;

                Rectangle playerRect = {g->player.pos.x, g->player.pos.y, g->player.size, g->player.size};
                Rectangle tileRect = {coords.x, coords.y, TILESIZE, TILESIZE};

                if (CheckCollisionRecs(playerRect, tileRect)) {
                    int tolerance = TILESIZE - 10;
                    int move_on_collision = 2.8;

                    //Top
                    int top_dist = tileTop - playerTop;
                    if (top_dist >= tolerance) {
                        g->player.pos.y = tileTop - g->player.size; //Move o player para fora do bloco caso a colisão seja muito superficial (Menor ou igual a 10), pois foi de onde a colisão partiu
                    } else if (top_dist < tolerance && top_dist > 1) {
                        g->player.pos.y -= move_on_collision; //Move o player ligeiramente para direção oposta á colisão, pois não foi de onde a colisão partiu
                    }

                    //Left
                    int left_dist = tileLeft - playerLeft;
                    if (left_dist >= tolerance) {
                        g->player.pos.x = tileLeft - g->player.size;
                    } else if (left_dist < tolerance && left_dist > 1) {
                        g->player.pos.x -= move_on_collision;
                    }

                    //Bottom
                    int bottom_dist = playerBottom - tileBottom;
                    if (bottom_dist >= tolerance) {
                        g->player.pos.y = tileBottom;
                    } else if (bottom_dist < tolerance && bottom_dist > 1) {
                        g->player.pos.y += move_on_collision;
                    }

                    //Right
                    int right_dist = playerRight - tileRight;
                    if (right_dist >= tolerance) {
                        g->player.pos.x = tileRight;
                    } else if (right_dist < tolerance && right_dist > 1) {
                        g->player.pos.x += move_on_collision;
                    }
                }
            }
        }
    }
}

//Obtém a última tecla pressionada, para não andar em dois eixos ao mesmo tempo
void getLastKey() {
    if (IsKeyPressed(KEY_W)) lastKey = KEY_W;
    if (IsKeyPressed(KEY_A)) lastKey = KEY_A;
    if (IsKeyPressed(KEY_S)) lastKey = KEY_S;
    if (IsKeyPressed(KEY_D)) lastKey = KEY_D;

    if (IsKeyReleased(KEY_W) || IsKeyReleased(KEY_A) || IsKeyReleased(KEY_S) || IsKeyReleased(KEY_D)) {
        if (IsKeyDown(KEY_D))
            lastKey = KEY_D;
        else if (IsKeyDown(KEY_S))
            lastKey = KEY_S;
        else if (IsKeyDown(KEY_A))
            lastKey = KEY_A;
        else if (IsKeyDown(KEY_W))
            lastKey = KEY_W;
        else
            lastKey = KEY_NULL;
    }
}


//Tela inicial teste (Não utilizado)
void mainMenu() {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("Bomb SUS", SCREEN_W/2 - 125, 50, 50, BLUE);

    DrawRectangle(SCREEN_W/2 - 125, SCREEN_H/2 - 50, 250, 100, BLUE);
    EndDrawing();
}

//Tela de Game Over
void gameOver() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Game Over!", SCREEN_W/2 - 75, 300, 25, WHITE);
    EndDrawing();
}

#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "TetrisUtils.h"

//NOTE I switched to using system("cls") instead of the escape
// sequences but this is Windows only
#include <stdlib.h> /* for system("cls")*/
#include <windows.h> /* for GetAsyncKeyState */

#define bool char

typedef int shape[TETROMINO_WIDTH][TETROMINO_HEIGHT];

char board[BOARD_HEIGHT][BOARD_WIDTH];

int score = 0;


typedef struct
{
    int xPos;
    int yPos;
    shape shape;
} Tetranimo;

const shape SHAPES[NUMBER_OF_SHAPES] = {
    {0,1,1,0, //SQUARE
     0,1,1,0,
     0,0,0,0,
     0,0,0,0},

    {1,1,1,1, //LINE
     0,0,0,0,
     0,0,0,0,
     0,0,0,0},

    {0,1,1,1, //T
     0,0,1,0,
     0,0,0,0,
     0,0,0,0},

    {0,1,0,0, //L
     0,1,0,0,
     0,1,1,0,
     0,0,0,0},

    {0,1,0,0, //J
     0,1,0,0,
     0,1,1,0,
     0,0,0,0},

    {0,1,1,0, //S
     1,1,0,0,
     0,0,0,0,
     0,0,0,0},

    {0,1,1,0, //Z
     0,0,1,1,
     0,0,0,0,
     0,0,0,0}
};



typedef enum tetranimoes {
    SQUARE,
    LINE,
    T,
    L,
    J,
    S,
    Z,
} Tetranimoes;

/* NOTE: Representing Player actions with an enum for now,
 * may need to change to something more complicated in the future if this sucks
 */
typedef enum playerAction {
    IDLE, // TODO: Have an IDLE action right now if the player is not doing anything this 'game-tick'. Stupid? Change this? 
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_DOWN,
    ROTATE_RIGHT,
    ROTATE_LEFT,
    FORCE_DOWN,
    QUIT
} PlayerAction;

void initialize();
void update(bool*, PlayerAction, Tetranimo*);
void draw();
void teardown();

void drawPiece(Tetranimo* piece);
void erasePiece(Tetranimo* piece);
bool checkCollision(Tetranimo* piece, int, int);
void placePiece(Tetranimo* piece);

//TODO Right now I'm using a global ActivePiece and passing it to
//functions by pointer. This might need to change later or maybe just
//permanently draw the piece onto the board once it becomes inactive.
void spawnPiece(Tetranimo* piece);
void movePieceLeft(Tetranimo* piece);
void movePieceRight(Tetranimo* piece);
void rotateRight(Tetranimo* piece);
void rotateLeft(Tetranimo* piece);
void movePieceDown(Tetranimo* piece);
void forceDown(Tetranimo* piece);
Tetranimo spawnTetanimo();

//Flag to tell whether current piece is still active and movable. When this is false, you know to spawn a new piece. This is garbage?
bool piece_active = 1;


int main() {



    bool game_over = false;

    struct timeb start, end;
    ftime(&start);
    int time_diff;
    ftime(&start);


    initialize();

    //TODO hard-coded for the moment.


    Tetranimo ActivePiece = spawnTetanimo();
    spawnPiece(&ActivePiece);




    PlayerAction playerAction = IDLE;

    while (!game_over) {
        //NOTE AFAIK C's Standard Libraries input is all buffered so I
        // couldn't figure out a way to break the game loop in real
        // time. This looks evil because GetAsyncKeyState returns a
        // short whose most significant integer is set, hence the
        // bit-shifting to check if that's the case.
        if (int input = GetAsyncKeyState(VK_ESCAPE) & (1 << 15) != 0)
            playerAction = QUIT;
        if (int input = GetAsyncKeyState(VK_LEFT) & (1 << 15) != 0)
            playerAction = MOVE_LEFT;
        if (int input = GetAsyncKeyState(VK_RIGHT) & (1 << 15) != 0)
            playerAction = MOVE_RIGHT;
        if (int input = GetAsyncKeyState(VK_DOWN) & (1 << 15) != 0)
            playerAction = MOVE_DOWN;
        if (int input = GetAsyncKeyState(VK_UP) & (1 << 15) != 0)
            playerAction = ROTATE_RIGHT;
        if (int input = GetAsyncKeyState(Z_KEY) & (1 << 15) != 0)
            playerAction = ROTATE_LEFT;
        if (int input = GetAsyncKeyState(VK_SPACE) & (1 << 15) != 0)
            playerAction = FORCE_DOWN;



        ftime(&end);
        time_diff = (int)1000 * (end.time - start.time) + (end.millitm - start.millitm);
        if (time_diff > FRAME_RATE) {
            if (!piece_active) {
                ActivePiece = spawnTetanimo();
                spawnPiece(&ActivePiece);
                piece_active = 1;
            }
            update(&game_over, playerAction, &ActivePiece);
            draw();
            ftime(&start);
            playerAction = IDLE;
        }


    }

    teardown();


}

void initialize() {
    system("cls");

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (j == 0 || j == BOARD_WIDTH - 1) {
                board[i][j] = '|';
            }
            else if (i == BOARD_HEIGHT - 1) {
                board[i][j] = '_';
            }
            else {
                board[i][j] = '.';
            }
        }
    }
}

void update(bool* game_over, PlayerAction playerAction,Tetranimo* ActivePiece) {
    switch (playerAction) {
    case QUIT: {
        *game_over = true;
        break;
    }
    case MOVE_LEFT: {
        movePieceLeft(ActivePiece);
        break;
    }
    case MOVE_RIGHT: {
        movePieceRight(ActivePiece);
        break;
    }
    case MOVE_DOWN: {
        movePieceDown(ActivePiece);
        break;
    }
    case ROTATE_RIGHT: {
        rotateRight(ActivePiece);
        break;
    }
    case ROTATE_LEFT: {
        rotateLeft(ActivePiece);
        break;
    }
    case FORCE_DOWN: {
        forceDown(ActivePiece);
        break;
    }
    }
    //Move a piece down once per game tick no matter what.
    //TODO: make this happen at a faster rate the longer the game goes.
    movePieceDown(ActivePiece);

}

void draw() {
    system("cls");
    printf("SCORE:%d\n\n", score);
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            printf("%c", board[i][j]);
        }
        printf("\n");
    }

    //NOTE FOR DEBUGGING
    //printf("Active Piece xPos: %d Active Piece yPos: %d", ActivePiece.xPos, ActivePiece.yPos);


}

void teardown() {
    system("cls");
    printf("GAME_OVER!");
    //free(ActivePiece);

}

/* spawnPiece: spawns a piece centered at the top of the board. It can be any shaped piece */
void spawnPiece(Tetranimo* piece)
{

    drawPiece(piece);
}

/* movePieceLeft: moves a piece to the left*/
void movePieceLeft(Tetranimo* piece)
{
    erasePiece(piece);
    piece->xPos--;
    drawPiece(piece);
}

void movePieceRight(Tetranimo* piece)
{
    erasePiece(piece);
    piece->xPos++;
    drawPiece(piece);
}

void movePieceDown(Tetranimo* piece)
{
    erasePiece(piece);
    piece->yPos++;
    drawPiece(piece);
}




void rotateRight(Tetranimo* piece) {
    //TODO: Right now I'm just rotating the internal shape of the tetranimo.
    erasePiece(piece);

    //To rotate 90 degrees clockwise (to the right). First take the transpose of the array and then reflect about the center by swaping elements in the ROWS.
    transposeOfTetranimoShapeMatrix(piece->shape);
    for (int i = 0; i < TETROMINO_WIDTH; i++) {
        for (int j = 0; j < TETROMINO_HEIGHT / 2; j++) {
            swap(piece->shape, i, j, i, TETROMINO_HEIGHT - j - 1);
        }
    }
    drawPiece(piece);
}

void rotateLeft(Tetranimo* piece) {
    //TODO: Right now I'm just rotating the internal shape of the tetranimo.
    erasePiece(piece);

    //To rotate 90 degrees counter-clockwise (to the left). First take the transpose of the array and then reflect about the center by swaping elements in the COLUMNS.
    transposeOfTetranimoShapeMatrix(piece->shape);
    for (int i = 0; i < TETROMINO_HEIGHT; i++) {
        for (int j = 0; j < TETROMINO_WIDTH / 2; j++) {
            swap(piece->shape, j, i, TETROMINO_WIDTH - j - 1, i);
        }
    }
    drawPiece(piece);
}


void forceDown(Tetranimo* piece)
{
    //erasePiece(piece);
    //piece->yPos = BOARD_HEIGHT - 2;
    //drawPiece(piece);
    while (piece_active) {
        movePieceDown(piece);
    }
    //placePiece(piece);
}


void drawPiece(Tetranimo* piece) {

    for (int i = 0; i < TETROMINO_WIDTH && piece->yPos + i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT && piece->xPos + j < BOARD_WIDTH; j++)
        {
            if (piece->shape[i][j] == 1)
            {
                if (!(checkCollision(piece, (piece->yPos + i), (piece->xPos + j))))
                {
                    board[piece->yPos + i][piece->xPos + j] = '#';
                }
                else
                {
                    break;
                }

            }

        }
    }
}

/* erasePiece: Erases a piece from the board */
void erasePiece(Tetranimo* piece)
{
    for (int i = 0; i < TETROMINO_WIDTH && piece->yPos + i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT && piece->xPos + j < BOARD_WIDTH; j++)
        {
            if (board[piece->yPos + i][piece->xPos + j] == '#')
                board[piece->yPos + i][piece->xPos + j] = '.';

        }
    }

}

/* checkCollision: checks to see if the piece collided with the edges
 * of the board, and returns true if that's the case. */
 //TODO Check for collisions with other placed pieces.
bool checkCollision(Tetranimo* piece, int y, int x)
{
    if (board[y][x] != '|' && board[y][x] != '_')
    {
        //There was no collision
        return false;
    }
    else
    {
        if (x <= 0)
        {
            //COLLISION WITH THE LEFT SIDE OF THE BOARD
            erasePiece(piece);
            piece->xPos++;
            drawPiece(piece);
            return true;
        }
        if (x > BOARD_WIDTH - 2)
        {
            //COLLISION WITH THE RIGHT SIDE OF THE BOARD
            erasePiece(piece);
            piece->xPos--;
            drawPiece(piece);
            return true;
        }
        if (y > BOARD_HEIGHT - 2)
        {
            //COLLISION WITH THE BOTTOM OF THE BOARD
            erasePiece(piece);
            piece->yPos--;
            drawPiece(piece);
            //If you are colliding with the bottom of the board, you ran out of time. Its over. The piece should be placed automatically for you.
            placePiece(piece);
            return true;
        }
        else {

            system("cls");
            printf("error: checkCollision, invalid collision\n");
            return true;
        }
    }
}

Tetranimo spawnTetanimo() {
    int shapeIndex;
    srand((unsigned)time(NULL));
    shapeIndex = rand() % 7;

    Tetranimo tetranimo;
    tetranimo.xPos = TETRAMINO_STARTING_XPOS;
    tetranimo.yPos = 0;
    memcpy(tetranimo.shape, &SHAPES[shapeIndex], sizeof(SHAPES[shapeIndex]));

    return tetranimo;

}

/* placePiece: places a piece onto the board */
void placePiece(Tetranimo* piece)
{
    for (int i = 0; i < TETROMINO_WIDTH && piece->yPos + i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT && piece->xPos + j < BOARD_WIDTH; j++)
        {
            if (board[piece->yPos + i][piece->xPos + j] == '#')
                board[piece->yPos + i][piece->xPos + j] = '*';

        }
    }
    //You placed the current piece so set the piece active flag off.
    piece_active = 0;
 
}

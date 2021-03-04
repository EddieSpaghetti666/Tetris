#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "TetrisUtils.h"
#include <simple2d.h>
#include <SDL2/SDL.h>
#undef main


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
    {0,1,1, //SQUARE
     0,1,1,
     0,0,0,
     },

    {1,1,1, //LINE
     0,0,0,
     0,0,0
     },

    {0,1,0, //T
     1,1,1,
     0,0,0,
    },

    {1,0,0, //L
     1,1,1,
     0,0,0,
    },

    {0,0,1, //J
     1,1,1,
     0,0,0,
     },

    {0,1,1, //S
     1,1,0,
     0,0,0,
     },

    {1,1,0, //Z
     0,1,1,
     0,0,0,
    }
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
void sweepBoard();

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

void render() {
    S2D_DrawTriangle(
        320, 50, 1, 0, 0, 1,
        540, 430, 0, 1, 0, 1,
        100, 430, 0, 0, 1, 1
    );
}


int main() {

    S2D_Window* window = S2D_CreateWindow(
        "Hello Triangle", 640, 480, NULL, render, 0
    );

    S2D_Show(window);



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
                playerAction = IDLE;
            }
            update(&game_over, playerAction, &ActivePiece);
            draw();
            ftime(&start);
            playerAction = IDLE;
        }


    }

    teardown();

    return 0;


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

void update(bool* game_over, PlayerAction playerAction, Tetranimo* ActivePiece) {
    switch (playerAction) {
    case QUIT: {
        *game_over = true;
        break;
    }
    case MOVE_LEFT: {
        movePieceLeft(ActivePiece);
        playerAction = IDLE;
        break;
    }
    case MOVE_RIGHT: {
        movePieceRight(ActivePiece);
        playerAction = IDLE;
        break;
    }
    case MOVE_DOWN: {
        movePieceDown(ActivePiece);
        playerAction = IDLE;
        break;
    }
    case ROTATE_RIGHT: {
        rotateRight(ActivePiece);
        playerAction = IDLE;
        break;
    }
    case ROTATE_LEFT: {
        rotateLeft(ActivePiece);
        playerAction = IDLE;
        break;
    }
    case FORCE_DOWN: {
        forceDown(ActivePiece);
        playerAction = IDLE;
        break;
    }
    }
    //Move a piece down once per game tick no matter what.
    //TODO: make this happen at a faster rate the longer the game goes.
    if (piece_active)
        movePieceDown(ActivePiece);

    //TODO Should this be done outside of update? It checks the board every game tick

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

}

void teardown() {
    system("cls");
    printf("GAME_OVER!");
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
    while (piece_active) {
        movePieceDown(piece);
    }
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
                    return;
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
 * of the board or a placed piece. If true, it returns true and places the piece. */
bool checkCollision(Tetranimo* piece, int y, int x)
{
    if (board[y][x] != '|' && board[y][x] != '_' && board[y][x] != '*')
    {
        //There was no collision
        return false;
    }
    else if (board[y][x] == '*')
    {
        erasePiece(piece);
        piece->yPos--;
        drawPiece(piece);
        placePiece(piece);

        return true;
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
    sweepBoard();
}

/* scanCompleteRow: Scans the entire board, and if it finds a
   completed row of '*'s it keeps track of it in an array of ints
   pointed to by *rows */
int* scanCompletedRow(void) {
    static int rows[4];
    int rowIndex = 0;
    int i, j, k;

    for (j = 0; j < BOARD_HEIGHT; j++)
    {
        for (i = 0, k = 0; i < BOARD_WIDTH; i++)
        {
            if (board[j][i] == '*')
                k++;

        }
        if (k == BOARD_WIDTH - 2)
            rows[rowIndex++] = j;
    }

    return rows;

}

/* breakCompleteRow: Turns a row on the board to '.'s */
void breakCompletedRow(int row)
{
    int i;
    for (i = 1; i < BOARD_WIDTH - 1; i++)
        board[row][i] = '.';
    // TODO: Idk how scoring works in Tetris so it goes up 1 per row for now.
    score++;

}

/* sweepBoard: small little function that checks for completed rows
   and calls breakCompletedRow on the appropriate row. Right now I
   only intend to do this after a piece is placed. */
void sweepBoard(void)
{
    int scancompletedRow();
    void breakCompletedRow(int);

    int* rowCheck;
    int i;
    for (i = 0, (rowCheck = scanCompletedRow()); i < 4 && *rowCheck > 0; i++, rowCheck++)
    {
        breakCompletedRow(*rowCheck);
        *rowCheck = 0;
    }
}

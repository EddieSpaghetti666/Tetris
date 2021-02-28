#include <stdio.h>
#include <sys/timeb.h>
#include "TetrisUtils.h"

//NOTE I switched to using system("cls") instead of the escape
// sequences but this is Windows only
#include <stdlib.h> /* for system("cls")*/
#include <windows.h> /* for GetAsyncKeyState */

#define bool char

char board[BOARD_HEIGHT][BOARD_WIDTH];

int score = 0;


typedef struct
{
    int xPos;
    int yPos;
    int shape[TETROMINO_WIDTH][TETROMINO_HEIGHT];
} Tetranimo;

//TODO "Empty Space above the square causes it to spawn in one line
//lower.
Tetranimo Square =
{
    0,
    0,
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
    },
};
Tetranimo L =
{
    0,
    0,
    {
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
    },
};

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
void update(bool*, PlayerAction);
void draw();
void teardown();

void drawPiece(Tetranimo* piece);
void erasePiece(Tetranimo* piece);
bool checkCollision(Tetranimo* piece, int, int);

//TODO Right now I'm using a global ActivePiece and passing it to
//functions by pointer. This might need to change later or maybe just
//permanently draw the piece onto the board once it becomes inactive.
static Tetranimo ActivePiece;
void spawnPiece(Tetranimo* piece);
void movePieceLeft(Tetranimo* piece);
void movePieceRight(Tetranimo* piece);
void rotateRight(Tetranimo* piece);
void rotateLeft(Tetranimo* piece);
void movePieceDown(Tetranimo* piece);

int main() {



    bool game_over = false;

    struct timeb start, end;
    ftime(&start);
    int time_diff;
    ftime(&start);


    initialize();

    //TODO hard-coded for the moment.
    spawnPiece(&L);
    ActivePiece = L;




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



        ftime(&end);
        time_diff = (int)1000 * (end.time - start.time) + (end.millitm - start.millitm);
        if (time_diff > FRAME_RATE) {
            update(&game_over, playerAction);
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

void update(bool* game_over, PlayerAction playerAction) {
    switch (playerAction) {
    case QUIT: {
        *game_over = true;
        break;
    }
    case MOVE_LEFT: {
        movePieceLeft(&ActivePiece);
        break;
    }
    case MOVE_RIGHT: {
        movePieceRight(&ActivePiece);
        break;
    }
    case MOVE_DOWN: {
        movePieceDown(&ActivePiece);
        break;
    }
    case ROTATE_RIGHT: {
        rotateRight(&ActivePiece);
        break;
    }
    case ROTATE_LEFT: {
        rotateLeft(&ActivePiece);
        break;
    }

    }

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
    printf("Active Piece xPos: %d Actvie Piece yPos: %d", ActivePiece.xPos, ActivePiece.yPos);


}

void teardown() {
    system("cls");
    printf("GAME_OVER!");

}

/* spawnPiece: spawns a piece centered at the top of the board. It can be any shaped piece */
void spawnPiece(Tetranimo* piece)
{
    int mid_x = (BOARD_WIDTH / 2) - 2;
    int i, j;

    piece->xPos = mid_x;
    piece->yPos = 0;


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

void drawPiece(Tetranimo* piece) {

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
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
    for (int i = 0; i < TETROMINO_WIDTH; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT; j++)
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
            return true;
        }
        else {

            system("cls");
            printf("error: checkCollision, invalid collision\n");
            return true;
        }
    }
}

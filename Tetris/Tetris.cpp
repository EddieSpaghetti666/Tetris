#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "TetrisUtils.h"
#include <stdlib.h> /* for system("cls")*/
#include <windows.h> /* for GetAsyncKeyState */

#define bool char

typedef struct
{
    int xPos;
    int yPos;
    shape shape;
} Tetranimo;

typedef enum tetranimoes {
    SQUARE,
    LINE,
    T,
    L,
    J,
    S,
    Z,
} Tetranimoes;

typedef enum class PlayerAction{
    IDLE, 
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_DOWN,
    ROTATE_RIGHT,
    ROTATE_LEFT,
    FORCE_DOWN,
    QUIT
};

typedef enum class PieceDirection{
    LEFT,
    RIGHT,
    DOWN
};

typedef enum class GameState{
    PLAYING,
    PAUSED,
    OVER
};

typedef struct {
    GameState state; 
    bool pieceIsActive;
    int level;
    char board[BOARD_HEIGHT][BOARD_WIDTH];
    int score;
    int totalLinesCleared;
    int framesUntilNextDrop;
    Tetranimo activePiece;
} Game;

Game initialize();
void update(PlayerAction, Game*);
void draw(Game*);
void teardown();
void drawActivePiece(Game*);
void eraseActivePiece(Game*);
bool checkCollision(Game*, int, int);
void placeActivePiece(Game*);
void sweepBoard(Game*);
void spawnActivePiece(Game*);
void moveActivePiece(Game*, PieceDirection);
void rotateActivePiece(Game*, bool);
void forceActivePieceDown(Game*);
void dropRows(Game*, int);
Tetranimo spawnTetranimo();

int main() {

    struct timeb gameStart, gameCurrent, frameStart, frameEnd;
    ftime(&frameStart);
    int frameTimeDiff, timeSinceStart;

    Game* game = &initialize();
    PlayerAction playerAction = PlayerAction::IDLE;


    /* Game Loop */
    while (game->state != GameState::OVER) {
      
        ftime(&frameEnd);
        frameTimeDiff = (int)1000 * (frameEnd.time - frameStart.time) + (frameEnd.millitm - frameStart.millitm);
       
        if (int input = GetAsyncKeyState(VK_ESCAPE) & (1 << 15) != 0)
            playerAction = PlayerAction::QUIT;
        else if (int input = GetAsyncKeyState(VK_LEFT) & (1 << 15) != 0)
            playerAction = PlayerAction::MOVE_LEFT;
        else if (int input = GetAsyncKeyState(VK_RIGHT) & (1 << 15) != 0)
            playerAction = PlayerAction::MOVE_RIGHT;
        else if (int input = GetAsyncKeyState(VK_DOWN) & (1 << 15) != 0)
            playerAction = PlayerAction::MOVE_DOWN;
        else if (int input = GetAsyncKeyState(VK_UP) & (1 << 15) != 0)
            playerAction = PlayerAction::ROTATE_RIGHT;
        else if (int input = GetAsyncKeyState(Z_KEY) & (1 << 15) != 0)
            playerAction = PlayerAction::ROTATE_LEFT;
        else if (int input = GetAsyncKeyState(VK_SPACE) & (1 << 15) != 0)
            playerAction = PlayerAction::FORCE_DOWN;

        if (frameTimeDiff > FRAME_RATE) {
            if (!game->pieceIsActive) {
                game->activePiece = spawnTetranimo();
                spawnActivePiece(game);
                game->pieceIsActive = true;
                playerAction = PlayerAction::IDLE;
            }
            update(playerAction, game);
            draw(game);
            ftime(&frameStart);
            playerAction = PlayerAction::IDLE;
        }


    }

    teardown();


}

Game initialize() {
    system("cls");
    Game game;
    game.level = 1;
    game.score = 0;
    game.totalLinesCleared = 0;
    game.pieceIsActive = false;
    game.state = GameState::PLAYING;
    game.framesUntilNextDrop = INITIAL_GRAVITY;
    //fill in the board.
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (j == 0 || j == BOARD_WIDTH - 1) {
                game.board[i][j] = '|';
            }
            else if (i == BOARD_HEIGHT - 1) {
                game.board[i][j] = '_';
            }
            else {
                game.board[i][j] = '.';
            }
        }
    }
    return game;
}

void update(PlayerAction playerAction, Game* game) {
    switch (playerAction) {
    case PlayerAction::QUIT: {
        game->state = GameState::OVER;
        break;
    }
    case PlayerAction::MOVE_LEFT: {
        moveActivePiece(game, PieceDirection::LEFT);
        break;
    }
    case PlayerAction::MOVE_RIGHT: {
        moveActivePiece(game, PieceDirection::RIGHT);
        break;
    }
    case PlayerAction::MOVE_DOWN: {
        moveActivePiece(game, PieceDirection::DOWN);
        break;
    }
    case PlayerAction::ROTATE_RIGHT: {
        rotateActivePiece(game, true); //True, because you are rotating clockwise.
        break;
    }
    case PlayerAction::ROTATE_LEFT: {
        rotateActivePiece(game, false); //False, you are rotating anticlockwise
        break;
    }
    case PlayerAction::FORCE_DOWN: {
        forceActivePieceDown(game);
        break;
    }
    }
    game->level = 1 + game->totalLinesCleared / 1;
    if (game->framesUntilNextDrop == 0) {
        moveActivePiece(game, PieceDirection::DOWN);
        game->framesUntilNextDrop = INITIAL_GRAVITY - game->level;
    }
    game->framesUntilNextDrop--;

}

void draw(Game* game) {
    system("cls");
    printf("Lines:%d\nLevel:%d\n\nSCORE:%d\n\n", game->totalLinesCleared,game->level,game->score);
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            printf("%c", game->board[i][j]);
        }
        printf("\n");
    }

}

void teardown() {
    system("cls");
    printf("GAME_OVER!");
}

/* spawnPiece: spawns a piece centered at the top of the board. It can be any shaped piece */
void spawnActivePiece(Game* game)
{
    drawActivePiece(game);
}

void moveActivePiece(Game* game, PieceDirection direction)
{
    eraseActivePiece(game);
    switch (direction) {
    case PieceDirection::LEFT: {
        game->activePiece.xPos--;
        break;
    }
    case PieceDirection::RIGHT: {
        game->activePiece.xPos++;
        break;
    }
    case PieceDirection::DOWN: {
        game->activePiece.yPos++;
        break;
    }
    }
    drawActivePiece(game);
}


void rotateActivePiece(Game* game, bool clockwise) {
    eraseActivePiece(game);

    transposeOfTetranimoShapeMatrix(game->activePiece.shape);

    for (int i = 0; i < TETROMINO_WIDTH; i++) {
        for (int j = 0; j < TETROMINO_HEIGHT / 2; j++) {
            //The only thing that changes if you rotate clockwise vs anticlockwise is whether or not you swap elements in the rows or columns:
            if (clockwise) {
                swap(game->activePiece.shape, i, j, i, TETROMINO_HEIGHT - j - 1);
            }
            else {
                //ANTICLOCKWISE ROTATION
                swap(game->activePiece.shape, j, i, TETROMINO_WIDTH - j - 1, i);
            }
        }
    }
    drawActivePiece(game);
}

void forceActivePieceDown(Game* game)
{
    while (game->pieceIsActive) {

        moveActivePiece(game, PieceDirection::DOWN);
    }
}


void drawActivePiece(Game* game) {

    for (int i = 0; i < TETROMINO_WIDTH && game->activePiece.yPos + i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT && game->activePiece.xPos + j < BOARD_WIDTH; j++)
        {
            if (game->activePiece.shape[i][j] == 1)
            {
                if (!(checkCollision(game, (game->activePiece.yPos + i), (game->activePiece.xPos + j))))
                {

                    game->board[game->activePiece.yPos + i][game->activePiece.xPos + j] = '#';
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
void eraseActivePiece(Game* game)
{
    for (int i = 0; i < TETROMINO_WIDTH && game->activePiece.yPos + i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT && game->activePiece.xPos + j < BOARD_WIDTH; j++)
        {
            if (game->board[game->activePiece.yPos + i][game->activePiece.xPos + j] == '#')
                game->board[game->activePiece.yPos + i][game->activePiece.xPos + j] = '.';

        }
    }

}

/* checkCollision: checks to see if the piece collided with the edges
 * of the board or a placed piece. If true, it returns true and places the piece. */
bool checkCollision(Game* game, int dest_row, int dest_col)
{
    if (game->board[dest_row][dest_col] != '|' && game->board[dest_row][dest_col] != '_' && game->board[dest_row][dest_col] != '*')
    {
        //There was no collision
        return false;
    }
    else if (game->board[dest_row][dest_col] == '*')
    {
        eraseActivePiece(game);
        game->activePiece.yPos--;
        drawActivePiece(game);
        placeActivePiece(game);

        return true;
    }
    else
    {
        if (dest_col <= 0)
        {
            //COLLISION WITH THE LEFT SIDE OF THE BOARD
            eraseActivePiece(game);
            game->activePiece.xPos++;
            drawActivePiece(game);
            return true;
        }
        if (dest_col > BOARD_WIDTH - 2)
        {
            //COLLISION WITH THE RIGHT SIDE OF THE BOARD
            eraseActivePiece(game);
            game->activePiece.xPos--;
            drawActivePiece(game);
            return true;
        }
        if (dest_row > BOARD_HEIGHT - 2)
        {
            //COLLISION WITH THE BOTTOM OF THE BOARD
            eraseActivePiece(game);
            game->activePiece.yPos--;
            drawActivePiece(game);
            //If you are colliding with the bottom of the board, you ran out of time. Its over. The piece should be placed automatically for you.
            placeActivePiece(game);
            return true;
        }
        else {

            system("cls");
            printf("error: checkCollision, invalid collision\n");
            return true;
        }
    }
}

Tetranimo spawnTetranimo() {
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
void placeActivePiece(Game* game)
{
    for (int i = 0; i < TETROMINO_WIDTH && game->activePiece.yPos + i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < TETROMINO_HEIGHT && game->activePiece.xPos + j < BOARD_WIDTH; j++)
        {
            if (game->board[game->activePiece.yPos + i][game->activePiece.xPos + j] == '#')
                game->board[game->activePiece.yPos + i][game->activePiece.xPos + j] = '*';

        }
    }
    //You placed the current piece so set the piece active flag off.
    game->pieceIsActive = false;
    sweepBoard(game);
}

/* scanCompleteRow: Scans the entire board, and if it finds a
   completed row of '*'s it keeps track of it in an array of ints
   pointed to by *rows */
int* scanCompletedRow(Game* game) {
    static int rows[4];
    int rowIndex = 0;
    int i, j, k;

    for (j = 0; j < BOARD_HEIGHT; j++)
    {
        for (i = 0, k = 0; i < BOARD_WIDTH; i++)
        {
            if (game->board[j][i] == '*')
                k++;

        }
        if (k == BOARD_WIDTH - 2)
            rows[rowIndex++] = j;
    }

    return rows;

}

/* breakCompleteRow: Turns a row on the board to '.'s */
void breakCompletedRow(Game* game, int row)
{
    int i;
    for (i = 1; i < BOARD_WIDTH - 1; i++)
        game->board[row][i] = '.';
    game->totalLinesCleared++;
    game->score += 40 * (game->level);

}

/* sweepBoard: small little function that checks for completed rows
   and calls breakCompletedRow on the appropriate row. Right now I
   only intend to do this after a piece is placed. */
void sweepBoard(Game* game)
{
    int topRowBroken = 0;

    int* rowCheck;
    int i;
    for (i = 0, (rowCheck = scanCompletedRow(game)); i < 4 && *rowCheck > 0; i++, rowCheck++)
    {
        breakCompletedRow(game,*rowCheck);
        if (topRowBroken == 0)
            topRowBroken = *rowCheck;
        *rowCheck = 0;
    }

    if (topRowBroken > 0)
        dropRows(game,topRowBroken);
}

/* dropRows: drops remaining placed blocks down after completed rows
 * are broken. Takes the top row broken as a parameter */
void dropRows(Game* game,int topRowBroken)
{
    int i, j, k;
    // TODO: this might be doing more work than we need it to. If we
    // made the check j > topRowBroken - 4 would that be enough?
    for (j = topRowBroken - 1; j > topRowBroken - 5; j--)
    {
        for (i = 1; i < BOARD_WIDTH - 1; i++)
        {
            if (game->board[j][i] == '*')
            {
                //erase the current block
                game->board[j][i] = '.';

                //find the bottom place on the board that is not a * or _
                for (k = 0; game->board[j + k][i] != '*' && game->board[j + k][i] != '_'; k++)
                    ;
                k--;

                //re-draw the block at that location
                game->board[j + k][i] = '*';


            }


        }
    }
}

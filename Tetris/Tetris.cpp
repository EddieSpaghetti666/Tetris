#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "TetrisUtils.h"
#include <stdlib.h> /* for system("cls")*/
#include <windows.h> /* for GetAsyncKeyState */

int main() {

    struct timeb frameStart, frameEnd;
    ftime(&frameStart);
    int frameTimeDiff;

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
                game->activePiece.type == Type::ACTIVE;
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
        rotateActivePiece(&game->activePiece, game->board, true); //True, because you are rotating clockwise.
        break;
    }
    case PlayerAction::ROTATE_LEFT: {
        rotateActivePiece(&game->activePiece, game->board, false); //False, you are rotating anticlockwise
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
    drawPiece(&game->activePiece, game->board);
}

void moveActivePiece(Game* game, PieceDirection direction)
{
    eraseActivePiece(&game->activePiece, game->board);
    Point destPoints[TETROMINO_POINTS];
    Point newPoint;
    //TODO: there is probably a better way to do this than looping through every time but I can't think right now.
    switch (direction) {
    case PieceDirection::LEFT: {
        for (int i = 0; i < TETROMINO_POINTS; i++) {
            newPoint.x = game->activePiece.points[i].x - 1;
            newPoint.y = game->activePiece.points[i].y;
            destPoints[i] = newPoint;
        }
        if (!checkCollision(destPoints, game->board)) {
            memcpy(game->activePiece.points, destPoints, sizeof(game->activePiece.points));
            game->activePiece.pivot = game->activePiece.points[1];
        }
        break;
    }
    case PieceDirection::RIGHT: {
        for (int i = 0; i < TETROMINO_POINTS; i++) {
            newPoint.x = game->activePiece.points[i].x + 1;
            newPoint.y = game->activePiece.points[i].y;
            destPoints[i] = newPoint;
        }
        if (!checkCollision(destPoints, game->board)) {
            memcpy(game->activePiece.points, destPoints, sizeof(game->activePiece.points));
            game->activePiece.pivot = game->activePiece.points[1];
        }
        break;
    }
    case PieceDirection::DOWN: {
        for (int i = 0; i < TETROMINO_POINTS; i++) {
            newPoint.x = game->activePiece.points[i].x;
            newPoint.y = game->activePiece.points[i].y + 1;
            destPoints[i] = newPoint;
        }
        if (!checkCollision(destPoints, game->board)) {
            memcpy(game->activePiece.points, destPoints, sizeof(game->activePiece.points));
            game->activePiece.pivot = game->activePiece.points[1];
        }
        else {
            placeActivePiece(game);
            return;
        }
        break;
    }
    }
    drawPiece(&game->activePiece, game->board);
}


void rotateActivePiece(Tetranimo* piece, Board board, bool clockwise) {
    Point relativeToPivot[4];
    Point* result = getPointsRelativeToPivot(piece->points, piece->pivot);
    for (int i = 0; i < 4; i++) {
        relativeToPivot[i] = result[i];
    }
    Point rotatedPoints[TETROMINO_POINTS];
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        Point rotatedPoint;
        int vector[2] = { relativeToPivot[i].x, relativeToPivot[i].y };
        int rotationMatrix[2][2] = { 0, -1,
                                    1,  0 };
        int* rotatedVector = matrixVectorProduct2(vector, rotationMatrix);
        rotatedPoint.x = rotatedVector[0] + piece->pivot.x;
        rotatedPoint.y = rotatedVector[1] + piece->pivot.y;
        rotatedPoints[i] = rotatedPoint;
    }
    eraseActivePiece(piece, board);
    if (!checkCollision(rotatedPoints,board)) {
        memcpy(piece->points, rotatedPoints, sizeof(piece->points));
        piece->pivot = piece->points[1];
    }
    drawPiece(piece, board);
   
}

void forceActivePieceDown(Game* game)
{
    while (game->pieceIsActive) {

        moveActivePiece(game, PieceDirection::DOWN);
    }
}


void drawPiece(Tetranimo* piece, Board board) {
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        board[piece->points[i].y][piece->points[i].x] = '#';
   }
}

/* erasePiece: Erases a piece from the board */
void eraseActivePiece(Tetranimo* piece, Board board)
{
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        board[piece->points[i].y][piece->points[i].x] = '.';
    }

}

/* checkCollision: checks to see if the piece collided with the edges
 * of the board or a placed piece. If true, it returns true and places the piece. */
bool checkCollision(Point points[], Board board)
{
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        int row = points[i].x;
        int col = points[i].y;
        if (row > BOARD_WIDTH - 1 || row < 0 || col > BOARD_HEIGHT - 1 || col < 0) {
            return true;
        }
        if (board[col][row] != '.') {
            return true;
        }
    }
    return false;
}

Tetranimo spawnTetranimo() {
    int shapeIndex;
    srand((unsigned)time(NULL));
    shapeIndex = rand() % 7;
    Tetranimo tetranimo;
    memcpy(tetranimo.points, SHAPES[shapeIndex], sizeof(tetranimo.points));
    tetranimo.pivot = tetranimo.points[1];

    return tetranimo;

}

/* placePiece: places a piece onto the board */
void placeActivePiece(Game* game)
{
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        int row = game->activePiece.points[i].y;
        int col = game->activePiece.points[i].x;
        game->board[row][col] = '*';
    }
    //You placed the current piece so set the piece active flag off.
    game->pieceIsActive = false;
    sweepBoard(game);
}

/* scanCompleteRow: Scans the entire board, and if it finds a
   completed row of '*'s it keeps track of it in an array of ints
   pointed to by *rows */
int* scanCompletedRow(Board board) {
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
    for (i = 0, (rowCheck = scanCompletedRow(game->board)); i < 4 && *rowCheck > 0; i++, rowCheck++)
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


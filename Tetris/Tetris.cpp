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
        else if (int input = GetAsyncKeyState(C_KEY) & (1 << 15) != 0)
            playerAction = PlayerAction::HOLD;

        if (frameTimeDiff > FRAME_RATE) {
            if (!game->pieceIsActive) {
                game->activePiece = spawnTetranimo();
                spawnActivePiece(game);
                game->activePiece.type = Type::ACTIVE;
                game->pieceIsActive = true;
                updateGhostPiece(game);
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
    game.pieceIsHeld = false;
    game.state = GameState::PLAYING;
    game.framesUntilNextDrop = INITIAL_GRAVITY;
    //This line is so that the held piece appears correctly as None instead of Line
    game.heldPiece.shape = 69;
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
    void holdPiece(Game * game);
    eraseActivePiece(&game->activePiece, game->board);
    eraseActivePiece(&game->ghostPiece, game->board);
    Tetranimo movedPiece = game->activePiece;
    switch (playerAction) {
    case PlayerAction::QUIT: {
        game->state = GameState::OVER;
        break;
    }
    case PlayerAction::MOVE_LEFT: {
        movedPiece = movePiece(game->activePiece, PieceDirection::LEFT);
        break;
    }
    case PlayerAction::MOVE_RIGHT: {
        movedPiece = movePiece(game->activePiece, PieceDirection::RIGHT);
        break;
    }
    case PlayerAction::MOVE_DOWN: {
        movedPiece = movePiece(game->activePiece, PieceDirection::DOWN);
        break;
    }
    case PlayerAction::ROTATE_RIGHT: {
        movedPiece = rotatePiece(game->activePiece, game->board, true); //True, because you are rotating clockwise.
        break;
    }
    case PlayerAction::ROTATE_LEFT: {
        movedPiece = rotatePiece(game->activePiece, game->board, false); //False, you are rotating anticlockwise
        break;
    }
    case PlayerAction::FORCE_DOWN: {
        game->activePiece = forcePieceDown(game->activePiece, game->board);
        placeActivePiece(game);
        return;
    }
    case PlayerAction::HOLD: {
        holdPiece(game);
        return;
        //break;
    }
    }
    //If there was a collision revert the piece movement.
    if (checkCollision(movedPiece.points, game->board)) {
        if (playerAction == PlayerAction::MOVE_DOWN) {
            placeActivePiece(game);
            return;
        }
        movedPiece = game->activePiece;
    }
    

    game->activePiece = movedPiece;
    handleGravity(game);
    updateGhostPiece(game);
    if(game->pieceIsActive)
        drawPiece(game->activePiece, game->board);
    if(game->drawGhostPiece)
        drawPiece(game->ghostPiece, game->board);

}

void draw(Game* game) {
    system("cls");
    printf("Lines:%d\nLevel:%d\n\nSCORE:%d\n\n", game->totalLinesCleared, game->level, game->score);
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            printf("%c", game->board[i][j]);
        }
        printf("\n");
    }

    /*TODO: THIS SECTION IS FOR THE HELD PIECE. MOVE THIS OUT TO A SEPERATE FUNCTION? */
    char held[10];
    switch (game->heldPiece.shape)
    {
    case 0:
        strcpy_s(held, "Line");
        break;
    case 1:
        strcpy_s(held, "Square");
        break;
    case 2:
        strcpy_s(held, "J");
        break;
    case 3:
        strcpy_s(held, "L");
        break;
    case 4:
        strcpy_s(held, "S");
        break;
    case 5:
        strcpy_s(held, "T");
        break;
    case 6:
        strcpy_s(held, "Z");
        break;
    default:
        strcpy_s(held,"None");
        break;
    }
    printf("\nHeld Piece: %s", held);
}

void teardown() {
    system("cls");
    printf("GAME_OVER!");
}

/* spawnPiece: spawns a piece centered at the top of the board. It can be any shaped piece */
void spawnActivePiece(Game* game)
{
    drawPiece(game->activePiece, game->board);
}
/* Takes a piece and a direction to move the piece in. Returns a new piece with updated coordinates.*/
Tetranimo movePiece(Tetranimo piece, PieceDirection direction)
{
    Point newCoords[TETROMINO_POINTS];
    int x_offset = 0;
    int y_offset = 0;
    switch (direction) {
    case PieceDirection::LEFT: {
        x_offset = -1;
        y_offset = 0;
        break;
    }
    case PieceDirection::RIGHT: {
        x_offset = 1;
        y_offset = 0;
        break;
    }
    case PieceDirection::DOWN: {
        x_offset = 0;
        y_offset = 1;
        break;
    }

    }
    Point newPoint;
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        newPoint.x = piece.points[i].x + x_offset;
        newPoint.y = piece.points[i].y + y_offset;
        newCoords[i] = newPoint;
    }
    Tetranimo movedPiece;
    memcpy(movedPiece.points, newCoords, sizeof(movedPiece.points));
    movedPiece.pivot = newCoords[1];
    movedPiece.shape = piece.shape;
    movedPiece.type = piece.type;
    return movedPiece;
}


Tetranimo rotatePiece(Tetranimo piece, Board board, bool clockwise) {
    Tetranimo rotatedPiece;
    Point relativeToPivot[4];
    Point* result = getPointsRelativeToPivot(piece.points, piece.pivot);
    for (int i = 0; i < 4; i++) {
        relativeToPivot[i] = result[i];
    }
    Point rotatedPoints[TETROMINO_POINTS];
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        Point rotatedPoint;
        int vector[2] = { relativeToPivot[i].x, relativeToPivot[i].y };
        int rotationMatrix[2][2];
        memcpy(rotationMatrix, clockwise ? ROTATION_MATRIX_90 : ROTATION_MATRIX_270, sizeof(rotationMatrix));
        int* rotatedVector = matrixVectorProduct2(vector, rotationMatrix);
        rotatedPoint.x = rotatedVector[0] + piece.pivot.x;
        rotatedPoint.y = rotatedVector[1] + piece.pivot.y;
        rotatedPoints[i] = rotatedPoint;
    }
    memcpy(rotatedPiece.points, rotatedPoints, sizeof(rotatedPiece.points));
    rotatedPiece.shape = piece.shape;
    rotatedPiece.type = piece.type;
    rotatedPiece.pivot = rotatedPoints[1];
    
    return rotatedPiece;
}

Tetranimo forcePieceDown(Tetranimo piece, Board board)
{
    Tetranimo droppedPiece = piece;
    while (!checkCollision(movePiece(droppedPiece, PieceDirection::DOWN).points,board)) {
        droppedPiece = movePiece(droppedPiece, PieceDirection::DOWN);
    }
    return droppedPiece;
}


void drawPiece(Tetranimo piece, Board board) {
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        board[piece.points[i].y][piece.points[i].x] = piece.type == Type::ACTIVE ? '#' : '~';
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
    tetranimo.shape = shapeIndex;

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
    int rowsBroken = 0;

    int* rowCheck;
    int i;
    for (i = 0, (rowCheck = scanCompletedRow(game->board)); i < 4 && *rowCheck > 0; i++, rowCheck++)
    {
        breakCompletedRow(game, *rowCheck);
        rowsBroken++;
        if (topRowBroken == 0)
            topRowBroken = *rowCheck;
        *rowCheck = 0;
    }

    if (topRowBroken > 0)
        dropRows(game, topRowBroken, rowsBroken);
}

/* dropRows: drops remaining placed blocks down after completed rows
 * are broken. Takes the top row broken and number of rows broken as parameters */
void dropRows(Game* game, int topRowBroken, int rowsBroken)
{
    int i, j, k;

    for (j = topRowBroken - 1; j > 0; j--)
    {
        for (i = 1; i < BOARD_WIDTH - 1; i++)
        {
            if (game->board[j][i] == '*')
            {
                //Erase the current square
                game->board[j][i] = '.';

                //re-draw the block at that location
                game->board[j + rowsBroken][i] = '*';


            }


        }
    }
}

/* holdPiece: Holds the Active Piece so that the player can use it again */
void holdPiece(Game* game)
{
    Tetranimo temp;

    //If there isn't a piece already held
    if (game->pieceIsHeld == FALSE)
    {
        // set the heldPiece to activePiece;
        game->heldPiece = game->activePiece;

        eraseActivePiece(&game->activePiece, game->board);

        //set the activePiece flag to false
        game->pieceIsActive = FALSE;

        game->pieceIsHeld = TRUE;
    }

    //If a piece is held already
    else if (game->pieceIsHeld == TRUE)
    {
        //erase the activePiece
        eraseActivePiece(&game->activePiece, game->board);

        //store the activePiece in a temp variable
        temp = game->activePiece;

        //set the activePiece to heldPiece
        game->activePiece = game->heldPiece;

        //set the heldPiece to the temp
        game->heldPiece = temp;

        //set the new activePiece's position back to it's starting position
        //game->activePiece.points = SHAPES[game->activePiece.shape];

        memcpy(game->activePiece.points, SHAPES[game->activePiece.shape], sizeof(game->activePiece.points));

        drawPiece(game->activePiece, game->board);

    }
}

void handleGravity(Game* game) {
    game->level = 1 + game->totalLinesCleared / 1;
    if (game->framesUntilNextDrop == 0) {
        Tetranimo droppedPiece = movePiece(game->activePiece, PieceDirection::DOWN);
        //If you collided with something falling due to gravity, you ran out of time so the game should place the piece for you.
        if (checkCollision(droppedPiece.points, game->board)) {
            placeActivePiece(game);
            //game->pieceIsActive = false;
        }
        else {
            game->activePiece = droppedPiece;
        }
        game->framesUntilNextDrop = INITIAL_GRAVITY - game->level;
    }
    game->framesUntilNextDrop--;
}

void updateGhostPiece(Game* game) {
    Tetranimo updatedGhostPiece = game->activePiece;
    updatedGhostPiece = forcePieceDown(updatedGhostPiece, game->board);
    updatedGhostPiece.type = Type::GHOST;
    game->drawGhostPiece = true;
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        int activePieceY = game->activePiece.points[i].y;
        for (int j = 0; j < TETROMINO_POINTS; j++) {
            if (updatedGhostPiece.points[j].y == activePieceY) {
                game->drawGhostPiece = false;
            }
        }
    }
    game->ghostPiece = updatedGhostPiece;
}



#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "TetrisUtils.h"
#include <windows.h> /* for GetAsyncKeyState */
#include <conio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "Texture.h"


const int SCREEN_WIDTH = 300; //This it the dimension of the board. this is a hack for now.
const int SCREEN_HEIGHT = 575;

//Starts up SDL and creates a window
bool initGfx();
//bool loadMedia(Texture* texture);
//Frees media and shuts down SDL
void close();

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

//Sprites
SDL_Rect spriteClips[7];

int main(int argc, char* argv[]) {

    Game game = initialize();
    PlayerAction playerAction = PlayerAction::IDLE;

    struct timeb frameStart, frameEnd;
    ftime(&frameStart);
    int frameTimeDiff;

    srand((unsigned)time(NULL));

    //Start up SDL and create window
    if (!initGfx())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        Texture boardTile(gRenderer);
        Texture spriteSheet(gRenderer);

        //TODO: MOVE TO A FUNCTION. CANT MOVE TO HEADER B/C TEXTURE NEEDS RENDERER
        //Sprite Rectangles

        /* Line */
        spriteClips[0].x = 1;
        spriteClips[0].y = 249;
        spriteClips[0].w = 16;
        spriteClips[0].h = 16;

        /* Square */
        spriteClips[1].x = 145;
        spriteClips[1].y = 305;
        spriteClips[1].w = 16;
        spriteClips[1].h = 16;

        /* J */
        spriteClips[2].x = 9;
        spriteClips[2].y = 177;
        spriteClips[2].w = 16;
        spriteClips[2].h = 16;

        /* L */
        spriteClips[3].x = 9;
        spriteClips[3].y = 129;
        spriteClips[3].w = 16;
        spriteClips[3].h = 16;

        /* S */
        spriteClips[4].x = 25;
        spriteClips[4].y = 305;
        spriteClips[4].w = 16;
        spriteClips[4].h = 16;

        /* T */
        spriteClips[5].x = 9;
        spriteClips[5].y = 65;
        spriteClips[5].w = 16;
        spriteClips[5].h = 16;

        /* Z */
        spriteClips[6].x = 153;
        spriteClips[6].y = 241;
        spriteClips[6].w = 16;
        spriteClips[6].h = 16;

        //Load media
        if (!boardTile.loadFromFile("Board_Tile.png"))
        {
            printf("Failed to load media! Tetris Board\n");
        }
        if (!spriteSheet.loadFromFile("Tetris_Sprites.png"))
        {
            printf("Failed to load media! Tetris Board\n");
        }
        else
        {

            SDL_Event event;

            while (game.state != GameState::OVER) {
                //Handle events on queue
                while (SDL_PollEvent(&event) != 0)
                {
                    //User requests quit
                    if (event.type == SDL_QUIT)
                    {
                        game.state = GameState::OVER;
                    }
                    else if (event.type == SDL_KEYDOWN) {
                        switch (event.key.keysym.sym)
                        {
                        case SDLK_ESCAPE:
                            playerAction = PlayerAction::QUIT;
                            break;

                        case SDLK_DOWN:
                            playerAction = PlayerAction::MOVE_DOWN;
                            break;

                        case SDLK_LEFT:
                            playerAction = PlayerAction::MOVE_LEFT;
                            break;

                        case SDLK_RIGHT:
                            playerAction = PlayerAction::MOVE_RIGHT;
                            break;


                        case SDLK_UP:
                            playerAction = PlayerAction::ROTATE_RIGHT;
                            break;

                        case SDLK_z:
                            playerAction = PlayerAction::ROTATE_LEFT;
                            break;

                        case SDLK_SPACE:
                            playerAction = PlayerAction::FORCE_DOWN;
                            break;

                        case SDLK_c:
                            playerAction = PlayerAction::HOLD;
                            break;

                        default:

                            break;
                        }
                    }
                }

                ftime(&frameEnd);
                frameTimeDiff = (int)1000 * (frameEnd.time - frameStart.time) + (frameEnd.millitm - frameStart.millitm);

                //TODO: Should this be handled in update instead?
                if (frameTimeDiff > FRAME_RATE) {

                    update(playerAction, &game);
                    draw(&game);
                    ftime(&frameStart);
                    playerAction = PlayerAction::IDLE;
                }


                //Clear screen
                SDL_RenderClear(gRenderer);

                //TODO do this is drawBoard                
                for (int i = 0; i < BOARD_HEIGHT; i++) {
                    for (int j = 0; j < BOARD_WIDTH; j++) {
                        if (game.board[i][j].occupyingPiece == TetranimoType::EMPTY)
                            boardTile.render((j * 16), (i * 16), 0);
                        else
                            spriteSheet.render((j * 16), (i * 16), &spriteClips[game.board[i][j].occupyingPiece]);

                    }
                }


                //Update screen
                SDL_RenderPresent(gRenderer);

            }
            //TODO Free somewhere else?
            boardTile.free();

            spriteSheet.free();
        }
    }

    teardown();


    return 0;

}

bool initGfx()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }

        //Create window
        gWindow = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {


                //Initialize renderer color
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags))
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }

    return success;
}
void close()
{
    //Destroy window    
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}


/*This function initializes the necissary Game state and Board. */
Game initialize() {
    clearScreen();
    Game game;
    game.level = 1;
    game.score = 0;
    game.totalLinesCleared = 0;
    game.pieceIsActive = false;
    game.pieceIsHeld = false;
    game.state = GameState::PLAYING;
    game.framesUntilNextDrop = INITIAL_GRAVITY;
    //This line is so that the held piece appears correctly as None instead of Line
    game.heldPiece.type = EMPTY;
    std::queue<Tetranimo> pieceQueue;
    for (int i = 0; i < PIECE_QUEUE_SIZE; i++) {
        Tetranimo queuedPiece = spawnTetranimo();
        queuedPiece.state = TetranimoState::QUEUED;
        pieceQueue.push(queuedPiece);
    }
    game.upcomingPieces = pieceQueue;
    //fill in the board.
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {

            game.board[i][j].oldBoardImg = '.';
            game.board[i][j].occupyingPiece = TetranimoType::EMPTY;

        }
    }
    int test = 2;
    return game;
}

//TODO: Seperate update into smaller more sensible functions?
void update(PlayerAction playerAction, Game* game) {

    if (!game->pieceIsActive) {
        game->activePiece = game->upcomingPieces.front();
        game->upcomingPieces.pop();
        Tetranimo nextQueuedPiece = spawnTetranimo();
        nextQueuedPiece.state == TetranimoState::QUEUED;
        game->upcomingPieces.push(nextQueuedPiece);
        spawnActivePiece(game);
        game->activePiece.state = TetranimoState::ACTIVE;
        game->pieceIsActive = true;
        updateGhostPiece(game);
        playerAction = PlayerAction::IDLE;
    }

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
    }
    }

    if (checkCollision(movedPiece.points, game->board)) {
        if (playerAction == PlayerAction::MOVE_DOWN) { // You are touching the board or other placed pieces. So place the active piece.
            placeActivePiece(game);
            return;
        }
        //If there was a collision revert the piece movement.
        movedPiece = game->activePiece;
    }


    game->activePiece = movedPiece;
    handleGravity(game);
    updateGhostPiece(game);
    if (game->pieceIsActive)
        drawPiece(game->activePiece, game->board);
    if (game->drawGhostPiece)
        drawPiece(game->ghostPiece, game->board);

}

void draw(Game* game) {

    //system("cls");
    clearScreen();
    printf("Lines:%d\tLevel:%d\nSCORE:%d\n", game->totalLinesCleared, game->level, game->score);

    Shape held;
    switch (game->heldPiece.type)
    {
    case LINE:
        memcpy(held, SHAPES[LINE], sizeof(held));
        break;
    case SQUARE:
        memcpy(held, SHAPES[SQUARE], sizeof(held));
        break;
    case J:
        memcpy(held, SHAPES[J], sizeof(held));
        break;
    case L:
        memcpy(held, SHAPES[L], sizeof(held));
        break;
    case S:
        memcpy(held, SHAPES[S], sizeof(held));
        break;
    case T:
        memcpy(held, SHAPES[T], sizeof(held));
        break;
    case Z:
        memcpy(held, SHAPES[Z], sizeof(held));
        break;
    default:
        memcpy(held, SHAPES[EMPTY], sizeof(held));
        break;
    }
    Shape queued;
    TetranimoType nextPieceShape = game->upcomingPieces.front().type;
    memcpy(queued, SHAPES[nextPieceShape], sizeof(queued));

    //NOTE: THIS IS FUCKING RETARDED.
    printf("Held Piece:\t\t Next Piece:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%c", held[i][j]);
        }
        printf("            \t\t");
        for (int k = 0; k < 4; k++) {
            printf("%c", queued[i][k]);

        }
        printf("\n");
    }



    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            printf("%c", game->board[i][j]);
        }
        printf("\n");
    }



}

void teardown() {

    //Free resources and close SDL


    /*for (int i = 0; i < 20; i++) {
        printf("\n\n\n\n\n");
    }*/
    //clearScreen();
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
    movedPiece.type = piece.type;
    movedPiece.state = piece.state;
    movedPiece.sprite = piece.sprite;
    return movedPiece;
}

/* Rotates a piece 90 degrees clockwise or anti-clockwise. Each piece has a list of coords on the board and a 'pivot value'.
   To rotate we get the coordinated relative to the pivot and transform them by a rotation matrix. Then return a piece with the
   new rotated coordinates. */
Tetranimo rotatePiece(Tetranimo piece, Board board, bool clockwise) {
    Tetranimo rotatedPiece;
    //Get the relative points to the pivot.
    Point* relativeToPivot = getPointsRelativeToPivot(piece.points, piece.pivot);
    Point rotatedPoints[TETROMINO_POINTS];

    //Generate the rotated points.
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        Point rotatedPoint;
        int vector[2] = { relativeToPivot[i].x, relativeToPivot[i].y };
        int rotationMatrix[2][2];
        memcpy(rotationMatrix, clockwise ? ROTATION_MATRIX_90 : ROTATION_MATRIX_270, sizeof(rotationMatrix));
        //TODO this might be bad? Might be using a pointer which isn't allocated on the heap!
        int* rotatedVector = matrixVectorProduct2(vector, rotationMatrix);
        rotatedPoint.x = rotatedVector[0] + piece.pivot.x;
        rotatedPoint.y = rotatedVector[1] + piece.pivot.y;
        rotatedPoints[i] = rotatedPoint;
    }

    free(relativeToPivot);

    //Now that you have the rotated points, create the 'rotated' piece and return it.
    memcpy(rotatedPiece.points, rotatedPoints, sizeof(rotatedPiece.points));
    rotatedPiece.type = piece.type;
    rotatedPiece.state = piece.state;
    rotatedPiece.pivot = rotatedPoints[1];
    rotatedPiece.sprite = piece.sprite;

    return rotatedPiece;
}

/* Moves a piece as far down to bottom of the board as possible. Used for positioning the ghost pieces right now too!*/
Tetranimo forcePieceDown(Tetranimo piece, Board board)
{
    //TODO: Since we know the dimensions of the board there might be a better way to do this?
    Tetranimo droppedPiece = piece;
    while (!checkCollision(movePiece(droppedPiece, PieceDirection::DOWN).points, board)) { //If the piece being moved down would not cause a collision.
        droppedPiece = movePiece(droppedPiece, PieceDirection::DOWN);
    }
    return droppedPiece;
}


void drawPiece(Tetranimo piece, Board board) {
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        board[piece.points[i].y][piece.points[i].x].occupyingPiece = piece.type;
    }
}

/* erasePiece: Erases a piece from the board */
void eraseActivePiece(Tetranimo* piece, Board board)
{
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        board[piece->points[i].y][piece->points[i].x].occupyingPiece = TetranimoType::EMPTY;
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
        if (board[col][row].occupyingPiece != TetranimoType::EMPTY) {
            return true;
        }
    }
    return false;
}

Tetranimo spawnTetranimo() {
    int shapeIndex;
    shapeIndex = rand() % 7;
    Tetranimo tetranimo;
    memcpy(tetranimo.points, STARTING_COORDS[shapeIndex], sizeof(tetranimo.points));
    tetranimo.pivot = tetranimo.points[1];
    tetranimo.type = (TetranimoType)shapeIndex;
    tetranimo.sprite = shapeIndex;

    return tetranimo;

}

/* placePiece: places a piece onto the board */
void placeActivePiece(Game* game)
{
    Tetranimo activePiece = game->activePiece;
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        if (activePiece.points[i].y == 0 && game->board[activePiece.points[i].x][0].occupyingPiece != TetranimoType::EMPTY) {
            game->state = GameState::OVER;
            return;
        }
    }
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        int row = activePiece.points[i].y;
        int col = activePiece.points[i].x;
        game->board[row][col].occupyingPiece = game->activePiece.type;
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
            if (board[j][i].occupyingPiece != TetranimoType::EMPTY)
                k++;

        }
        if (k == BOARD_WIDTH)
            rows[rowIndex++] = j;
    }

    return rows;

}

void breakCompletedRow(Game* game, int row)
{
    int i;
    for (i = 0; i < BOARD_WIDTH; i++)
        game->board[row][i].occupyingPiece = TetranimoType::EMPTY;
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
    int i, j;

    /* NOTE: j = (topRowBroken - 1) b/c the loop checks every row ABOVE the
     * top broken row. The top broken row should be empty because it
     * has been broken. */
    for (j = topRowBroken - 1; j > 0; j--)
    {
        for (i = 0; i < BOARD_WIDTH; i++)
        {
            TetranimoType currentSquare = game->board[j][i].occupyingPiece;

            if (currentSquare != TetranimoType::EMPTY)
            {
                //Keep track of what used to be there.
                TetranimoType temp = currentSquare;

                //Erase the current square
                game->board[j][i].occupyingPiece = TetranimoType::EMPTY;

                //re-draw the block at the row it was on + the number of rows broken
                game->board[j + rowsBroken][i].occupyingPiece = temp;


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

        memcpy(game->activePiece.points, STARTING_COORDS[game->activePiece.type], sizeof(game->activePiece.points));

        drawPiece(game->activePiece, game->board);

    }
}

/*The game needs to automatically move the active piece down And place it if the player runs out of time to move it.
  This function moves the piece down if it needs to and updates the state of the gravity.*/
void handleGravity(Game* game) {
    game->level = 1 + game->totalLinesCleared / 1;
    if (game->framesUntilNextDrop == 0) {
        Tetranimo droppedPiece = movePiece(game->activePiece, PieceDirection::DOWN);
        //If you collided with something falling due to gravity, you ran out of time so the game should place the piece for you.
        if (checkCollision(droppedPiece.points, game->board)) {
            placeActivePiece(game);
        }
        else {
            game->activePiece = droppedPiece;
        }
        game->framesUntilNextDrop = INITIAL_GRAVITY - game->level;
    }
    game->framesUntilNextDrop--;
}
/* A ghost piece is a indicator of where the piece would land if the player forced it down.
   This function keeps the ghost piece in the correct position relative to the active piece */
void updateGhostPiece(Game* game) {
    Tetranimo updatedGhostPiece = game->activePiece;
    updatedGhostPiece = forcePieceDown(updatedGhostPiece, game->board);
    updatedGhostPiece.state = TetranimoState::GHOST;
    game->drawGhostPiece = true;

    /*If any of the real active piece would cover the ghost piece, drawing the ghost piece is no longer useful!*/
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

void clearScreen() {
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

}

/* drawBoard: Renders the appropraite textures to the SDL Window based on the current state of the board */
void drawBoard(Game* game)
{
}



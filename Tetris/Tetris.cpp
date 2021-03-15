#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "TetrisUtils.h"
#include <windows.h> /* for GetAsyncKeyState */
#include <conio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <SDL_ttf.h>
#include "Gfx.cpp"
#include <set>


/* animation stuff */
int rowBreakAnimationFlag[4] = {-1, -1, -1, -1 };
void animateRowBreak(int);
void drawAnimation(Game*, int);
#define ANIMATION_LENGTH 2000 / 30 /* I don't know what this number means */
int animationTime = 0;
bool animationPlaying = FALSE;

//NOT GOOD.
Texture boardTile;
Texture spriteSheet;
Texture ghostSpriteSheet;
Texture background;
Texture boardBorder;
Texture heldBox;
Texture scoreBox;
Texture nextPiecesBox;
Texture levelBox;
Texture scoreFont;

//Also not good.
UIComponent UI_HeldBox;
UIComponent UI_ScoreBox;
UIComponent UI_LevelBox;
UIComponent UI_NextPiecesBox;

Texture whiteSquare;

int main(int argc, char* argv[]) {


	srand((unsigned)time(NULL));

	Game game = initialize();
	PlayerAction playerAction = PlayerAction::IDLE;

	struct timeb frameStart, frameEnd;
	ftime(&frameStart);
	int frameTimeDiff;

	//Start up SDL and create window
	if (!Gfx::initGfx())
	{
		printf("Failed to initialize!\n");
		exit(-1);
	}

	loadMedia();

	SDL_Event event;

	/* Game Loop */
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
				playerAction = getAction(event);
			}
		}

		ftime(&frameEnd);
		frameTimeDiff = (int)1000 * (frameEnd.time - frameStart.time) + (frameEnd.millitm - frameStart.millitm);

		if (frameTimeDiff > FRAME_RATE) {
            
            //Clear screen
			Gfx::clearRenderer(gRenderer);


			//Draw things
			drawUI(game);
			drawBoard(game);


            //Draw an animation if there is one to draw
            drawAnimation(&game, frameTimeDiff);
            
			//Update Game state (unless an animation is playing)
            if(!animationPlaying)
                update(playerAction, &game);




			//Present what renderer drew
			Gfx::renderPresent(gRenderer);

            
			ftime(&frameStart);

            playerAction = PlayerAction::IDLE;
		}
	}
	/*End of Game loop*/

	//TODO: I'm leaking a lot memory right now by not freeing textures NOT GOOD.
	teardown();

	return 0;

}

PlayerAction getAction(SDL_Event event) {

	PlayerAction action;

	switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
		action = PlayerAction::QUIT;
		break;

	case SDLK_DOWN:
		action = PlayerAction::MOVE_DOWN;
		break;

	case SDLK_LEFT:
		action = PlayerAction::MOVE_LEFT;
		break;

	case SDLK_RIGHT:
		action = PlayerAction::MOVE_RIGHT;
		break;


	case SDLK_UP:
		action = PlayerAction::ROTATE_RIGHT;
		break;

	case SDLK_z:
		action = PlayerAction::ROTATE_LEFT;
		break;

	case SDLK_SPACE:
		action = PlayerAction::FORCE_DOWN;
		break;

	case SDLK_c:
		action = PlayerAction::HOLD;
		break;

	default:
		action = PlayerAction::IDLE;
		break;
	}
	return action;
}

void loadMedia() {
	//TODO: handle textures in a better way than this probably.
	boardTile = Gfx::loadTextureFromFile("BoardTile2.png");
	spriteSheet = Gfx::loadTextureFromFile("Tetris_Sprites.png");
	ghostSpriteSheet = Gfx::loadTextureFromFile("Tetris_Ghost_Sprites.png");
	background = Gfx::loadTextureFromFile("Tetris_BG.jpg");
	boardBorder = Gfx::loadTextureFromFile("BGBorder.png");
	heldBox = Gfx::loadTextureFromFile("Hold.png");
	scoreBox = Gfx::loadTextureFromFile("Score.png");
	nextPiecesBox = Gfx::loadTextureFromFile("NextSmall.png");
	levelBox = Gfx::loadTextureFromFile("Level.png");

	//Initialize the UI_Compenents with their textures and positions
	UI_HeldBox = { heldBox , scoreHoldViewPort.w - 100, CENTER(scoreHoldViewPort.h,BOARD_PIXEL_HEIGHT) , heldBox.w, heldBox.h };
	UI_ScoreBox = { scoreBox , UI_HeldBox.x, UI_HeldBox.y + heldBox.h + 5, scoreBox.w, scoreBox.h };
	UI_NextPiecesBox = { nextPiecesBox , 5, CENTER(SCREEN_HEIGHT , BOARD_PIXEL_HEIGHT) + 24, nextPiecesBox.w, nextPiecesBox.h };
	UI_LevelBox = { levelBox , UI_HeldBox.x, UI_ScoreBox.y + scoreBox.h + 5, levelBox.w, levelBox.h };

	whiteSquare = Gfx::loadTextureFromFile("White_Square.png");
}

/*This function initializes the necissary Game state and Board. */
Game initialize() {
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
			game.board[i][j].occupyingPiece.type = TetranimoType::EMPTY;
		}
	}

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


	erasePiece(&game->activePiece, game->board);
	erasePiece(&game->ghostPiece, game->board);
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
		movedPiece = rotatePiece(game->activePiece, true); //True, because you are rotating clockwise.
		break;
	}
	case PlayerAction::ROTATE_LEFT: {
		movedPiece = rotatePiece(game->activePiece, false); //False, you are rotating anticlockwise
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
		else {
			//If there was a collision revert the piece movement.
			movedPiece = game->activePiece;
		}
	}


	game->activePiece = movedPiece;
	handleGravity(game);
	updateGhostPiece(game);
	if (game->pieceIsActive)
		drawPiece(game->activePiece, game->board);
	if (game->drawGhostPiece)
		drawPiece(game->ghostPiece, game->board);

}

void teardown() {
	//Get rid of the font.
	TTF_CloseFont(gFont);
	gFont = NULL;

	//Free resources and close SDL
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
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
Tetranimo rotatePiece(Tetranimo piece, bool clockwise) {
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
		board[piece.points[i].y][piece.points[i].x].occupyingPiece = piece;
	}
}

/* erasePiece: Erases a piece from the board */
void erasePiece(Tetranimo* piece, Board board)
{
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		board[piece->points[i].y][piece->points[i].x].occupyingPiece.type = TetranimoType::EMPTY;
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
		if (board[col][row].occupyingPiece.type != TetranimoType::EMPTY) {
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
		if (activePiece.points[i].y == 0 && game->board[activePiece.points[i].x][0].occupyingPiece.type != TetranimoType::EMPTY) {
			game->state = GameState::OVER;
			return;
		}
	}
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		int row = activePiece.points[i].y;
		int col = activePiece.points[i].x;
		game->board[row][col].occupyingPiece = game->activePiece;
	}
	//You placed the current piece so set the piece active flag off.
	game->pieceIsActive = false;
	sweepBoard(game);
}


void breakCompletedRow(Game* game, int row)
{
	Tetranimo empty;
	empty.type = TetranimoType::EMPTY;
	int i;

	for (i = 0; i < BOARD_WIDTH; i++)
		game->board[row][i].occupyingPiece = empty;
	game->totalLinesCleared++;
	game->score += 40 * (game->level);

}

/* Checks board for completed rows and delete's them*/
void sweepBoard(Game* game)
{
	int completedRows[4] = { -1, -1, -1, -1 };
	int rowIndex = 0;
	int nonEmptySquares = 0;
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		if (rowCompleted(game->board, row))
        {
            rowBreakAnimationFlag[rowIndex] = row;
			completedRows[rowIndex++] = row;
                
        }
	}

	for (int i = 0; i < 4; i++) {
		if (completedRows[i] != -1) {
			
			
			breakCompletedRow(game, completedRows[i]);
            //NOTE: Moved dropRow() to the animation function
		}
	}
}

/* Starting from a given row, Move every piece in each row above it down one row. This should be called every time a row is completed.*/
void dropRow(Game* game, int row)
{
	
	for (int rowAbove = row - 1; rowAbove > 0; rowAbove--) {
		for (int col = 0; col < BOARD_WIDTH; col++) {
			game->board[rowAbove + 1][col].occupyingPiece = game->board[rowAbove][col].occupyingPiece;

		}
	}
}

bool rowCompleted(Board board, int row) {
	for (int col = 0; col < BOARD_WIDTH; col++) {
		if (board[row][col].occupyingPiece.type == TetranimoType::EMPTY) {
			return false;
		}
	}
	return true;
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

		erasePiece(&game->activePiece, game->board);

		//set the activePiece flag to false
		game->pieceIsActive = FALSE;

		game->pieceIsHeld = TRUE;
	}

	//If a piece is held already
	else if (game->pieceIsHeld == TRUE)
	{
		//erase the activePiece
		erasePiece(&game->activePiece, game->board);

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

void drawUI(Game game) {

	//Render Background
	Gfx::setViewPort(gRenderer, 0);
	Gfx::render(0, 0, background, 0, 0.5);

	//Render Hold Piece Box
	Gfx::setViewPort(gRenderer, &scoreHoldViewPort);
	Gfx::render(UI_HeldBox.x, UI_HeldBox.y, UI_HeldBox.texture); //IDK WTF THIS IS ANY MORE IM JUST TYPING RANDOM NUMBERS!!

	//Render the held piece if there is one
	TetranimoType heldPiece = game.heldPiece.type;
	if (heldPiece != TetranimoType::EMPTY) {

		//We need to offset where we draw the pieces based on their size to keep them centered
		int pieceWidthOffset = fullPieceClips[heldPiece].w / 2;
		int pieceHeightOffset = heldPiece == TetranimoType::LINE ? 0 : fullPieceClips[heldPiece].h / 4;

		//TODO: setAlpha is garbage.
		Gfx::setAlpha(spriteSheet, 0.5);
		Gfx::render(UI_HeldBox.x + (UI_HeldBox.w / 2 - pieceWidthOffset), // Horizontal center of held piece box
			UI_HeldBox.y + (UI_HeldBox.h / 2 - pieceHeightOffset), // Vertical center of held piece box
			spriteSheet,
			&fullPieceClips[heldPiece]);
		Gfx::setAlpha(spriteSheet, 1.0);
	}

	//Render Score Box
	Gfx::render(UI_ScoreBox.x, UI_ScoreBox.y, UI_ScoreBox.texture);

	//TODO: Try to render score with "blended" text mode maybe to make it look less pixelated?

	//Render the score as Text
	char scoreTextBuffer[100];
	sprintf_s(scoreTextBuffer, "%d", game.score);
	SDL_Color textColor = { 0xFF, 0xFF, 0xFF }; //White
	scoreFont = Gfx::loadFromRenderedText(scoreTextBuffer, textColor);

	Gfx::render(UI_ScoreBox.x + UI_ScoreBox.w / 2 - scoreFont.w / 2, //Center text Horizontally in box
		UI_ScoreBox.y + UI_ScoreBox.h / 2 + 4, //Center text Vertically in box
		scoreFont);

	//Render Level Box
	Gfx::render(UI_LevelBox.x, UI_LevelBox.y, UI_LevelBox.texture);

	//Render the level as Text
	char levelTextBuffer[10];
	sprintf_s(levelTextBuffer, "%d", game.level);
	scoreFont = Gfx::loadFromRenderedText(levelTextBuffer, textColor);

	//GOOD FUCKING LORD THIS IS A MESS RIGHT NOW
	Gfx::render(UI_LevelBox.x + (levelBox.w / 2) - (scoreFont.w / 2), //Center text Horizontally in box
		UI_LevelBox.y + levelBox.h / 2 + 10, //Center text Vertically in box
		scoreFont);



	//Render Next Pieces box
	Gfx::setViewPort(gRenderer, &nextPiecesViewPort);
	Gfx::render(UI_NextPiecesBox.x, UI_NextPiecesBox.y, UI_NextPiecesBox.texture); // +24 is because I feel like it looks better further down. IDK why.

	//Render Next Pieces
	std::queue<Tetranimo> nextPieces = game.upcomingPieces;


	Tetranimo nextPiece = nextPieces.front();
	TetranimoType type = nextPiece.type;

	int pieceWidthOffset = fullPieceClips[type].w / 2;
	int pieceHeightOffset = type == TetranimoType::LINE ? 0 : fullPieceClips[type].h / 4;

	Gfx::setAlpha(spriteSheet, .75);
	Gfx::render(UI_NextPiecesBox.x + UI_NextPiecesBox.w / 2 - pieceWidthOffset, // Horizontal center of next piece box
		UI_NextPiecesBox.y + UI_NextPiecesBox.h / 2 - pieceHeightOffset, // Vorizontal center of next piece box
		spriteSheet,
		&fullPieceClips[type]);
	Gfx::setAlpha(spriteSheet, 1.0);

}

/* drawBoard: Renders the appropraite textures to the SDL Window based on the current state of the board */
void drawBoard(Game game)
{
	Gfx::setViewPort(gRenderer, &boardViewPort);

	Gfx::render(0, 0, boardBorder, 0);

	//TODO do this is drawBoard                
	Gfx::setAlpha(boardTile, .5); // this sets the board tiles to ~50% opacity.
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++) {
			Tetranimo piece = game.board[i][j].occupyingPiece;
	
			if (piece.type == TetranimoType::EMPTY)
				Gfx::render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, boardTile); //NOTE: The +5's are to account for the border width. Holy shit this is ugly.
			else if (piece.state == TetranimoState::GHOST)
				Gfx::render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, ghostSpriteSheet, &ghostSpriteClips[piece.type]);
			else
				Gfx::render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, spriteSheet, &spriteClips[piece.type]);

		}
	}


}


/* animateRowBreak: Animates a given row break. */
void animateRowBreak(int row)
{
    //TODO: setAlpha is garbage.
    Gfx::setAlpha(whiteSquare, 0.5);
    for(int i = 0; i < BOARD_WIDTH; i++)
    {
        Gfx::render((i * SQUARE_PIXEL_SIZE) + 5, (row * SQUARE_PIXEL_SIZE) + 5, whiteSquare);
    }    

}

/* drawAnimation: handles animation logic and draws the correct animation. There's only a row breaking animation right now. */
void drawAnimation(Game* game, int frameTimeDiff)
{
    #define MAX_ROWS_BROKEN 4

    //If the animation flag is set
    if(rowBreakAnimationFlag[0] > 0)
    {
        //Loop through the array to see what rows need to be animated
        for(int i = 0; i < MAX_ROWS_BROKEN; i++)
            if(rowBreakAnimationFlag[i] > 0)
                animateRowBreak(rowBreakAnimationFlag[i]); /* call animateRowBreak on a valid row */
        
        //Begin the animation
        if(animationTime == 0 && animationPlaying == FALSE)
        {
            animationTime = ANIMATION_LENGTH;
            animationPlaying = TRUE;
        }
        //If the animation has run its course
        else if(animationTime <= 0 && animationPlaying == TRUE)
        {
            //Set the flags back to 0;
            animationTime = 0;
            animationPlaying = FALSE;

            for(int i = 0; i < MAX_ROWS_BROKEN; i++)
                if(rowBreakAnimationFlag[i] > 0)
                {
                    dropRow(game, rowBreakAnimationFlag[i]); /* call dropRow on the appropriate rows */
                    rowBreakAnimationFlag[i] = -1; /* set the rowBreakAnimationFlag array back to the default -1s */ 
                }
        }
        //If not, keep track of how long the animation has been running
        else if(animationTime > 0)
            animationTime -= frameTimeDiff;
    }


}

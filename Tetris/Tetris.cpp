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
#include <SDL_ttf.h>


const int SCREEN_WIDTH = 600; //These are the dimensions of the BG texture. This is a hack for now.
const int SCREEN_HEIGHT = 565;

const int SQUARE_PIXEL_SIZE = 16;

const int BORDER_PADDING = 10; //There are 10 pixels of padding for the board border.

const int BOARD_PIXEL_WIDTH = BOARD_WIDTH * SQUARE_PIXEL_SIZE + BORDER_PADDING;
const int BOARD_PIXEL_HEIGHT = BOARD_HEIGHT * SQUARE_PIXEL_SIZE + BORDER_PADDING;

const SDL_Rect boardViewPort = { (SCREEN_WIDTH - BOARD_PIXEL_WIDTH) / 2, (SCREEN_HEIGHT - BOARD_PIXEL_HEIGHT) / 2, BOARD_PIXEL_WIDTH, BOARD_PIXEL_HEIGHT }; //This centers the board.
const SDL_Rect scoreHoldViewPort = { 0, 0, ((SCREEN_WIDTH - BOARD_PIXEL_WIDTH) / 2), SCREEN_HEIGHT };
const SDL_Rect nextPiecesViewPort = { ((SCREEN_WIDTH - BOARD_PIXEL_WIDTH) / 2) + BOARD_PIXEL_WIDTH, 0, scoreHoldViewPort.w + boardViewPort.w, SCREEN_HEIGHT };

//These are the positions of the full piece images on the sprite sheet.
SDL_Rect fullPieceClips[7] = { {1, 249, 64, 16}, //Line
									 {145, 305, 32, 32}, //Square
									 {9, 177, 48, 32}, //J
									 {9, 113, 48, 32}, //L
									 {9, 305, 48, 32}, //S
									 {9, 49, 48, 32}, //T
									 {137, 241, 48, 32}, //Z 
};

//These are the positions of individal sprite blocks used to draw on the board.
SDL_Rect spriteClips[7] = { {1, 249, 16, 16}, //Line
							{145, 305, 16, 16}, //Square
							{9, 177, 16, 16}, //J
							{9, 129, 16, 16}, //L
							{25, 305, 16, 16}, //S
							{9, 65, 16, 16}, //T
							{153, 241, 16, 16}, //Z 
};

SDL_Rect ghostSpriteClips[7] = { {1, 249, 16, 16}, //Line
							     {145, 321, 16, 16}, //Square
							     {9, 177, 16, 16}, //J
							     {9, 129, 16, 16}, //L
							     {25, 305, 16, 16}, //S
							     {9, 65, 16, 16}, //T
							     {137, 241, 16, 16}, //Z 
};


//Starts up SDL and creates a window
bool initGfx();
//bool loadMedia(Texture* texture);
//Frees media and shuts down SDL

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

//Globally used font used to generate the texture used to render the score and other stuff.
TTF_Font* scoreFont = NULL;

int main(int argc, char* argv[]) {


	srand((unsigned)time(NULL));

	Game game = initialize();
	PlayerAction playerAction = PlayerAction::IDLE;

	struct timeb frameStart, frameEnd;
	ftime(&frameStart);
	int frameTimeDiff;

	//Start up SDL and create window
	if (!initGfx())
	{
		printf("Failed to initialize!\n");
	}
	else
	{

		scoreFont = TTF_OpenFont("ScoreFont.ttf", 12);
		if (scoreFont == NULL)
		{
			printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		}

		//TODO: handle textures in a better way than this probably.
		Texture boardTile(gRenderer);
		Texture spriteSheet(gRenderer);
		Texture ghostSpriteSheet(gRenderer);
		Texture background(gRenderer);
		Texture boardBorder(gRenderer);
		Texture heldBox(gRenderer);
		Texture scoreBox(gRenderer);
		Texture nextPiecesBox(gRenderer);
		Texture levelBox(gRenderer);
		Texture scoreFont(gRenderer,scoreFont);

		

		//Load media
		if (!boardTile.loadFromFile("BoardTile2.png"))
		{
			printf("Failed to load media! Tetris Board Tile\n");
		}
		if (!spriteSheet.loadFromFile("Tetris_Sprites.png"))
		{
			printf("Failed to load media! Tetris Sprites\n");
		}
		if (!ghostSpriteSheet.loadFromFile("Tetris_Ghost_Sprites.png"))
		{
			printf("Failed to load media! Tetris Sprites\n");
		}
		if (!background.loadFromFile("Tetris_BG.jpg"))
		{
			printf("Failed to load media! Tetris Background\n");
		}
		if (!boardBorder.loadFromFile("BGBorder.png"))
		{
			printf("Failed to load media! Tetris Board Border\n");
		}
		if (!heldBox.loadFromFile("Hold.png"))
		{
			printf("Failed to load media! Tetris Hold Box\n");
		}
		if (!scoreBox.loadFromFile("Score.png"))
		{
			printf("Failed to load media! Tetris Score Box\n");
		}
		if (!nextPiecesBox.loadFromFile("NextSmall.png"))
		{
			printf("Failed to load media! Tetris Next Box\n");
		}
		if (!levelBox.loadFromFile("Level.png"))
		{
			printf("Failed to load media! Tetris Next Box\n");
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

				if (frameTimeDiff > FRAME_RATE) {
					update(playerAction, &game);
					draw(&game);
					ftime(&frameStart);
					playerAction = PlayerAction::IDLE;
				}


				//Clear screen
				SDL_RenderClear(gRenderer);

				//Render Background
				SDL_RenderSetViewport(gRenderer, 0);
				background.render(0, 0, 0, 0.5);
				//Render Hold Piece Box
				SDL_RenderSetViewport(gRenderer, &scoreHoldViewPort);
				heldBox.render(scoreHoldViewPort.w - 100, (scoreHoldViewPort.h - BOARD_PIXEL_HEIGHT) / 2); //IDK WTF THIS IS ANY MORE IM JUST TYPING RANDOM NUMBERS!!

				//Render the held piece if there is one
				TetranimoType heldPiece = game.heldPiece.type;
				if (heldPiece != TetranimoType::EMPTY) {

					//We need to offset where we draw the pieces based on their size to keep them centered
					int pieceWidthOffset = fullPieceClips[heldPiece].w / 2;
					int pieceHeightOffset = heldPiece == TetranimoType::LINE ? 0 : fullPieceClips[heldPiece].h / 4;

					//TODO: setAlpha is garbage.
					spriteSheet.setAlpha(185);
					spriteSheet.render(scoreHoldViewPort.w - 100 + (heldBox.getWidth() / 2 - pieceWidthOffset), // Horizontal center of held piece box
						(scoreHoldViewPort.h - BOARD_PIXEL_HEIGHT) / 2 + (heldBox.getHeight() / 2 - pieceHeightOffset), // Vertical center of held piece box
						&fullPieceClips[heldPiece]);
					spriteSheet.setAlpha(225);
				}

				//Render Score Box
				scoreBox.render(scoreHoldViewPort.w - 100, ((scoreHoldViewPort.h - BOARD_PIXEL_HEIGHT) / 2) + heldBox.getHeight() + 5);

				//TODO: Try to render score with "blended" text mode maybe to make it look less pixelated?

				//Render the score as Text
				char scoreTextBuffer[100];
				sprintf_s(scoreTextBuffer, "%d", game.score);
				SDL_Color textColor = { 0xFF, 0xFF, 0xFF }; //White
				if (!scoreFont.loadFromRenderedText(scoreTextBuffer, textColor))
				{
					printf("Failed to load media! Tetris Next Box\n");
				}

				scoreFont.render(scoreHoldViewPort.w - 100 + scoreBox.getWidth() / 2 - scoreFont.getWidth()/2, //Center text Horizontally in box
					((scoreHoldViewPort.h - BOARD_PIXEL_HEIGHT) / 2) + heldBox.getHeight() + scoreBox.getHeight()/2 + 4); //Center text Vertically in box
					

				//Render Level Box
				levelBox.render(scoreHoldViewPort.w - 100, ((scoreHoldViewPort.h - BOARD_PIXEL_HEIGHT) / 2) + heldBox.getHeight() + scoreBox.getHeight() + 10);

				//Render the level as Text
				char levelTextBuffer[10];
				sprintf_s(levelTextBuffer, "%d", game.level);
				if (!scoreFont.loadFromRenderedText(levelTextBuffer, textColor))
				{
					printf("Failed to load media! Tetris Next Box\n");
				}

				//GOOD FUCKING LORD THIS IS A MESS RIGHT NOW
				scoreFont.render(scoreHoldViewPort.w - 100 + levelBox.getWidth() / 2 - scoreFont.getWidth() / 2, //Center text Horizontally in box
					((scoreHoldViewPort.h - BOARD_PIXEL_HEIGHT) / 2) + heldBox.getHeight() + scoreBox.getHeight() + levelBox.getHeight()/2 + 10 ); //Center text Vertically in box



				//Render Next Pieces box
				SDL_RenderSetViewport(gRenderer, &nextPiecesViewPort);
				nextPiecesBox.render(4, (SCREEN_HEIGHT - BOARD_PIXEL_HEIGHT) / 2 + 24); // +24 is because I feel like it looks better further down. IDK why.

				//Render Next Pieces
				std::queue<Tetranimo> nextPieces = game.upcomingPieces;


				Tetranimo nextPiece = nextPieces.front();
				TetranimoType type = nextPiece.type;

				int pieceWidthOffset = fullPieceClips[type].w / 2;
				int pieceHeightOffset = type == TetranimoType::LINE ? 0 : fullPieceClips[type].h / 4;


				//TODO: setAlpha is garbage.
				spriteSheet.setAlpha(185);
				spriteSheet.render(3 + nextPiecesBox.getWidth() / 2 - pieceWidthOffset, // Horizontal center of next piece box
					((SCREEN_HEIGHT - BOARD_PIXEL_HEIGHT) / 2 + 24) + (nextPiecesBox.getHeight() / 2) - pieceHeightOffset, // Vorizontal center of next piece box
					&fullPieceClips[type]);
				spriteSheet.setAlpha(225);




				SDL_RenderSetViewport(gRenderer, &boardViewPort);

				boardBorder.render(0, 0, 0);
				//TODO do this is drawBoard                
				boardTile.setAlpha(122); // this sets the board tiles to ~50% opacity.
				for (int i = 0; i < BOARD_HEIGHT; i++) {
					for (int j = 0; j < BOARD_WIDTH; j++) {
						Tetranimo piece = game.board[i][j].occupyingPiece;
						if (piece.type == TetranimoType::EMPTY)
							boardTile.render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, 0); //NOTE: The +5's are to account for the border width. Holy shit this is ugly.
						else if(piece.state == TetranimoState::GHOST)
							ghostSpriteSheet.render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, &ghostSpriteClips[piece.type]);
						else
							spriteSheet.render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, &spriteClips[piece.type]);

					}
				}


				//Update screen
				SDL_RenderPresent(gRenderer);

			}
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

				//Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}

	return success;
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

void draw(Game* game) {

}

void teardown() {
	//Get rid of the font.
	TTF_CloseFont(scoreFont);
	scoreFont = NULL;

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
	int i;
	for (i = 0; i < BOARD_WIDTH; i++)
		game->board[row][i].occupyingPiece.type = TetranimoType::EMPTY;
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
			completedRows[rowIndex++] = row;
	}

	for (int i = 0; i < 4; i++) {
		if (completedRows[i] != -1) {
			breakCompletedRow(game, completedRows[i]);
			dropRow(game, completedRows[i]);
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

/* drawBoard: Renders the appropraite textures to the SDL Window based on the current state of the board */
void drawBoard(Game* game)
{
}



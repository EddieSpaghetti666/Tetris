#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include <conio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <SDL_ttf.h>
#include <set>
#include <queue>
#include <SDL_mixer.h>
#include "Tetranimo.h"
#include "Board.h"
#include "TetrisUtils.h"
#include "Sound.h"
#include "Animation.h"
#include "Gfx.cpp"
#include "Game.h"


PlayerAction getAction(SDL_Event);
void loadMedia();
void teardown();
void drawUI(Game);
void drawBoard(Game); 
void drawPiece(Tetranimo piece);


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

Texture whiteSquare;

//Also not good.
UIComponent UI_HeldBox;
UIComponent UI_ScoreBox;
UIComponent UI_LevelBox;
UIComponent UI_NextPiecesBox;


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

		//start chune
		if (!playingMusic())
		{
			setMusicVolume(2); //Turn that racket down!
			//Play the music
			playMusic();
		}
	
		

		ftime(&frameEnd);
		frameTimeDiff = (int)1000 * (frameEnd.time - frameStart.time) + (frameEnd.millitm - frameStart.millitm);

		if (frameTimeDiff > FRAME_RATE) {
            
            //Clear screen
			Gfx::clearRenderer(gRenderer);


			//Draw things
			drawUI(game);
			drawBoard(game);

			
			handleAnimations(game, frameTimeDiff);

			if(!animatingRowBreak())
				update(playerAction, game, frameTimeDiff);

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
	loadSounds();
	
	//TODO: handle textures in a better way than this probably.
	boardTile = Gfx::loadTextureFromFile("BoardTile2.png");
	spriteSheet = Gfx::loadTextureFromFile("Tetris_Sprites.png");
	ghostSpriteSheet = Gfx::loadTextureFromFile("Ghost_Sprites.png");
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



void teardown() {
	//Get rid of the font.
	TTF_CloseFont(gFont);
	gFont = NULL;
	
	freeSounds();
	Gfx::freeTextures();

	//Free resources and close SDL
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	Mix_Quit();
}


/* holdPiece: Holds the Active Piece so that the player can use it again */
void holdPiece(Game* game)
{
	playSFX(SFX::PIECE_HOLD);
	Tetranimo temp;

	//If there isn't a piece already held
	if (!game->pieceIsHeld)
	{
		// set the heldPiece to activePiece;
		game->heldPiece = game->activePiece;

		erasePiece(&game->activePiece, game->board);

		//set the activePiece flag to false
		game->pieceIsActive = false;

		game->pieceIsHeld = true;

	}

	//If a piece is held already
	else
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

		updatePieceBoardPosition(game->activePiece, game->board);

	}

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
	scoreFont = Gfx::loadFromRenderedText(scoreTextBuffer, textColor, true);

	Gfx::render(UI_ScoreBox.x + UI_ScoreBox.w / 2 - scoreFont.w / 2, //Center text Horizontally in box
		UI_ScoreBox.y + UI_ScoreBox.h / 2 , //Center text Vertically in box
		scoreFont);

	//Render Level Box
	Gfx::render(UI_LevelBox.x, UI_LevelBox.y, UI_LevelBox.texture);

	//Render the level as Text
	char levelTextBuffer[10];
	sprintf_s(levelTextBuffer, "%d", game.level);
	scoreFont = Gfx::loadFromRenderedText(levelTextBuffer, textColor, true);

	//GOOD FUCKING LORD THIS IS A MESS RIGHT NOW
	Gfx::render(UI_LevelBox.x + (levelBox.w / 2) - (scoreFont.w / 2), //Center text Horizontally in box
		UI_LevelBox.y + (UI_LevelBox.h / 2), //Center text Vertically in box
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

			/* Draw in the placed pieces and empty squares */
	
			if (piece.type == TetranimoType::EMPTY)
				Gfx::render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, boardTile); //NOTE: The +5's are to account for the border width. Holy shit this is ugly.
			else if (piece.state == TetranimoState::ACTIVE || piece.state == TetranimoState::GHOST)
				continue;
			else if (piece.state == TetranimoState::PLACED)
				Gfx::render((j * SQUARE_PIXEL_SIZE) + 5, (i * SQUARE_PIXEL_SIZE) + 5, spriteSheet, &spriteClips[piece.type]);

		}
	}

	//TODO: Move this out?
	
	drawPiece(game.ghostPiece);
	/* Ugly stupid hack because before active piece spawn's locking will be undefined */
	if (game.activePiece.locking && game.activePiece.state == TetranimoState::ACTIVE) {
		double opacity = ((double)game.framesSinceLastDrop / 100.0);
		Gfx::setAlpha(spriteSheet, opacity);
	}
	drawPiece(game.activePiece);
}

void drawPiece(Tetranimo piece) {
	Gfx::setViewPort(gRenderer, &boardViewPort);
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		Point point = piece.points[i];
		if (piece.state == TetranimoState::ACTIVE) {
			Gfx::render((point.x * SQUARE_PIXEL_SIZE) + 5, (point.y * SQUARE_PIXEL_SIZE) + 5, spriteSheet, &spriteClips[piece.type]);
		}
		else if (piece.state == TetranimoState::GHOST) {
			Gfx::render((point.x * SQUARE_PIXEL_SIZE) + 5, (point.y * SQUARE_PIXEL_SIZE) + 5, ghostSpriteSheet, &ghostSpriteClips[piece.type]);
		}
		
		
	}

}



#pragma once
#include <queue>
#include "Tetranimo.h"



typedef enum class GameState {
	PLAYING,
	PAUSED,
	OVER
} gamestate;

typedef struct Square {
	//This seems retarded but I can't think of a better way to do this right now.
	Tetranimo occupyingPiece;
} square;

typedef Square Board[BOARD_HEIGHT][BOARD_WIDTH];

//TODO: MOVE THIS!!!!!!
typedef struct Game {
	GameState state;
	bool pieceIsActive;
	bool drawGhostPiece;
	bool pieceIsHeld;
	int level;
	Board board;
	int score;
	int totalLinesCleared;
	int framesSinceLastDrop;
	int gravity;
	Tetranimo activePiece;
	Tetranimo ghostPiece;
	Tetranimo heldPiece;
	std::queue<Tetranimo> upcomingPieces;
} game;


/*This function initializes the necissary Game state and Board. */
Game initialize();

/* Main function responsible for updating the state of the game */
void update(PlayerAction playerAction, Game& game, int frameTime);

void spawnActivePiece(Game& game);

/* Updates the position a piece on the board */
void updatePieceBoardPosition(Tetranimo piece, Board board);

void erasePiece(Tetranimo* piece, Board board);

/* Update position of Ghost piece which, which indicates where active piece would land if player forced down */
void updateGhostPiece(Game* game);

/* Move pieces automatically if they need to b/c of gravity */
bool handleGravity(Game& game, int frameTime);

/* Moves a piece as far down to bottom of the board as possible. Used for positioning the ghost pieces right now too!*/
Tetranimo forcePieceDown(Tetranimo piece, Board board);








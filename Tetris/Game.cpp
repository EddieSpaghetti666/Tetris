#include "Game.h"
#include "Sound.h"
#include "Animation.h"

//Forward declare functions because code is spaghetti and meatballs
void placeActivePiece(Game& game);
bool checkCollision(Point points[], Board board);
int* completedRows(Board board);
void handleFullRows(Game& game);

void playAnimation(AnimationType type);


Game initialize() {
	Game game;
	game.level = 1;
	game.score = 0;
	game.totalLinesCleared = 0;
	game.pieceIsHeld = false;
	game.state = GameState::PLAYING;
	game.gravity = INITIAL_GRAVITY;
	game.framesSinceLastDrop = 0;
	game.activePiece = spawnTetranimo();
	game.activePiece.state = TetranimoState::ACTIVE;
	game.pieceIsActive = true;
	game.ghostPiece = game.activePiece;
	game.highScore = loadScore();

	//This line is so that the held piece appears correctly as None instead of Line
	game.heldPiece.type = TetranimoType::EMPTY;

	/* Generate Piece Queue */
	std::queue<Tetranimo> pieceQueue;
	for (int i = 0; i < PIECE_QUEUE_SIZE; i++) {
		Tetranimo queuedPiece = spawnTetranimo();
		queuedPiece.state = TetranimoState::QUEUED;
		pieceQueue.push(queuedPiece);
	}
	game.upcomingPieces = pieceQueue;

	/* Initialize the Board */
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++) {
			game.board[i][j].occupyingPiece.type = TetranimoType::EMPTY;
		}
	}

	return game;
}

//TODO: Seperate update into smaller more sensible functions?
void update(PlayerAction playerAction, Game& game, int frameTime) {

	if (!game.pieceIsActive) {
		spawnActivePiece(game);
		updateGhostPiece(&game);
	}

	void holdPiece(Game * game);

	erasePiece(&game.activePiece, game.board);
	erasePiece(&game.ghostPiece, game.board);

	Tetranimo movedPiece = game.activePiece;
	switch (playerAction) {
	case PlayerAction::QUIT: {
		game.state = GameState::OVER;
		return;
	}
	case PlayerAction::MOVE_LEFT: {
		movedPiece = movePiece(game.activePiece, PieceDirection::LEFT);
		break;
	}
	case PlayerAction::MOVE_RIGHT: {
		movedPiece = movePiece(game.activePiece, PieceDirection::RIGHT);
		break;
	}
	case PlayerAction::MOVE_DOWN: {
		movedPiece = movePiece(game.activePiece, PieceDirection::DOWN);
		break;
	}
	case PlayerAction::ROTATE_RIGHT: {
		movedPiece = rotatePiece(game.activePiece, game.board, true); //True, because you are rotating clockwise.
		break;
	}
	case PlayerAction::ROTATE_LEFT: {
		movedPiece = rotatePiece(game.activePiece, game.board, false); //False, you are rotating anticlockwise
		break;
	}
	case PlayerAction::FORCE_DOWN: {
		game.activePiece = forcePieceDown(game.activePiece, game.board);
		placeActivePiece(game);
		playSFX(SFX::HARD_DROP);
		return;
	}
	case PlayerAction::HOLD: {
		holdPiece(&game);
		return;
	}
	}

	if (!checkCollision(movedPiece.points, game.board) && moved(game.activePiece, movedPiece)){
		game.activePiece = movedPiece;
		//play SFX
		switch (playerAction) {
		case PlayerAction::MOVE_DOWN: {
			playSFX(SFX::SOFT_DROP);
			break;
		}

		case PlayerAction::ROTATE_LEFT: {
			playSFX(SFX::ROTATE);
			break;
		}
		case PlayerAction::ROTATE_RIGHT: {
			playSFX(SFX::ROTATE);
			break;
		}
		default: {
			playSFX(SFX::PIECE_MOVE);
			break;
		}
		}
		
	}
	



	handleGravity(game, frameTime);
	if (game.activePiece.locking)
		game.activePiece.lockDelay--;
	updateGhostPiece(&game);
	if (game.pieceIsActive)
		updatePieceBoardPosition(game.activePiece, game.board);
	if (game.drawGhostPiece)
		updatePieceBoardPosition(game.ghostPiece, game.board);

}

/* spawnPiece: spawns a piece centered at the top of the board. It can be any shaped piece */
void spawnActivePiece(Game& game)
{
	/*Take the first piece off the queue */
	game.activePiece = game.upcomingPieces.front();
	game.upcomingPieces.pop();

	/* Create a new Piece to enqueue*/
	Tetranimo nextQueuedPiece = spawnTetranimo();
	nextQueuedPiece.state = TetranimoState::QUEUED;
	game.upcomingPieces.push(nextQueuedPiece);

	/* Put the new active piece on the board*/
	updatePieceBoardPosition(game.activePiece, game.board);

	game.activePiece.state = TetranimoState::ACTIVE;
	game.pieceIsActive = true;
}

/* erasePiece: Erases a piece from the board */
void erasePiece(Tetranimo* piece, Board board)
{
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		board[piece->points[i].y][piece->points[i].x].occupyingPiece.type = TetranimoType::EMPTY;
	}

}


void updatePieceBoardPosition(Tetranimo piece, Board board) {
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		board[piece.points[i].y][piece.points[i].x].occupyingPiece = piece;
	}
}

/* A ghost piece is an indicator of where the piece would land if the player forced it down.
This function keeps the ghost piece in the correct position relative to the active piece */
void updateGhostPiece(Game * game) {
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

/*The game needs to automatically move the active piece down And place it if the player runs out of time to move it.
  This function moves the piece down if it needs to and updates the state of the gravity.*/
bool handleGravity(Game& game, int frameTime) {
	bool placePiece = false;

	game.level = 1 + game.totalLinesCleared / 10;
	//If you collided with something falling due to gravity, you ran out of time so the game should place the piece for you.
	if (game.framesSinceLastDrop == game.gravity) {

		Tetranimo droppedPiece = movePiece(game.activePiece, PieceDirection::DOWN);
		if (checkCollision(droppedPiece.points, game.board))
			game.activePiece.locking = true;

		if (game.activePiece.lockDelay <= 0)
			placeActivePiece(game);

		else if(!game.activePiece.locking)
			game.activePiece = droppedPiece;

		game.gravity = INITIAL_GRAVITY - game.level;
		game.framesSinceLastDrop = 0;
	}
	game.framesSinceLastDrop++;
	return placePiece;
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

int loadScore() {
	// Save the score to file
	FILE* fscore;
	errno_t error = fopen_s(&fscore, "highscore.txt", "r");
	if (error != 0) {
		printf("Couldn't open highscore.txt!");
		return 0;
	}
	int score = 0;
	fscanf_s(fscore, "%d", &score);
	fclose(fscore);
	return score;
}

void saveScore(int score) {
	// Save the score to file
	FILE* fscore;
	errno_t error = fopen_s(&fscore, "highscore.txt", "w+");
	if (fscore == NULL) {
		printf("Couldn't open highscore.txt!");
		return;
	}
	fprintf(fscore, "%d", score);
	fclose(fscore);
}

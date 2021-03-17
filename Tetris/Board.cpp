#include "Board.h"
#include "Game.h"
#include "TetrisUtils.h"
#include "Animation.h"
#include "Sound.h"

bool rowCompleted(Board board, int row) {
	for (int col = 0; col < BOARD_WIDTH; col++) {
		if (board[row][col].occupyingPiece.type == TetranimoType::EMPTY) {
			return false;
		}
	}
	return true;
}

void breakCompletedRow(Game& game, int row)
{
	Tetranimo empty;
	empty.type = TetranimoType::EMPTY;
	int i;

	for (i = 0; i < BOARD_WIDTH; i++)
		game.board[row][i].occupyingPiece = empty;

	game.totalLinesCleared++;
	game.score += 40 * (game.level);

}

int* completedRows(Board board) {
	int rowIndex = 0;
	int* rowsCompleted = (int*)malloc(sizeof(int) * 4);
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		if (rowCompleted(board, row))
		{
			rowsCompleted[rowIndex] = row;
			rowIndex++;
		}
	}
	return rowsCompleted;

}


/* placePiece: places a piece onto the board */
void placeActivePiece(Game& game)
{
	Tetranimo activePiece = game.activePiece;
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		if (activePiece.points[i].y == 0 && game.board[activePiece.points[i].x][0].occupyingPiece.type != TetranimoType::EMPTY) {
			game.state = GameState::OVER;
			return;
		}
	}
	/* Put the piece on the board */
	game.activePiece.state = TetranimoState::PLACED;
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		int row = activePiece.points[i].y;
		int col = activePiece.points[i].x;
		game.board[row][col].occupyingPiece = game.activePiece;
	}
	//You placed the current piece so set the piece active flag off.
	game.pieceIsActive = false;

	int* rows = completedRows(game.board);
	if (rows[0] >= 0) {
		playAnimation(AnimationType::ROW_BREAK);
	}
	free(rows);
}

void handleFullRows(Game& game) {
	int* rows = completedRows(game.board);
	int i = 0;
	for (int i = 0; i < 4; i++) {
		if (rows[i] < 0)
			break;
		breakCompletedRow(game, rows[i]);
		dropRow(game.board, rows[i]);
	}
	free(rows);
	//play sound effect
	playSFX(SFX::ROW_CLEAR);
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

/* Starting from a given row, Move every piece in each row above it down one row. This should be called every time a row is completed.*/
void dropRow(Board& board, int row)
{

	for (int rowAbove = row - 1; rowAbove > 0; rowAbove--) {
		for (int col = 0; col < BOARD_WIDTH; col++) {
			board[rowAbove + 1][col].occupyingPiece = board[rowAbove][col].occupyingPiece;

		}
	}
}

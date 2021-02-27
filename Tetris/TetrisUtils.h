#pragma once
static const int FRAME_RATE = 1000 / 2;
static const int BOARD_WIDTH = 12;
static const int BOARD_HEIGHT = 21;
static const int TETROMINO_WIDTH = 4; //Maximum width of a tetromino.
static const int TETROMINO_HEIGHT = 4; //Maximum height of a tetromino.
static const short Z_KEY = 0x5A;

void swap(int[TETROMINO_WIDTH][TETROMINO_HEIGHT], int, int, int, int);


/* Takes the transpose, of a shape matrix. Used for rotating the tetanimoes */
static void transposeOfTetranimoShapeMatrix(int matrix[TETROMINO_WIDTH][TETROMINO_HEIGHT]) {
	for (int i = 0; i < TETROMINO_WIDTH; i++) {
		for (int j = 0; j < i; j++) {
			swap(matrix, i, j, j, i);
		}
	}

}

static void printTetranimoShape(int shape[TETROMINO_WIDTH][TETROMINO_HEIGHT]) {
	for (int i = 0; i < TETROMINO_WIDTH; i++) {
		for (int j = 0; j < TETROMINO_HEIGHT; j++) {
			printf("%d", shape[i][j]);
		}
		printf("\n");
	}
}
/* Swap values in a tetranimo shape matrix. Useful when rotating them and maybe somewhere else idk. */
static void swap(int matrix[TETROMINO_WIDTH][TETROMINO_HEIGHT], int origin_i, int origin_j, int dest_i, int dest_j) {
	int temp = matrix[origin_i][origin_j];
	matrix[origin_i][origin_j] = matrix[dest_i][dest_j];
	matrix[dest_i][dest_j] = temp;
}

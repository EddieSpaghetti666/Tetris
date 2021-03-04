#pragma once
static const int FRAME_RATE = 1000 / 30;
static const int BOARD_WIDTH = 12;
static const int BOARD_HEIGHT = 21;
static const int TETROMINO_WIDTH = 3; //Maximum width of a tetromino.
static const int TETROMINO_HEIGHT = 3; //Maximum height of a tetromino.
static const short Z_KEY = 0x5A;
static const int NUMBER_OF_3X3_SHAPES = 5;
static const int NUMBER_OF_4X4_SHAPES = 2;
static const int TETRAMINO_STARTING_XPOS = 5;
static const int INITIAL_GRAVITY = 15;

typedef int shape[TETROMINO_WIDTH][TETROMINO_HEIGHT];

const shape SHAPES[7] = {
    {1,1,0, //Square
     1,1,0,
     0,0,0,
     },

     {1,1,1, //Line
     0,0,0,
     0,0,0,
     },

    {0,1,0, //T
     1,1,1,
     0,0,0,
     },

    {0,0,1, //L
     1,1,1,
     0,0,0,
     },

   {1,0,0, //J
    1,1,1,
    0,0,0,
    },

   {0,1,1, //S
    1,1,0,
    0,0,0,
    },

    {1,1,0, //Z
     0,1,1,
     0,0,0,
     },
};



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

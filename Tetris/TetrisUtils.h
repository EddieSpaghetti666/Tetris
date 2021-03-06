#pragma once
static const int FRAME_RATE = 1000 / 30;
static const int BOARD_WIDTH = 12;
static const int BOARD_HEIGHT = 21;
static const int TETROMINO_POINTS = 4;
static const short Z_KEY = 0x5A;
static const short C_KEY = 0x43;
static const int TETRAMINO_STARTING_XPOS = 5;
static const int INITIAL_GRAVITY = 15;

#define bool char

typedef struct {
    int x;
    int y;
} Point;

typedef Point StartingPos[4];

const StartingPos SHAPES[7] = {
    { {5,0}, {5,1}, {5,2}, {5,3} }, // Line
    { {5,0}, {6,0}, {5,1}, {6,1} }, // Square
    { {4,0}, {4,1}, {5,1}, {6,1} }, // J
    { {4,1}, {5,1}, {6,1}, {6,0} }, // L
    { {4,1}, {5,1}, {5,0}, {6,0} }, // S
    { {5,0}, {5,1}, {4,1}, {6,1} }, // T
    { {4,0}, {5,0}, {5,1}, {6,1} }  // Z
};

typedef char Board[BOARD_HEIGHT][BOARD_WIDTH];

typedef enum class Type {
    ACTIVE,
    GHOST,
    QUEUED
};

typedef struct
{
    int shape;
    Type type;
    Point points[TETROMINO_POINTS];
    Point pivot;

} Tetranimo;

typedef enum tetranimoes {
    SQUARE,
    LINE,
    T,
    L,
    J,
    S,
    Z,
} Tetranimoes;

typedef enum class PlayerAction {
    IDLE,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_DOWN,
    ROTATE_RIGHT,
    ROTATE_LEFT,
    FORCE_DOWN,
    QUIT,
    HOLD
};

typedef enum class PieceDirection {
    LEFT,
    RIGHT,
    DOWN
};

typedef enum class GameState {
    PLAYING,
    PAUSED,
    OVER
};

typedef struct {
    GameState state;
    bool pieceIsActive;
    bool pieceIsHeld;
    int level;
    Board board;
    int score;
    int totalLinesCleared;
    int framesUntilNextDrop;
    Tetranimo activePiece;
    Tetranimo ghostPiece;
    Tetranimo heldPiece;
} Game;

Game initialize();
void update(PlayerAction, Game*);
void draw(Game*);
void teardown();
void drawPiece(Tetranimo*, Board);
void eraseActivePiece(Tetranimo*, Board);
bool checkCollision(Point[], Board);
void placeActivePiece(Game*);
void sweepBoard(Game*);
void spawnActivePiece(Game*);
void moveActivePiece(Game*, PieceDirection);
void rotateActivePiece(Tetranimo*, Board, bool);
void forceActivePieceDown(Game*);
void dropRows(Game*, int, int);
Tetranimo spawnTetranimo();

Point* getPointsRelativeToPivot(Point points[], Point pivot) {
    int pivot_x = pivot.x;
    int pivot_y = pivot.y;
    Point relativePoints[TETROMINO_POINTS];
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        Point relativePoint;
        relativePoint.x = points[i].x - pivot_x;
        relativePoint.y = points[i].y - pivot_y;
        relativePoints[i] = relativePoint;
    }
    return relativePoints;
}

int dotProduct2(int vector1[2], int vector2[2]) {
    int product = 0;
    for (int i = 0; i < 2; i++) {
        product += vector1[i] * vector2[i];
    }
    return product;
}

int* matrixVectorProduct2(int vector[2], int matrix[2][2]) {
    int result[2];
    for (int i = 0; i < 2; i++) {
        int element = dotProduct2(vector, matrix[i]);
        result[i] = element;
    }
    return result;
}



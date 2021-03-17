#pragma once
#include <string>


static const int FRAME_RATE = 1000 / 60;
static const int INITIAL_GRAVITY = 45;
static const int PIECE_QUEUE_SIZE = 3;
static const int TETROMINO_POINTS = 4;


static const int ROTATION_MATRIX_90[2][2] = { 0, -1,
                                              1, 0 };
static const int ROTATION_MATRIX_270[2][2] = { 0, 1,
                                              -1, 0 };

static const int BOARD_WIDTH = 10;
static const int BOARD_HEIGHT = 20;


enum class PlayerAction {
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

enum class PieceDirection {
    LEFT,
    RIGHT,
    DOWN
};


typedef struct Point {
    int x;
    int y;
} point;



static Point* getPointsRelativeToPivot(Point points[], Point pivot) {
    int pivot_x = pivot.x;
    int pivot_y = pivot.y;
    Point* relativePoints = (Point*)malloc(sizeof(Point)*TETROMINO_POINTS);
    for (int i = 0; i < TETROMINO_POINTS; i++) {
        Point relativePoint;
        relativePoint.x = points[i].x - pivot_x;
        relativePoint.y = points[i].y - pivot_y;
        relativePoints[i] = relativePoint;
    }
    return relativePoints;
}

static int dotProduct2(int vector1[2], int vector2[2]) {
    int product = 0;
    for (int i = 0; i < 2; i++) {
        product += vector1[i] * vector2[i];
    }
    return product;
}

static int* matrixVectorProduct2(int vector[2], int matrix[2][2]) {
    int result[2];
    for (int i = 0; i < 2; i++) {
        int element = dotProduct2(vector, matrix[i]);
        result[i] = element;
    }
    return result;
}



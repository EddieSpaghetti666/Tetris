#pragma once
#include "TetrisUtils.h"

#define LOCK_DELAY 30

//Forward declerations
enum class PieceDirection;

/* The orientation of a piece relative to it's initial orientation.*/
typedef enum TetranimoOrientation {
    DEFAULT,
    RIGHT, //Rotated 90 Degrees Right
    LEFT, //Rotated 90 Degrees Right
    TWO //Rotated 90 Degrees twice, either derection gices same orientation.
} tetranimoOrientation;

typedef enum TetranimoType {
    LINE,
    SQUARE,
    J,
    L,
    S,
    T,
    Z,
    EMPTY
} tetranimoType;

typedef enum class TetranimoState {
    ACTIVE,
    GHOST,
    QUEUED,
    PLACED
} tetranimoState;

typedef struct
{
    TetranimoType type;
    TetranimoState state;
    Point points[TETROMINO_POINTS];
    Point pivot;
    TetranimoOrientation orientation;
    bool locking;
    int lockDelay;

} Tetranimo;

typedef Point StartingPos[4];

const StartingPos STARTING_COORDS[7] = {
    { {4,0}, {5,0}, {6,0}, {7,0} }, // Line
    { {5,0}, {6,0}, {5,1}, {6,1} }, // Square
    { {4,0}, {4,1}, {5,1}, {6,1} }, // J
    { {4,1}, {5,1}, {6,1}, {6,0} }, // L
    { {4,1}, {5,1}, {5,0}, {6,0} }, // S
    { {5,0}, {5,1}, {4,1}, {6,1} }, // T
    { {4,0}, {5,0}, {5,1}, {6,1} }  // Z
};

/* Compares position of piece to see if it moved */
bool moved(Tetranimo originalPos, Tetranimo newPos);

/* Takes a piece and a direction to move the piece in. Returns a new piece with updated coordinates.*/
Tetranimo movePiece(Tetranimo piece, PieceDirection direction);

/* Rotates a piece 90 degrees */
Tetranimo rotatePiece(Tetranimo piece, bool clockwise);


/* Creates and returns a new tetaneemo */
Tetranimo spawnTetranimo();

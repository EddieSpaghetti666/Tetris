#pragma once
#include "TetrisUtils.h"
#include <map>

#define LOCK_DELAY 30

//Forward declerations
enum class PieceDirection;

typedef struct Square Board[BOARD_HEIGHT][BOARD_WIDTH];

/* The orientation of a piece relative to it's initial orientation.*/
typedef enum class TetranimoOrientation {
    DEFAULT,
    RIGHT, //Rotated 90 Degrees Right
    TWO, //Rotated 90 Degrees twice, either derection gices same orientation.
    LEFT, //Rotated 90 Degrees Left
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

//SRS stuff
typedef std::pair<int, int> Kick;
typedef std::pair<TetranimoOrientation, TetranimoOrientation> Rotation;

//Note these are indexes into the points array that the piece has so we start counting at 0.
const std::map<TetranimoType, int> PIVOT_INDECES = { {TetranimoType::SQUARE, 2}, {TetranimoType::LINE, 2}, {TetranimoType::J, 2}, {TetranimoType::L, 1}, {TetranimoType::S, 1}, {TetranimoType::T, 1}, {TetranimoType::Z, 2} };

/* Compares position of piece to see if it moved */
bool moved(Tetranimo originalPos, Tetranimo newPos);

/* Takes a piece and a direction to move the piece in. Returns a new piece with updated coordinates.*/
Tetranimo movePiece(Tetranimo piece, PieceDirection direction);

/* Rotates a piece 90 degrees */
Tetranimo rotatePiece(Tetranimo piece, Board board, bool clockwise);

bool fixRotation(Tetranimo& piece, Rotation attemptedRotation, Board board, bool clockwise);

/* Creates and returns a new tetaneemo */
Tetranimo spawnTetranimo();

/* Populates SRS Kicks data structure if it's the first time being used */
void generateSRSKicks();

/* Returns new piece orientation given the current orientation and whether you are rotating clockwise or anticlockwise */
TetranimoOrientation getNewOrientation(TetranimoOrientation orientation, bool clockwise);



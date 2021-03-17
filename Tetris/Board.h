#pragma once
#include "Game.h"
/* Checks if a certain row on the board is completed */
bool rowCompleted(Board board, int row);

/* Breaks a given row in the Board */
void breakCompletedRow(Game& game, int row);

/* Returns an array of completed rows */
int* completedRows(Board board);

/* Go through and break all the rows that are currently completed */
void handleFullRows(Game& game);

/* Permenantly puts down the active piece onto the Board */
void placeActivePiece(Game& game);

/* Checks if any points of a piece would collide with the edge of the board or a filled square on the board */
bool checkCollision(Point points[], Board board);

/* Starting from a given row, Move every piece in each row above it down one row. This should be called every time a row is completed.*/
void dropRow(Board& board, int row);

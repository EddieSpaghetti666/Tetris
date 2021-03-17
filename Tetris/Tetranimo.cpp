#include "Tetranimo.h"
#include "TetrisUtils.h"


/* Compares position of piece to see if it moved */
bool moved(Tetranimo originalPos, Tetranimo newPos) {
	for (int i = 0; i < 4; i++) {
		Point original = originalPos.points[i];
		Point dest = newPos.points[i];
		if ((original.x != dest.x) || (original.y != dest.y)) {
			return true;
		}
	}
	return false;
}

/* Takes a piece and a direction to move the piece in. Returns a new piece with updated coordinates.*/
Tetranimo movePiece(Tetranimo piece, PieceDirection direction)
{
	Point newCoords[TETROMINO_POINTS];
	int x_offset = 0;
	int y_offset = 0;
	switch (direction) {
	case PieceDirection::LEFT: {
		x_offset = -1;
		y_offset = 0;
		break;
	}
	case PieceDirection::RIGHT: {
		x_offset = 1;
		y_offset = 0;
		break;
	}
	case PieceDirection::DOWN: {
		x_offset = 0;
		y_offset = 1;
		break;
	}

	}
	Point newPoint;
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		newPoint.x = piece.points[i].x + x_offset;
		newPoint.y = piece.points[i].y + y_offset;
		newCoords[i] = newPoint;
	}
	Tetranimo movedPiece;
	memcpy(movedPiece.points, newCoords, sizeof(movedPiece.points));
	movedPiece.pivot = newCoords[1];
	movedPiece.type = piece.type;
	movedPiece.state = piece.state;
	movedPiece.locking = false;
	movedPiece.lockDelay = LOCK_DELAY;
	return movedPiece;
}

/* Rotates a piece 90 degrees clockwise or anti-clockwise. Each piece has a list of coords on the board and a 'pivot value'.
   To rotate we get the coordinated relative to the pivot and transform them by a rotation matrix. Then return a piece with the
   new rotated coordinates. */
Tetranimo rotatePiece(Tetranimo piece, bool clockwise) {
	Tetranimo rotatedPiece;
	//Get the relative points to the pivot.
	Point* relativeToPivot = getPointsRelativeToPivot(piece.points, piece.pivot);
	Point rotatedPoints[TETROMINO_POINTS];

	//Generate the rotated points.
	for (int i = 0; i < TETROMINO_POINTS; i++) {
		Point rotatedPoint;
		int vector[2] = { relativeToPivot[i].x, relativeToPivot[i].y };
		int rotationMatrix[2][2];
		memcpy(rotationMatrix, clockwise ? ROTATION_MATRIX_90 : ROTATION_MATRIX_270, sizeof(rotationMatrix));
		//TODO this might be bad? Might be using a pointer which isn't allocated on the heap!
		int* rotatedVector = matrixVectorProduct2(vector, rotationMatrix);
		rotatedPoint.x = rotatedVector[0] + piece.pivot.x;
		rotatedPoint.y = rotatedVector[1] + piece.pivot.y;
		rotatedPoints[i] = rotatedPoint;
	}

	free(relativeToPivot);

	//Now that you have the rotated points, create the 'rotated' piece and return it.
	memcpy(rotatedPiece.points, rotatedPoints, sizeof(rotatedPiece.points));
	rotatedPiece.type = piece.type;
	rotatedPiece.state = piece.state;
	rotatedPiece.pivot = rotatedPoints[1];
	rotatedPiece.locking = false;
	rotatedPiece.lockDelay = LOCK_DELAY;

	return rotatedPiece;
}


Tetranimo spawnTetranimo() {
	int shapeIndex;
	shapeIndex = rand() % 7;
	Tetranimo tetranimo;
	memcpy(tetranimo.points, STARTING_COORDS[shapeIndex], sizeof(tetranimo.points));
	tetranimo.pivot = tetranimo.points[1];
	tetranimo.type = (TetranimoType)shapeIndex;


	tetranimo.locking = false;
	tetranimo.lockDelay = LOCK_DELAY;

	return tetranimo;

}

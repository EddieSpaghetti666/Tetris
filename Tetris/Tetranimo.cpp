#include "Tetranimo.h"
#include "TetrisUtils.h"
#include <map>
#include <utility>
#include <vector>
#include "Board.h"
#include <cassert>



//This Data structure holds all the information needed to perform a 'Wall-Kick' or 'Floor-Kick' according to the information on this wiki page about them: https://tetris.wiki/Super_Rotation_System
std::map<TetranimoType, std::map<Rotation, std::vector<Kick>>> SRSKicks;

/*Different Tetranimos have different pivot positions.
  For example: the 'S' piece's 2nd point (going from left->right bottom->top) in it's default orientation is it's pivot 
  the 'Z' piece's pivot is it's 3rd point of it's default orientation.
  consult this chart to see wtf I'm talking about, this explanation sucked : https://tetris.wiki/images/thumb/1/17/SRS-true-rotations.png/300px-SRS-true-rotations.png */

//Note these are indexes into the points array that the piece has so we start counting at 0.
const std::map<TetranimoType, int> PIVOT_INDECES = { {TetranimoType::SQUARE, 2}, {TetranimoType::LINE, 2}, {TetranimoType::J, 2}, {TetranimoType::L, 1}, {TetranimoType::S, 1}, {TetranimoType::T, 1}, {TetranimoType::Z, 2} };


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

	//TODO: Instead of doing this I should probably just memcpy the whole thing and only change the points.
	movedPiece.type = piece.type;
	movedPiece.state = piece.state;
	movedPiece.locking = false;
	movedPiece.lockDelay = LOCK_DELAY;
	movedPiece.orientation = piece.orientation;
	movedPiece.pivot = newCoords[PIVOT_INDECES.find(movedPiece.type)->second];


	return movedPiece;
}

/* Rotates a piece 90 degrees clockwise or anti-clockwise. Each piece has a list of coords on the board and a 'pivot value'.
   To rotate we get the coordinated relative to the pivot and transform them by a rotation matrix. Then return a piece with the
   new rotated coordinates. */
Tetranimo rotatePiece(Tetranimo piece, Board board, bool clockwise) {
	if (piece.type == TetranimoType::SQUARE) {
		//Squares don't rotate
		return piece;
	}
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

	//Create the newly rotated piece, reset the locking Delay because you have succesfully moved it!
	memcpy(rotatedPiece.points, rotatedPoints, sizeof(rotatedPiece.points));
	rotatedPiece.type = piece.type;
	rotatedPiece.state = piece.state;
	rotatedPiece.locking = false;
	rotatedPiece.lockDelay = LOCK_DELAY;
	rotatedPiece.pivot = rotatedPoints[PIVOT_INDECES.find(rotatedPiece.type)->second];
	rotatedPiece.orientation = piece.orientation;


	TetranimoOrientation newOrientation = getNewOrientation(piece.orientation, clockwise);

	if (checkCollision(rotatedPiece.points, board)) {
		//If the rotation failed, try and fix according to the SRS specifications
		Rotation attemptedRotation = { piece.orientation, newOrientation };
		if (fixRotation(rotatedPiece, attemptedRotation, board, clockwise)) {
			return rotatedPiece;
		}
		else {
			// The rotation couldn't be fixed, so don't rotate it at all!
			return piece;
		}
	}
	else {
		//The rotation is good and doesn't need any kicks! assign it's new orientation and return the rotated piece
		rotatedPiece.orientation = newOrientation;
		return rotatedPiece;
	}
}

bool fixRotation(Tetranimo& piece, Rotation attemptedRotation, Board board, bool clockwise) {
	if (SRSKicks.empty()) {
		generateSRSKicks();
	}
	std::vector<Kick> possibleKicks = SRSKicks[piece.type][attemptedRotation];
	//Try all possible kicks and see if any of them work
	for (int i = 0; i < possibleKicks.size(); i++) {
		Point newPoints[TETROMINO_POINTS];
		Kick attempt = possibleKicks[i];
		for (int j = 0; j < TETROMINO_POINTS; j++) {
			Point original = piece.points[j];
			Point newPoint = { original.x + attempt.first, original.y + attempt.second };
			newPoints[j] = newPoint;
		}
		if (!checkCollision(newPoints, board)) {
			//Success! you found kick which puts the piece back in bounds! assign these new points to the piece and update it's pivot and orientation.
			memcpy(piece.points, newPoints, sizeof(piece.points));
			piece.pivot = newPoints[PIVOT_INDECES.find(piece.type)->second];
			piece.orientation = attemptedRotation.second;
			return true;
		}
	}
	//If you didn't find anything the rotation is impossible.
	return false;
}


Tetranimo spawnTetranimo() {
	int shapeIndex;
	shapeIndex = rand() % 7;
	Tetranimo tetranimo;
	memcpy(tetranimo.points, STARTING_COORDS[shapeIndex], sizeof(tetranimo.points));
	tetranimo.type = (TetranimoType)shapeIndex;

	tetranimo.orientation = TetranimoOrientation::DEFAULT;
	tetranimo.locking = false;
	tetranimo.lockDelay = LOCK_DELAY;
	tetranimo.pivot = tetranimo.points[PIVOT_INDECES.find(tetranimo.type)->second];

	return tetranimo;

}

void generateSRSKicks() {
	std::map<Rotation, std::vector<Kick>> nonLinePieceKicks;

	/* Default Orientation -> 90 Deg Right */
	std::vector<Kick> kicks = { Kick{0,0}, Kick{-1,0}, Kick{-1,1}, Kick{0,-2}, Kick{-1,-2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::DEFAULT, TetranimoOrientation::RIGHT}, kicks });

	/* 90 Degrees Right -> Default Orientation */
	kicks = { Kick{0,0}, Kick{1,0}, Kick{1,-1}, Kick{0,2}, Kick{1,2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::RIGHT, TetranimoOrientation::DEFAULT}, kicks });

	/* 90 Degrees Right -> 2 Rotates */
	kicks = { Kick{0,0}, Kick{1,0}, Kick{1,-1}, Kick{0,2}, Kick{1,2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::RIGHT, TetranimoOrientation::TWO}, kicks });

	/* 2 Rotates -> 90 Degrees Right */
	kicks = { Kick{0,0}, Kick{-1,0}, Kick{-1,1}, Kick{0,-2}, Kick{-1,-2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::TWO, TetranimoOrientation::RIGHT}, kicks });

	/* 2 Rotates -> 90 Degrees Left */
	kicks = { Kick{0,0}, Kick{1,0}, Kick{1,1}, Kick{0,-2}, Kick{1,-2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::TWO, TetranimoOrientation::LEFT}, kicks });

	/* 90 Degrees Left -> 2 Rotates */
	kicks = { Kick{0,0}, Kick{-1,0}, Kick{-1,-1}, Kick{0,2}, Kick{-1,2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::LEFT, TetranimoOrientation::TWO}, kicks });

	/* 90 Degrees Left -> Default Orientation */
	kicks = { Kick{0,0}, Kick{-1,0}, Kick{-1,-1}, Kick{0,2}, Kick{-1,2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::LEFT, TetranimoOrientation::DEFAULT}, kicks });

	/* Default Orientation -> 90 Degrees Left */
	kicks = { Kick{0,0}, Kick{1,0}, Kick{1,1}, Kick{0,-2}, Kick{1,-2} };
	nonLinePieceKicks.insert({ Rotation{TetranimoOrientation::DEFAULT, TetranimoOrientation::LEFT}, kicks });

	/* Add these to SRS kick data structure for all pieces except LINES, because those are special */
	for (int type = TetranimoType::SQUARE; type <= TetranimoType::Z; type++) {
		if (type == TetranimoType::LINE)
			continue;
		SRSKicks[(TetranimoType)type] = nonLinePieceKicks;
	}

	std::map<Rotation, std::vector<Kick>> linePieceKicks;

	/* Default Orientation -> 90 Deg Right */
	kicks = { Kick{0,0}, Kick{-2,0}, Kick{1,0}, Kick{-2,-1}, Kick{1,2} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::DEFAULT, TetranimoOrientation::RIGHT}, kicks });

	/* 90 Degrees Right -> Default Orientation */
	kicks = { Kick{0,0}, Kick{2,0}, Kick{-1,0}, Kick{2,1}, Kick{-1,-2} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::RIGHT, TetranimoOrientation::DEFAULT}, kicks });

	/* 90 Degrees Right -> 2 Rotates */
	kicks = { Kick{0,0}, Kick{-1,0}, Kick{2,0}, Kick{-1,2}, Kick{2,-1} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::RIGHT, TetranimoOrientation::TWO}, kicks });

	/* 2 Rotates -> 90 Degrees Right */
	kicks = { Kick{0,0}, Kick{1,0}, Kick{-2,0}, Kick{1,-2}, Kick{-2,1} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::TWO, TetranimoOrientation::RIGHT}, kicks });

	/* 2 Rotates -> 90 Degrees Left */
	kicks = { Kick{0,0}, Kick{2,0}, Kick{-1,0}, Kick{2,1}, Kick{-1,-2} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::TWO, TetranimoOrientation::LEFT}, kicks });

	/* 90 Degrees Left -> 2 Rotates */
	kicks = { Kick{0,0}, Kick{-2,0}, Kick{1,0}, Kick{-2,1}, Kick{1,2} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::LEFT, TetranimoOrientation::TWO}, kicks });

	/* 90 Degrees Left -> Default Orientation */
	kicks = { Kick{0,0}, Kick{1,0}, Kick{-2,0}, Kick{1,-2}, Kick{-2,1} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::LEFT, TetranimoOrientation::DEFAULT}, kicks });

	/* Default Orientation -> 90 Degrees Left */
	kicks = { Kick{0,0}, Kick{-1,0}, Kick{2,0}, Kick{-1,2}, Kick{2,-1} };
	linePieceKicks.insert({ Rotation{TetranimoOrientation::DEFAULT, TetranimoOrientation::LEFT}, kicks });

	SRSKicks[TetranimoType::LINE] = linePieceKicks;
}

TetranimoOrientation getNewOrientation(TetranimoOrientation orientation, bool clockwise) {
	TetranimoOrientation result;
	int currentOrientation = (int)orientation;
	int newOrientation;
	if(clockwise){
		newOrientation = (currentOrientation + 1);
		//There are 4 possible rotations, so if you go past 4 wrap around to start.
		result = newOrientation >= 4 ? TetranimoOrientation::DEFAULT : (TetranimoOrientation)newOrientation;
	}
	else { //AntiClockwise
		newOrientation = (currentOrientation - 1);
		//There are 4 possible rotations, so if you go below 0 we wrap around to the end.
		result = newOrientation < 0 ? TetranimoOrientation::LEFT : (TetranimoOrientation)newOrientation;
	}
		
	return result;
}

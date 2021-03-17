#pragma once
#include "Game.h"

#define ROW_BREAK_LENGTH 3000 / 30 /* I don't know what this number means */
#define TETRANIMO_LOCK_LENGTH 1000/15

enum class AnimationType {
	ROW_BREAK
};

typedef struct Animation {
	int time;
	bool playing;
	AnimationType type;
} animation;


/* Add an animation to the list of playing animations */
void playAnimation(AnimationType type);

/* Update the state of all currently running animations and turn them off if you need to */
void handleAnimations(Game& game, int frameDiff);

/* Animate a row break */
void animateRowBreak(int rows[]);

/* Returns whether or not a row break is in progress. Used because row breaks should be 'blocking' */
bool animatingRowBreak();

#pragma once
#include "Game.h"
#include "Gfx.h"

#define ROW_BREAK_LENGTH 5 /* I don't know what this number means */
#define SINGLE_ANIM_LENGTH 30
#define DOUBLE_ANIM_LENGTH 40
#define TRIPLE_ANIM_LENGTH 50
#define TETRIS_ANIM_LENGTH 100

enum class AnimationType {
	ROW_BREAK,
	SINGLE,
	DOUBLE,
	TRIPLE,
	TETRIS
};

typedef struct Animation {
	int time;
	bool playing;
	AnimationType type;
	int xPos;
	int yPos;
} animation;


/* Add an animation to the list of playing animations */
void playAnimation(AnimationType type);

/* Update the state of all currently running animations and turn them off if you need to */
void handleAnimations(Game& game, int frameDiff);

/* Animate a row break */
void animateRowBreak(int rows[]);

/* Returns whether or not a row break is in progress. Used because row breaks should be 'blocking' */
bool animatingRowBreak();

void animateUpCummies(Animation* animation);

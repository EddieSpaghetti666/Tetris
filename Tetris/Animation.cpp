#include "Animation.h"
#include "Game.h"
#include <map>
#include "Tetranimo.h"
#include "Board.h"
#include "Gfx.h"


const std::map<AnimationType, int> animationLengths = { {AnimationType::ROW_BREAK, ROW_BREAK_LENGTH} };

/* List of currently running animations */
std::vector<Animation> animations;

// TODO: Remove Foward Declaration 
void drawPiece(Tetranimo piece);


void playAnimation(AnimationType type) {
    Animation animation = { animationLengths.find(type)->second , false, type };
    animations.push_back(animation);
}

void handleAnimations(Game& game, int frameDiff) {
   
    for (unsigned int i = 0; i < animations.size(); i++) {
        Animation* animation = &animations.at(i);

        /* If the animation isn't playing yet, start it*/
        if (!animation->playing) {
            animation->playing = true;
        }
        /* If the animation is playing and has run it's course remove it from list */
        else if (animation->playing && animation->time <= 0) {
            animation->playing = false;
            animations.erase(animations.begin() + i);
        }

        switch (animation->type) {
        case AnimationType::ROW_BREAK: {
            if (!animation->playing && animation->time <= 0) {
                //If the animation is over, tell the board to delete the rows.
                handleFullRows(game);
            }
            else if (animation->playing) {
                int* rows = completedRows(game.board);
                animateRowBreak(rows);
                free(rows);
            }
            break;
        }
        }
        animation->time -= frameDiff;
    }

}

void animateRowBreak(int rows[]) {
    Gfx::setAlpha(whiteSquare, .5);
    for (int i = 0; i < 4; i++) {
        int row = rows[i];
        if (row < 0)
            break;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            Gfx::render((j * SQUARE_PIXEL_SIZE) + 5, (row * SQUARE_PIXEL_SIZE) + 5, whiteSquare);
        }
    }
}

bool animatingRowBreak() {
    for (unsigned int i = 0; i < animations.size(); i++) {
        if (animations.at(i).type == AnimationType::ROW_BREAK) {
            return true;
        }
    }
    return false;
}





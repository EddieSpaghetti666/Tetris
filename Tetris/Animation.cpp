#include "Animation.h"
#include "Game.h"
#include <map>
#include "Tetranimo.h"
#include "Board.h"
//#include "Gfx.h"


const std::map<AnimationType, int> animationLengths = { {AnimationType::ROW_BREAK, ROW_BREAK_LENGTH}, {AnimationType::SINGLE, SINGLE_ANIM_LENGTH}, {AnimationType::DOUBLE, DOUBLE_ANIM_LENGTH}, 
    {AnimationType::TRIPLE, TRIPLE_ANIM_LENGTH}, {AnimationType::TETRIS, TETRIS_ANIM_LENGTH} };

/* List of currently running animations */
std::vector<Animation> animations;

// TODO: Remove Foward Declaration 
void drawPiece(Tetranimo piece);


void playAnimation(AnimationType type) {
    
    
    //assign random initial x and y pos for upcummies animations
    int xPos = 100 + rand() % 300; //between 100-300
    int yPos = 150 + rand() % 300; //between 150-450

    Animation animation = { 0 , false, type, xPos, yPos };
    animations.push_back(animation);

    //TODO there is some weird visual bug with multiple animations playing in succession that seems random and hard to replicate. Maybe switching to a double buffer system would fix this?
    
}

void handleAnimations(Game& game, int frameDiff) {
   
    for (unsigned int i = 0; i < animations.size(); i++) {
        Animation* animation = &animations.at(i);

        /* If the animation isn't playing yet, start it*/
        if (!animation->playing) {
            animation->playing = true;
        }
        /* If the animation is playing and has run it's course remove it from list */
        else if (animation->playing && animation->time > animationLengths.find(animation->type)->second) {
            animation->playing = false;
            animations.erase(animations.begin() + i);
        }



        /*Play the animation*/
        switch (animation->type) {
        case AnimationType::ROW_BREAK: {
            if (!animation->playing) {
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
        case AnimationType::SINGLE:
        case AnimationType::DOUBLE:
        case AnimationType::TRIPLE:
        case AnimationType::TETRIS: {
            if (animation->playing) {
                animateUpCummies(animation);
                break;
            }
        }

           
        }
        /*Update the running time of the animation*/
        animation->time++;
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

void animateUpCummies(Animation* animation) {
    
    Gfx::setViewPort(0);
  
    Texture animationTexture;
    double scale = 0;

    //TODO use hashtable?
    switch (animation->type) {
    case AnimationType::SINGLE: {
        animationTexture = singleAnim;
        scale = 0.4;
        break;
    }
    case AnimationType::DOUBLE: {
        animationTexture = doubleAnim;
        scale = 0.5;
        break;
    }
    case AnimationType::TRIPLE: {
        animationTexture = tripleAnim;
        scale = 0.6;
        break;
    }
    case AnimationType::TETRIS: {
        animationTexture = tetrisAnim; 
        scale = 0.8;
        break;
    }
    default:
        return; //OOPS SHOUDLN'T HAVE GOTTEN HERE
    }
    double opacity = 1.0 - (double)animation->time / 100;
    Gfx::setAlpha(animationTexture, opacity);
    animation->xPos;
    animation->yPos++;
    Gfx::render(animation->xPos, animation->yPos, animationTexture, 0, scale);
}





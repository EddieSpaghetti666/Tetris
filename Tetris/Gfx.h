#pragma once
#include <SDL_image.h>

#define CENTER(x,y) (x - y)/2

const int SCREEN_WIDTH = 600; //These are the dimensions of the BG texture. This is a hack for now.
const int SCREEN_HEIGHT = 565;

typedef struct Texture {
	SDL_Texture* texture;
	int w;
	int h;
} texture;

extern Texture boardTile;
extern Texture spriteSheet;
extern Texture ghostSpriteSheet;
extern Texture background;
extern Texture boardBorder;
extern Texture heldBox;
extern Texture scoreBox;
extern Texture nextPiecesBox;
extern Texture levelBox;
extern Texture scoreFont;

extern Texture whiteSquare;

typedef struct UIComponent {
	Texture texture;
	int x;
	int y;
	int w;
	int h;
} uiComponent;

const int SQUARE_PIXEL_SIZE = 16;

const int BORDER_PADDING = 10; //There are 10 pixels of padding for the board border.

const int BOARD_PIXEL_WIDTH = BOARD_WIDTH * SQUARE_PIXEL_SIZE + BORDER_PADDING;
const int BOARD_PIXEL_HEIGHT = BOARD_HEIGHT * SQUARE_PIXEL_SIZE + BORDER_PADDING;

const SDL_Rect boardViewPort = { CENTER(SCREEN_WIDTH, BOARD_PIXEL_WIDTH) , CENTER(SCREEN_HEIGHT, BOARD_PIXEL_HEIGHT), BOARD_PIXEL_WIDTH, BOARD_PIXEL_HEIGHT }; //This centers the board.
const SDL_Rect scoreHoldViewPort = { 0, 0, (CENTER(SCREEN_WIDTH , BOARD_PIXEL_WIDTH)), SCREEN_HEIGHT };
const SDL_Rect nextPiecesViewPort = { (CENTER(SCREEN_WIDTH , BOARD_PIXEL_WIDTH)) + BOARD_PIXEL_WIDTH, 0, scoreHoldViewPort.w + boardViewPort.w, SCREEN_HEIGHT };

//These are the positions of the full piece images on the sprite sheet.
const SDL_Rect fullPieceClips[7] = { {1, 249, 64, 16}, //Line
									 {145, 305, 32, 32}, //Square
									 {9, 177, 48, 32}, //J
									 {9, 113, 48, 32}, //L
									 {9, 305, 48, 32}, //S
									 {9, 49, 48, 32}, //T
									 {137, 241, 48, 32}, //Z 
};

//These are the positions of individal sprite blocks used to draw on the board.
const SDL_Rect spriteClips[7] = { {1, 249, 16, 16}, //Line
							{145, 305, 16, 16}, //Square
							{9, 177, 16, 16}, //J
							{9, 129, 16, 16}, //L
							{25, 305, 16, 16}, //S
							{9, 65, 16, 16}, //T
							{153, 241, 16, 16}, //Z 
};

const SDL_Rect ghostSpriteClips[7] = { {1, 249, 16, 16}, //Line
								 {145, 321, 16, 16}, //Square
								 {9, 177, 16, 16}, //J
								 {9, 129, 16, 16}, //L
								 {25, 305, 16, 16}, //S
								 {9, 65, 16, 16}, //T
								 {137, 241, 16, 16}, //Z 
};
namespace Gfx{

	bool initGfx();

	Texture loadTextureFromFile(std::string filePath);

	Texture loadFromRenderedText(std::string textureText, SDL_Color textColor);


	//I'm naming this simply 'render' because we are only ever rendering textures
	void render(int x, int y, Texture texture, const SDL_Rect* clip = 0, double scale = 0);


	void setViewPort(SDL_Renderer* renderer, const SDL_Rect* viewPort);

	void setAlpha(Texture& texture, double alpha);


	void renderPresent(SDL_Renderer* renderer);

	void clearRenderer(SDL_Renderer* renderer);

	void freeTextures();

}


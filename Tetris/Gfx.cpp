#pragma once
#include <SDL.h>
#include <string>
#include <SDL_image.h>
#include <SDL_ttf.h>

const int SCREEN_WIDTH = 600; //These are the dimensions of the BG texture. This is a hack for now.
const int SCREEN_HEIGHT = 565;


//Global Window context
SDL_Window* gWindow = NULL;
//Global Renderer context
SDL_Renderer* gRenderer = NULL;

//Globally used font for score and level etc.
TTF_Font* gFont = NULL;

typedef struct Texture {
	SDL_Texture* texture;
	int w;
	int h;
};

namespace Gfx {
	bool initGfx()
	{
		//Initialization flag
		bool success = true;

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}

		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}

		//Create renderer for window
		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
		if (gRenderer == NULL)
		{
			printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}

		//Initialize renderer color
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

		//Initialize PNG loading
		int imgFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imgFlags) & imgFlags))
		{
			printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
			success = false;
		}

		//Initialize SDL_ttf
		if (TTF_Init() == -1)
		{
			printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
			success = false;
		}

		gFont = TTF_OpenFont("ScoreFont.ttf", 12);
		if (gFont == NULL)
		{
			printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
			success = false;
		}

		return success;
	}

	Texture loadTextureFromFile(std::string filePath) {
		Texture result;

		//Create a surface from the given path
		SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
		if (loadedSurface == NULL) {
			printf("Failed to load texture from: %s\n", filePath);
			exit(-1);
		}


		//Generate the Texture from our surface.
		SDL_Texture* loadedTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);

		if (loadedTexture == NULL) {
			printf("Failed to create texture from: %s : %s", filePath, SDL_GetError());
			exit(-1);
		}

		result.texture = loadedTexture;
		//The width and height of our texture are the same as the surface width and height.
		result.w = loadedSurface->w;
		result.h = loadedSurface->h;
		
		SDL_FreeSurface(loadedSurface);


		return result;
	}

	Texture loadFromRenderedText(std::string textureText, SDL_Color textColor)
	{
		Texture result;
		//Render text surface
		SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
		if (textSurface == NULL)
		{
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
			exit(-1);
		}

		//Create texture from surface pixels
		SDL_Texture* loadedTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (loadedTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
			exit(-1);
		}

		result.texture = loadedTexture;
		//The width and height of our texture are the same as the surface width and height.
		result.w = textSurface->w;
		result.h = textSurface->h;

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
		//Return success
		return result;
	}

	//I'm naming this simply 'render' because we are only ever rendering textures
	void render(int x, int y, Texture texture, const SDL_Rect* clip = 0, double scale = 0) {
		int width = texture.w;
		int height = texture.h;

		//If you a clipping from a sheet or scaling the texture, modify them accordingly
		if (clip != 0) {
			width = clip->w;
			height = clip->h;
		}
		if (scale != 0) {
			width *= scale;
			height *= scale;
		}

		SDL_Rect dest = { x, y, width, height };
		SDL_RenderCopy(gRenderer, texture.texture, clip, &dest);
	}


	void setViewPort(SDL_Renderer* renderer, const SDL_Rect* viewPort) {
		if (SDL_RenderSetViewport(renderer, viewPort) != 0) {
			printf("Error setting View Port %s\n", SDL_GetError());
		}
	}

	void setAlpha(Texture& texture, double alpha) {
		if (alpha > 1.0 || alpha < 0) {
			printf("Set texture alpha failed! Use values between 0 - 1 please!\n");
			return;
		}
		//For some reason SDL_SetTextureAlpha takes an int between 0x00 & 0xFF and divides it by 0xFF to set the alpha. 
		//I'm doing it this way instead because thinking of alpha values between 0 - 1 is more intuitive for me.
		int ialpha = alpha * 255;

		SDL_SetTextureAlphaMod(texture.texture, ialpha);
	}

	void renderPresent(SDL_Renderer* renderer) {
		SDL_RenderPresent(renderer);
	}

	void clearRenderer(SDL_Renderer* renderer) {
		SDL_RenderClear(renderer);
	}

}

#pragma once
#include<string>;
#include<SDL.h>
#include<SDL_ttf.h>


class Texture
{
public:
	//Initializes variables
	Texture(SDL_Renderer* renderer);

	//Texture with a font
	Texture(SDL_Renderer* renderer, TTF_Font* font);

	//Deallocates memory
	~Texture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);

	//Deallocates texture
	void free();

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double scale = 1.0);


	int getAlpha();

	void setAlpha(int alpha);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	SDL_Renderer* renderer;

	TTF_Font* font;

	//Image dimensions
	int mWidth;
	int mHeight;

	int mAlpha;
};


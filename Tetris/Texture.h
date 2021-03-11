#pragma once
#include<string>;
#include<SDL.h>


class Texture
{
public:
	//Initializes variables
	Texture(SDL_Renderer* renderer);

	//Deallocates memory
	~Texture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

	//Deallocates texture
	void free();

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double scale = 1.0);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	SDL_Renderer* renderer;

	//Image dimensions
	int mWidth;
	int mHeight;
};


#include "Texture.h"
#include <SDL_image.h>

Texture::Texture(SDL_Renderer* renderer)
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mAlpha = 0;
	this->renderer = renderer;
	
}

Texture::~Texture()
{
	//Deallocate
	free();
}


bool Texture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_FALSE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void Texture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void Texture::render(int x, int y, SDL_Rect* clip, double scale)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	renderQuad.w *= scale;
	renderQuad.h *= scale;

	//Render to screen
	if (SDL_RenderCopy(renderer, mTexture, clip, &renderQuad) != 0) {
		printf("%s", SDL_GetError());
	}
}

int Texture::getWidth()
{
	return mWidth;
}

int Texture::getHeight()
{
	return mHeight;
}

int Texture::getAlpha() {
	return mAlpha;
}

void Texture::setAlpha(int alpha){
	mAlpha = alpha;
	if(mTexture != NULL)
		SDL_SetTextureAlphaMod(mTexture, mAlpha);
}




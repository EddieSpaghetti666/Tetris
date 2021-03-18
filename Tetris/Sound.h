#pragma once
#include <SDL_mixer.h>
#include <string>


enum class SFX {
	ROW_CLEAR,
	HARD_DROP,
	PIECE_HOLD,
	PIECE_MOVE,
	SINGLE,
	DOUBLE,
	TRIPLE,
	TETRIS,
	SOFT_DROP,
	ROTATE
};

void loadSounds();

/* Are you playing music right now?*/
bool playingMusic();

/* Set music volume between 0 - 128 ?*/
void setMusicVolume(int vol);

/* Play music in any channel */
void playMusic(int loops = -1);

/* Set SFX volume between 0 - 128 ? */
void setSFXVolume(SFX effect, int col);

/* Play a sound effect once in nearest channel */
void playSFX(SFX effect);

/* Loads music track from file name */
Mix_Music* loadMusicFromFile(std::string file);

/* Loads SFX from WAV file name */
Mix_Chunk* loadSFXFromWAV(std::string wav);

void freeSounds();
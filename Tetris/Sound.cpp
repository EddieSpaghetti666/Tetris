#include "Sound.h"
#include <map>
#include <string>


// The sound effects that will be used
/*Chune*/
Mix_Music* gMusic = NULL;
Mix_Chunk* SFX_RowClear = NULL;
Mix_Chunk* SFX_HardDrop = NULL;
Mix_Chunk* SFX_PieceHold = NULL;
Mix_Chunk* SFX_PieceMove = NULL;
Mix_Chunk* SFX_Single = NULL;
Mix_Chunk* SFX_Double = NULL;
Mix_Chunk* SFX_Triple = NULL;
Mix_Chunk* SFX_Tetris = NULL;


std::map<SFX, Mix_Chunk*> SOUND_EFFECTS;

void loadSounds() {
	/* Load chune */
	gMusic = loadMusicFromFile("Tetris.mp3");
	/* Load sound effects */
	SFX_RowClear = Mix_LoadWAV("SoundEffects\\SFX_RowClear.wav");
	SFX_PieceMove = Mix_LoadWAV("SoundEffects\\SFX_Move.wav");
	SFX_HardDrop = Mix_LoadWAV("SoundEffects\\SFX_HardDrop.wav");
	SFX_PieceHold = Mix_LoadWAV("SoundEffects\\SFX_Hold.wav");
	SFX_Single = Mix_LoadWAV("SoundEffects\\SFX_Single.wav");
	SFX_Double = Mix_LoadWAV("SoundEffects\\SFX_Double.wav");
	SFX_Triple = Mix_LoadWAV("SoundEffects\\SFX_Triple.wav");
	SFX_Tetris = Mix_LoadWAV("SoundEffects\\SFX_Tetris.wav");
	

	SOUND_EFFECTS.insert({ SFX::ROW_CLEAR, SFX_RowClear });
	SOUND_EFFECTS.insert({ SFX::PIECE_MOVE, SFX_PieceMove });
	SOUND_EFFECTS.insert({ SFX::HARD_DROP, SFX_HardDrop });
	SOUND_EFFECTS.insert({ SFX::PIECE_HOLD, SFX_PieceHold });
	SOUND_EFFECTS.insert({ SFX::SINGLE, SFX_Single });
	SOUND_EFFECTS.insert({ SFX::DOUBLE, SFX_Double });
	SOUND_EFFECTS.insert({ SFX::TRIPLE, SFX_Triple });
	SOUND_EFFECTS.insert({ SFX::TETRIS, SFX_Tetris });

	//Mixing Volume of other SFX
	setSFXVolume(SFX::PIECE_MOVE, 10);
	setSFXVolume(SFX::HARD_DROP, 10);
	setSFXVolume(SFX::PIECE_HOLD, 10);
	setSFXVolume(SFX::ROW_CLEAR, 10);
	setSFXVolume(SFX::SINGLE, 10);
	setSFXVolume(SFX::DOUBLE, 10);
	setSFXVolume(SFX::TRIPLE, 15);
	setSFXVolume(SFX::TETRIS, 20);
}
bool playingMusic() {
	return Mix_PlayingMusic() == 1;
}

void setMusicVolume(int vol) {
	Mix_VolumeMusic(vol);
}

void playMusic(int loops) {
	if (Mix_PlayMusic(gMusic, loops) != 0) {
		printf("Failed to play Music! %s", Mix_GetError());
	}
}

void setSFXVolume(SFX effect, int vol) {
	auto search = SOUND_EFFECTS.find(effect);
	if (search != SOUND_EFFECTS.end()) {
		Mix_VolumeChunk(search->second, vol);
	}
}

void playSFX(SFX effect) {
	auto search = SOUND_EFFECTS.find(effect);
	if (search != SOUND_EFFECTS.end()) {
		Mix_Chunk* sfx = search->second;
		if (Mix_PlayChannel(-1, sfx, 0) == -1) {
			printf("Failed to play sound effect %s", Mix_GetError());
		}
	}
}

Mix_Music* loadMusicFromFile(std::string file) {
	Mix_Music* music = Mix_LoadMUS(file.c_str());
	if (music == NULL) {
		printf("Failed to load Music from %s : %s", file.c_str(), Mix_GetError());
	}
	return music;
}

Mix_Chunk* loadSFXFromWAV(std::string wav) {
	Mix_Chunk* effect = Mix_LoadWAV(wav.c_str());
	if (effect == NULL) {
		printf("Failed to load Music from %s : %s", wav.c_str(), Mix_GetError());
	}
	return effect;
}

void freeSounds() {
	Mix_FreeMusic(gMusic);
	Mix_FreeChunk(SFX_RowClear);
	Mix_FreeChunk(SFX_HardDrop);
	Mix_FreeChunk(SFX_PieceMove);
}
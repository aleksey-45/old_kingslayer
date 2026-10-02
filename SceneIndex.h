#pragma once
#include "Scene.h"
#include "Object.h"
#include "UI.h"


#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <map>
#include <cstring>
#include <array>
#include <vector>
#include <string>

class Game;

class SceneIndex : public Scene{
public:
	const std::string sceneName;
	SceneIndex();
	~SceneIndex();

	void update(float deltaTime) override;
	void render() override;
	void handleEvent(SDL_Event* event) override;
	void init() override;
	void calcCardTemplate();
	void clean() override;

	void loadAnimation();

	SceneStatus currentStatus = SCENE_WAITING;
	int substep = 1;

private:
	Game& game;

	//Mix_Music* bgm;
	std::map<std::string, Mix_Chunk*> sounds;

	Background backgroundImg;

	Sprite title;
	Button quitBtn;
	Button startBtn,honorBtn,settingBtn,infoBtn;
	Card indexCard[4];

	Card loadCard;
	Sprite loadMask;
	std::array<std::array<SDL_Rect, 14>, 5> cardSrcRects;

	const int cardW = 49, cardH = 65;

	float cardPos[4] = { 0,90,180,270 };
	float cardDeg[4] = { 0,90,180,270 };

	float btnCenterX = 0.5f;
	float btnCenterY = 0.7f;
	float btnShiftX = 0.15f;
	float btnShiftY = 0.2f;

	float titleY = 0.2f;
};



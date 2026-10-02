#pragma once
#include "Scene.h"
#include "Object.h"
#include "GameData.h"
#include "UI.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <map>
#include <cstring>
#include <string>

class Game;

class SceneEnd : public Scene {
public:
	const std::string sceneName;
	SceneEnd();
	~SceneEnd();

	void update(float deltaTime) override;
	void render() override;
	void handleEvent(SDL_Event* event) override;
	void init() override;
	void clean() override;

	void loadAnimation();

	SceneStatus currentStatus = SCENE_WAITING;
	int substep = 1;
private:
	Game& game;
	GameData gameData;

	//Mix_Music* bgm;
	std::map<std::string, Mix_Chunk*> sounds;
	Background backgroundImg;
	Button homeBtn,restartBtn;
	Sprite Board;
	TextBox title;

	TextBox bossText;
	TextBox gameInfoText;
	TextBox dataText;
	TextBox mottoText;

	std::vector<std::string> mottos = { u8"111ÕæµÄÂð¸ç£¿",u8"¹Ø×¢ RoFtaCD ß÷£¬¹Ø×¢ RoFtaCD Ð»Ð»ß÷~",u8"Who is Your Daddy£¿" };

	float titleY = 0.2f;

	float bossTextY = 0.4f;
	float gameInfoTextY = 0.45f;
	float dataTextY = 0.2f;
	float mottoTextY = 0.6f;

	float restartBtnY = 0.7f;
	float homeBtnY = 0.85f;
	float btnX = 0.5f;
};



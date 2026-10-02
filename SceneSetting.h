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

class SceneSetting : public Scene {
public:
	const std::string sceneName;
	SceneSetting();
	~SceneSetting();

	void update(float deltaTime) override;
	void render() override;
	void handleEvent(SDL_Event* event) override;
	void init() override;
	void clean() override;

	void loadAnimation();
	void hideAllSettings();

	SceneStatus currentStatus = START;
	int substep = 1;
private:
	Game& game;
	GameData gameData;

	//Mix_Music* bgm;
	std::map<std::string, Mix_Chunk*> sounds;

	Background backgroundImg;

	TextBoxUI settingTitle;
	Menu settingMenu;
	Button langBtn;
	Button imageBtn;
	Button audioBtn;
	Button videoBtn;
	Button backBtn;
	BoardUI settingBoard;

	DropDownList fpsList;
	DropDownList windowList;
	DropDownList rotateList;
	DropDownList crtList;
	DropDownList particleList;


	Slider bgmSlider;
	Slider sfxSlider;
	
	std::vector<ButtonText> langTypes;
	std::map<std::string, std::string> langName = { {"zh_CN",u8"简体中文"},{"en_US",u8"English(US)"},{"downshey_CN",u8"真的吗哥"} };

	Sprite startLoadMask;

	float settingTitleX = 0.5f;
	float settingTitleY = 0.08f;
	float settingBgX = 0.5f;
	float settingBgY = 0.2f;

};


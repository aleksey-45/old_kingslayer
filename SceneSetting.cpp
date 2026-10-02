#include "SceneSetting.h"
#include "Game.h"
#include "SceneIndex.h"

SceneSetting::SceneSetting() : game(Game::getInstance()),sceneName("setting") {
}

SceneSetting::~SceneSetting() {

}

void SceneSetting::update(float deltaTime){

	switch (currentStatus){
	case START:
		switch (substep) {
		case 1:
			loadAnimation();
			for (int i = 0; i < langTypes.size(); ++i) {
				langTypes[i].setDisplay(true);
			}
			substep = 2;
			break;
		case 2:
			startLoadMask.action(deltaTime);
			if (startLoadMask.movements.empty()) substep = 3;
			break;
		case 3:
			currentStatus = SETTING_LANG;
			substep = 1;
			break;
		}
		break;
	case SCENE_WAITING:
		break;
	case TO_HOME: {
		game.updateSetting();
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(0, &mode); // 获取主显示器的显示模式
		if (game.gameSettings.fullscreen) SDL_SetWindowFullscreen(game.getWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);
		else SDL_SetWindowFullscreen(game.getWindow(), 0);
		SDL_SetWindowSize(game.getWindow(), game.gameSettings.screenWidth, game.gameSettings.screenHeight);
		SDL_SetWindowPosition(game.getWindow(), (mode.w - game.gameSettings.screenWidth) / 2, (mode.h - game.gameSettings.screenHeight) / 2);
		SDL_RenderSetViewport(game.getRenderer(), NULL);

		game.gameSettings.save("data/settings.json");
		Mix_PlayChannel(-1, sounds["metal_bars"], 0);
		SDL_Delay(250);
		auto sceneIndex = new SceneIndex();
		game.changeScene(sceneIndex);
		return;
	}
	case SETTING_LANG:
		switch (substep){
		case 1:
			settingTitle.text = game.translations["settingLang"];
			for (int i = 0; i < langTypes.size(); ++i) {
				langTypes[i].setDisplay(true);
			}
			substep = 2;
			break;
		case 2: 
			int i = 0;
			for (auto& k : langName) {
				if (k.first == game.gameSettings.language) langTypes[i].setEnabled(false);
				else langTypes[i].setEnabled(true);
				++i;
			}
			break;
		}
		break;

	case SETTING_AUDIO:
		switch (substep) {
		case 1:
			settingTitle.text = game.translations["settingAudio"];
			bgmSlider.setDisplay(true);
			sfxSlider.setDisplay(true);
			substep = 2;
			break;
		case 2:
			break;
		}
		break;
	case SETTING_VIDEO:
		switch (substep) {
		case 1:
			settingTitle.text = game.translations["settingVideo"];
			fpsList.setDisplay(true);
			rotateList.setDisplay(true);
			particleList.setDisplay(true);
			substep = 2;
			break;
		case 2:
			break;
		}
		break;
	case SETTING_IMAGE:
		switch (substep) {
		case 1:
			settingTitle.text = game.translations["settingImage"];
			windowList.setDisplay(true);
			crtList.setDisplay(true);
			substep = 2;
			break;
		case 2:
			break;
		}
		break;
	}

	backgroundImg.degree -= 0.01;
	if (backgroundImg.degree >= 2 * M_PI) backgroundImg.degree -= 2 * M_PI;
}

void SceneSetting::render(){
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	SDL_Rect backgroundRect = { -10,-windowW * 0.2,windowW + 20,windowW + 20 };
	SDL_Point backgroundCenter = { windowW / 2 ,windowH / 2 - windowW * 0.2 };
	SDL_RenderCopyEx(game.getRenderer(), backgroundImg.texture, NULL, &backgroundRect, cos(backgroundImg.degree) * 0.2, &backgroundCenter, SDL_FLIP_NONE);

	settingBoard.render();
	settingTitle.render(game.getRenderer(), game.textFont,TTF_STYLE_BOLD);
	settingMenu.render();
	backBtn.render();
	for (ButtonText& k : langTypes) {
		k.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
	}

	fpsList.render();
	windowList.render();
	rotateList.render();
	crtList.render();
	particleList.render();

	bgmSlider.render();
	sfxSlider.render();

	if (currentStatus == START) {
		SDL_Rect loadMaskRect = { startLoadMask.position.x,startLoadMask.position.y,startLoadMask.width,startLoadMask.height };
		SDL_Point center = { startLoadMask.width / 2,startLoadMask.height / 2 };
		SDL_RenderCopyEx(game.getRenderer(), startLoadMask.texture, NULL, &loadMaskRect, startLoadMask.degree, &center, SDL_FLIP_NONE);
	}


}

void SceneSetting::handleEvent(SDL_Event* event){
	settingMenu.handleEvent(*event);
	backBtn.handleEvent(*event);
	for (ButtonText& k : langTypes) {
		k.handleEvent(*event);
	}
	fpsList.handleEvent(*event);
	windowList.handleEvent(*event);
	rotateList.handleEvent(*event);
	crtList.handleEvent(*event);
	particleList.handleEvent(*event);

	bgmSlider.handleEvent(*event,game.gameSettings.bgmVolume);
	sfxSlider.handleEvent(*event,game.gameSettings.sfxVolume);
}

void SceneSetting::init() {
	game.initLang(sceneName);
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	sounds["click"] = Mix_LoadWAV("assets/audio/click.mp3");
	sounds["metal_bars"] = Mix_LoadWAV("assets/audio/metal_bars.wav");
	//sounds["wood_hit"] = Mix_LoadWAV("assets/audio/wood_hit.wav");
	//sounds["tap"] = Mix_LoadWAV("assets/audio/tap.mp3");

	// Load Start Mask
	startLoadMask.position = { 0,0 };
	startLoadMask.width = windowW * 1.4f;
	startLoadMask.height = windowW * 1.4f;
	startLoadMask.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/start_load_mask.png");

	backgroundImg.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/background.png");
	SDL_QueryTexture(backgroundImg.texture, NULL, NULL, &backgroundImg.width, &backgroundImg.height);


	settingTitle.init({ windowW * settingTitleX,windowH * settingTitleY }, "", 66 * game.getWindowScale(), { 255,255,255,255 }, { 0,0,0,255 }, { 4,4 });
	settingTitle.text = game.translations["settingLang"];

	int btnW = 64, btnH = 32;
	float btnScale = 3.0f * game.getWindowScale();
	float btnScale2 = 2.5f * game.getWindowScale();
	settingBoard.init(windowW * 0.22, windowH * 0.18, windowW * 0.63, windowH * 0.77, game.getRenderer());
	settingMenu.init(windowW*0.05,windowH*0.15,windowW*0.12,windowH*0.8,game.getRenderer());

	langBtn.init(0, 0, btnW, btnH, btnScale, game.getRenderer());
	langBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/setting/setting_lang_button.png");
	langBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		settingMenu.changeBtn(0);
		hideAllSettings();
		currentStatus = SETTING_LANG;
		substep = 1;
	});
	settingMenu.menuBtn.push_back(&langBtn);
	
	imageBtn.init(0, 0, btnW, btnH, btnScale, game.getRenderer());
	imageBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/setting/setting_image_button.png");
	imageBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		settingMenu.changeBtn(1);
		hideAllSettings();
		currentStatus = SETTING_IMAGE;
		substep = 1;
		});
	settingMenu.menuBtn.push_back(&imageBtn);

	audioBtn.init(0, 0, btnW, btnH, btnScale, game.getRenderer());
	audioBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/setting/setting_audio_button.png");
	audioBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		settingMenu.changeBtn(2);
		hideAllSettings();
		currentStatus = SETTING_AUDIO;
		substep = 1;
		});
	settingMenu.menuBtn.push_back(&audioBtn);

	videoBtn.init(0, 0, btnW, btnH, btnScale, game.getRenderer());
	videoBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/setting/setting_video_button.png");
	videoBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		settingMenu.changeBtn(3);
		hideAllSettings();
		currentStatus = SETTING_VIDEO;
		substep = 1;
		});
	settingMenu.menuBtn.push_back(&videoBtn);
	settingMenu.changeBtn(0);

	backBtn.init(windowW * 0.01, windowH * 0.02, btnW, btnH, 2.0f,game.getRenderer());
	backBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/setting/back_button.png");
	backBtn.setOnClickCallback([&]()->void {
		currentStatus = TO_HOME;
		substep = 1;
	});
	

	int i = 0;
	langTypes.resize(langName.size());
	for (auto& k : langName) {
		ButtonText& x = langTypes[i];
		x.init(windowW * 0.25f + i * 1.5 * btnW * btnScale2, windowH * 0.25f, btnW, btnH, btnScale2, game.getRenderer(), k.second, 24 * game.getWindowScale());
		x.shadow = true;
		x.shadowShift = { 1,1 };
		x.textOffsetY = 3;
		x.setOnClickCallback([&]() -> void{
			Mix_PlayChannel(-1, sounds["click"], 0);
			game.gameSettings.language = k.first;
		});
		x.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/setting/button.png");
		if (k.first == game.gameSettings.language) x.setEnabled(false);
		else x.setEnabled(true);
		++i;
	}

	fpsList.init(windowW * 0.25f, windowH * 0.25f, btnW * 2 * game.getWindowScale(), btnH * 2 * game.getWindowScale(), "FPS", game.getRenderer(), game.textFont);
	fpsList.setTextSize(32 * game.getWindowScale());
	fpsList.addItem("30");
	fpsList.addItem("60");
	fpsList.addItem("80");
	fpsList.addItem("120");
	if (game.gameSettings.FPS == 30) fpsList.setSelectedIndex(0);
	if (game.gameSettings.FPS == 60) fpsList.setSelectedIndex(1);
	if (game.gameSettings.FPS == 80) fpsList.setSelectedIndex(2);
	if (game.gameSettings.FPS == 120) fpsList.setSelectedIndex(3);
	fpsList.setOnClickCallback([&]() -> void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		game.gameSettings.FPS = std::stoi(fpsList.getSelectedText());
		game.updateSetting();
	});

	rotateList.init(windowW * 0.4f, windowH * 0.25f, btnW * 2 * game.getWindowScale(), btnH * 2 * game.getWindowScale(), game.translations["dropdownRotate"], game.getRenderer(), game.textFont);
	rotateList.setTextSize(32 * game.getWindowScale());
	rotateList.addItem(game.translations["settingOpen"]);
	rotateList.addItem(game.translations["settingClose"]);
	if (game.gameSettings.cardRotate) rotateList.setSelectedIndex(0);
	else rotateList.setSelectedIndex(1);
	rotateList.setOnClickCallback([&]() -> void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		game.gameSettings.cardRotate = rotateList.getSelectedIndex() == 0;
		game.updateSetting();
	});

	crtList.init(windowW * 0.42f, windowH * 0.25f, btnW * 2 * game.getWindowScale(), btnH * 2 * game.getWindowScale(), game.translations["dropdownCRT"], game.getRenderer(), game.textFont);
	crtList.setTextSize(32 * game.getWindowScale());
	crtList.addItem(game.translations["settingOpen"]);
	crtList.addItem(game.translations["settingClose"]);
	if (game.gameSettings.CRT) crtList.setSelectedIndex(0);
	else crtList.setSelectedIndex(1);
	crtList.setOnClickCallback([&]() -> void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		game.gameSettings.CRT = crtList.getSelectedIndex() == 0;
		game.updateSetting();
	});

	particleList.init(windowW * 0.55f, windowH * 0.25f, btnW * 2 * game.getWindowScale(), btnH * 2 * game.getWindowScale(), game.translations["dropdownParticle"], game.getRenderer(), game.textFont);
	particleList.setTextSize(32 * game.getWindowScale());
	particleList.addItem(game.translations["settingOpen"]);
	particleList.addItem(game.translations["settingClose"]);
	if (game.gameSettings.isParticle) particleList.setSelectedIndex(0);
	else particleList.setSelectedIndex(1);
	particleList.setOnClickCallback([&]() -> void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		game.gameSettings.isParticle = particleList.getSelectedIndex() == 0;
		game.updateSetting();
	});

	windowList.init(windowW * 0.25f, windowH * 0.25f, btnW * 3 * game.getWindowScale(), btnH * 2 * game.getWindowScale(), game.translations["dropdownWindow"], game.getRenderer(), game.textFont);
	windowList.setTextSize(32* game.getWindowScale());
	windowList.addItem(game.translations["dropdownWindowFull"]);
	windowList.addItem("1920x1080");
	windowList.addItem("1600x900");
	windowList.addItem("1280x720");
	windowList.addItem("1024x768");
	if (game.gameSettings.screenWidth == 1920) windowList.setSelectedIndex(1);
	if (game.gameSettings.screenWidth == 1600) windowList.setSelectedIndex(2);
	if (game.gameSettings.screenWidth == 1280) windowList.setSelectedIndex(3);
	if (game.gameSettings.screenWidth == 1024) windowList.setSelectedIndex(4);
	if (game.gameSettings.fullscreen) windowList.setSelectedIndex(0);
	windowList.setOnClickCallback([&]() -> void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		switch (windowList.getSelectedIndex()) {
		case 0:
			SDL_Rect rect;
			SDL_GetDisplayBounds(0, &rect);
			game.gameSettings.fullscreen = true;
			game.gameSettings.screenWidth = rect.w;
			game.gameSettings.screenHeight = rect.h;
			break;
		case 1:
			game.gameSettings.screenWidth = 1920;
			game.gameSettings.screenHeight = 1080;
			game.gameSettings.fullscreen = false;
			break;
		case 2:
			game.gameSettings.screenWidth = 1600;
			game.gameSettings.screenHeight = 900;
			game.gameSettings.fullscreen = false;
			break;
		case 3:
			game.gameSettings.screenWidth = 1280;
			game.gameSettings.screenHeight = 720;
			game.gameSettings.fullscreen = false;
			break;
		case 4:
			game.gameSettings.screenWidth = 1024;
			game.gameSettings.screenHeight = 768;
			game.gameSettings.fullscreen = false;
			break;
		}
	});

	bgmSlider.init(windowW * 0.25f, windowH * 0.30f, btnW * 6 * game.getWindowScale(), btnH * game.getWindowScale(), game.translations["slideBgm"], game.getRenderer(), game.textFont);
	bgmSlider.setRange(0, 100);
	bgmSlider.setValue(game.gameSettings.bgmVolume * 100);
	bgmSlider.setTextSize(30 * game.getWindowScale());

	sfxSlider.init(windowW * 0.25f, windowH * 0.45f, btnW * 6 * game.getWindowScale(), btnH* game.getWindowScale(), game.translations["slideSfx"], game.getRenderer(), game.textFont);
	sfxSlider.setRange(0, 100);
	sfxSlider.setValue(game.gameSettings.sfxVolume * 100);
	sfxSlider.setTextSize(30 * game.getWindowScale());

	hideAllSettings();
}

void SceneSetting::clean() {
	for (auto& sound : sounds) {
		if (sound.second != nullptr) {
			Mix_FreeChunk(sound.second);
		}
	}
}

void SceneSetting::loadAnimation() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	startLoadMask.position = SDL_FPoint{ 0,0 };
	Movement fly;
	fly.rotate = { -360,300.0f };
	fly.move = { SDL_FPoint{windowW / 2.0f,windowH / 2.0f}, 1000.0f };
	fly.scale = { SDL_FPoint{-windowW * 1.4f,-windowW * 1.4f}, 4000.0f };
	startLoadMask.movements.push(fly);
}

void SceneSetting::hideAllSettings(){
	for (int i = 0; i < langTypes.size(); ++i) {
		langTypes[i].setDisplay(false);
	}
	fpsList.setDisplay(false);
	windowList.setDisplay(false);
	bgmSlider.setDisplay(false);
	sfxSlider.setDisplay(false);
	crtList.setDisplay(false);
	rotateList.setDisplay(false);
	particleList.setDisplay(false);
}

#include "SceneIndex.h"
#include "SceneMain.h"
#include "SceneSetting.h"
#include "Game.h"

SceneIndex::SceneIndex() : game(Game::getInstance()) ,sceneName("index"){
}

SceneIndex::~SceneIndex() {

}

void SceneIndex::update(float deltaTime){

	const float step = deltaTime;
	title.angle += step;
	if (title.angle > M_PI * 2)  title.angle -= M_PI * 2;

	switch (currentStatus){
	case TO_GAME:
	case TO_SETTING:
		switch (substep) {
		case 1:
			Mix_PlayChannel(-1, sounds["metal_bars"], 0);
			loadAnimation();
			substep = 2;
			break;
		case 2:
			loadMask.action(deltaTime);
			if (loadMask.movements.empty()) {
				substep = 3;
			}
			break;
		case 3:
			substep = 1;
			if (currentStatus == TO_GAME) {
				auto sceneMain = new SceneMain();
				game.changeScene(sceneMain);
			}
			else if (currentStatus == TO_SETTING) {
				auto sceneSetting = new SceneSetting();
				game.changeScene(sceneSetting);
			}
			return;
		}
		
		break;
	case SCENE_WAITING:
		loadMask.position = { (float)game.getWindowWidth(),(float)game.getWindowHeight() };
		loadMask.width = 1;
		loadMask.height = 1;
		break;
	}

	float cardStep[4] = { 0.015f,0.012f,0.01f,0.008f };
	float len[4] = { 100,150,200,250 };
	for (int i = 0; i < 4; ++i) {
		indexCard[i].position = { game.getWindowWidth() * btnCenterX - indexCard[i].width / 2,game.getWindowHeight() * btnCenterY - indexCard[i].height / 2 };
		indexCard[i].degree = sin(cardDeg[i]) * len[i];
		indexCard[i].position.x += cos(cardPos[i]) * len[i];
		indexCard[i].position.y += sin(cardPos[i]) * len[i];
		cardPos[i] += cardStep[i];
		cardDeg[i] += cardStep[i]/5;
		if (cardDeg[i] >= M_PI * 2) cardDeg[i] -= M_PI * 2;
	}

	backgroundImg.degree -= 0.01;
	if (backgroundImg.degree >= 2 * M_PI) backgroundImg.degree -= 2 * M_PI;

}

void SceneIndex::render() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	SDL_Rect backgroundRect = { -10,-windowW * 0.2,windowW + 20,windowW + 20 };
	SDL_Point backgroundCenter = { windowW / 2 ,windowH / 2 - windowW * 0.2 };
	SDL_RenderCopyEx(game.getRenderer(), backgroundImg.texture, NULL, &backgroundRect,cos(backgroundImg.degree)*0.2, &backgroundCenter,SDL_FLIP_NONE);

	for (int i = 3; i >= 0; --i) {
		SDL_Rect cardRect = { indexCard[i].position.x,indexCard[i].position.y,indexCard[i].width,indexCard[i].height };
		SDL_Rect cardCilpRect = cardSrcRects[indexCard[i].suit][indexCard[i].num];
		SDL_Point cardRotateCenter = { cardRect.w / 2 , cardRect.h / 2 };
		indexCard[i].render(cardRotateCenter, 0, game.getRenderer(), indexCard[i].texture, cardCilpRect, cardRect);
	}

	SDL_Rect titleRect = { game.getWindowWidth() * 0.5f - title.width / 2,game.getWindowHeight() * titleY - title.height / 2,title.width,title.height };
	SDL_Point center = {title.width/2, title.height/2};
	SDL_RenderCopyEx(game.getRenderer(), title.texture, NULL, &titleRect, sin(title.angle) * 0.3f, &center, SDL_FLIP_NONE);

	quitBtn.render();
	startBtn.render();
	settingBtn.render();
	honorBtn.render();
	infoBtn.render();

	SDL_Rect loadMaskRect = { loadMask.position.x,loadMask.position.y,loadMask.width,loadMask.height };
	center = { loadMask.width/2,loadMask.height/2 };
	SDL_RenderCopyEx(game.getRenderer(), loadMask.texture, NULL, &loadMaskRect, loadMask.degree, &center, SDL_FLIP_NONE);

	
}

void SceneIndex::handleEvent(SDL_Event* event) {
	startBtn.handleEvent(*event);
	settingBtn.handleEvent(*event);
	honorBtn.handleEvent(*event);
	infoBtn.handleEvent(*event);
	quitBtn.handleEvent(*event);
}

void SceneIndex::init() {
	game.initLang(sceneName);
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	sounds["click"] = Mix_LoadWAV("assets/audio/click.mp3");
	sounds["metal_bars"] = Mix_LoadWAV("assets/audio/metal_bars.wav");
	//sounds["wood_hit"] = Mix_LoadWAV("assets/audio/wood_hit.wav");
	//sounds["tap"] = Mix_LoadWAV("assets/audio/tap.mp3");

	// Load Background Iamge
	backgroundImg.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/background.png");
	SDL_QueryTexture(backgroundImg.texture, NULL, NULL, &backgroundImg.width, &backgroundImg.height);

	float titleScale = 3.5f * game.getWindowScale();
	title.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/title.png");
	SDL_QueryTexture(title.texture, NULL, NULL, &title.width, &title.height);
	title.width *= titleScale;
	title.height *= titleScale;

	int btnW = 32, btnH = 32;
	float btnScale = 3.0f * game.getWindowScale();

	quitBtn.init(windowW - btnW * btnScale/2, 0, btnW, btnH, btnScale/2, game.getRenderer());
	quitBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/button_quit.png");
	quitBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		game.isRunning = false;
	});

	startBtn.init(windowW * btnCenterX - btnW * btnScale / 2, windowH * (btnCenterY - btnShiftY) - btnH * btnScale / 2, btnW, btnH, btnScale, game.getRenderer());
	startBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/button_start.png");
	startBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		currentStatus = TO_GAME;
	});

	honorBtn.init(windowW * (btnCenterX - btnShiftX) - btnW * btnScale / 2, windowH * btnCenterY - btnH * btnScale / 2, btnW, btnH, btnScale, game.getRenderer());

	honorBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/button_honor.png");
	honorBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
	});

	settingBtn.init(windowW * (btnCenterX + btnShiftX) - btnW * btnScale / 2, windowH * btnCenterY - btnH * btnScale / 2, btnW, btnH, btnScale, game.getRenderer());
	settingBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/button_setting.png");
	settingBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		currentStatus = TO_SETTING;
	});

	infoBtn.init(windowW * btnCenterX - btnW * btnScale / 2, windowH * (btnCenterY + btnShiftY) - btnH * btnScale / 2, btnW, btnH, btnScale, game.getRenderer());
	infoBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/button_info.png");
	infoBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		});

	loadMask.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/index/load_mask.png");
	SDL_QueryTexture(loadMask.texture, NULL, NULL, &loadMask.width, &loadMask.height);
	substep = 1;

	float cardScale = 3.0f * game.getWindowScale();
	for (int i = 0; i < 4; ++i) {
		indexCard[i].texture = IMG_LoadTexture(game.getRenderer(), "assets/image/cards.png");
		indexCard[i].width = cardW * cardScale, indexCard[i].height = cardH * cardScale;
		indexCard[i].position = { windowW * btnCenterX - indexCard[i].width / 2,windowH * btnCenterY - indexCard[i].height / 2 };
		indexCard[i].num = 13;
		indexCard[i].suit = i+1;
	}
	calcCardTemplate();

}

void SceneIndex::calcCardTemplate() {
	calcCardSrcRects(cardSrcRects, cardW, cardH);
}

void SceneIndex::clean(){
	//if ( != nullptr) {
		//SDL_DestroyTexture();
	//}

	//backgroundImg.clean();
	//loadMask.clean();
	//startBtn.clean();
	//collectionBtn.clean();
	//settingBtn.clean();
	//title.clean();

	for (auto sound : sounds) {
		if (sound.second != nullptr) {
			Mix_FreeChunk(sound.second);
		}
	}
}

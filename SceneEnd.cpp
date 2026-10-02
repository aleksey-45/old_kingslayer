#include "SceneEnd.h"
#include "Game.h"
#include "GameData.h"
#include "SceneIndex.h"
#include "SceneMain.h"
#include "GameSave.h"

SceneEnd::SceneEnd() : game(Game::getInstance()), sceneName("end") {
}

SceneEnd::~SceneEnd() {
}

void SceneEnd::update(float deltaTime) {
	switch (currentStatus){
	case TO_RESTART: {
		Mix_PlayChannel(-1, sounds["metal_bars"], 0);
		SDL_Delay(250);
		auto sceneMain = new SceneMain();
		game.changeScene(sceneMain);
		return;
	}
	case TO_HOME: {
		Mix_PlayChannel(-1, sounds["metal_bars"], 0);
		SDL_Delay(250);
		auto sceneIndex = new SceneIndex();
		game.changeScene(sceneIndex);
		return;
	}
	case SCENE_WAITING:
		break;

	}

}

void SceneEnd::render() {
	
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	SDL_Rect backgroundRect = { 0,0,windowW,windowH };
	SDL_RenderCopy(game.getRenderer(), backgroundImg.texture, NULL, &backgroundRect);

	title.render(game.getRenderer(),game.titleFont, TTF_STYLE_BOLD);
	bossText.render(game.getRenderer(),game.textFont, TTF_STYLE_BOLD);
	gameInfoText.render(game.getRenderer(),game.textFont, TTF_STYLE_BOLD);
	mottoText.render(game.getRenderer(),game.textFont, TTF_STYLE_BOLD);
	homeBtn.render();
	restartBtn.render();
}

void SceneEnd::handleEvent(SDL_Event* event) {
	homeBtn.handleEvent(*event);
	restartBtn.handleEvent(*event);
}

void SceneEnd::init(){
	game.initLang(sceneName);
	Mix_PlayMusic(game.backgroundMusic, -1);
	GameSave::clearGameSave();

	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	gameData = gameDataTemplate;

	sounds["click"] = Mix_LoadWAV("assets/audio/click.mp3");
	sounds["metal_bars"] = Mix_LoadWAV("assets/audio/metal_bars.wav");
	backgroundImg.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/end/background.png");
	//Board.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/end/background.png");


	int btnW = 32, btnH = 32;
	float btnScale = 3.0f * game.getWindowScale();
	homeBtn.init(windowW * btnX - btnW * btnScale / 2, windowH * homeBtnY - btnH * btnScale / 2, btnW, btnH, btnScale, game.getRenderer());
	homeBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/end/button_home.png");
	homeBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		currentStatus = TO_HOME;
	});
	restartBtn.init(windowW * btnX - btnW * btnScale / 2, windowH * restartBtnY - btnH * btnScale / 2, btnW, btnH, btnScale, game.getRenderer());
	restartBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/end/button_restart.png");
	restartBtn.setOnClickCallback([&]()->void {
		Mix_PlayChannel(-1, sounds["click"], 0);
		currentStatus = TO_RESTART;
	});

	title.init(
		SDL_FPoint{ windowW * 0.5f ,windowH * titleY },
		"GAME OVER",64,
		SDL_Color{ 230,230,230,255 },
		{ 0,0,0,255 },
		{ 0,5 }
	);

	bossText.init(
		SDL_FPoint{ windowW * 0.5f ,windowH * bossTextY }, 
		"Killed By " + suitName[gameData.currentBoss.bossType.suit] + " " + numName[gameData.currentBoss.bossType.num],32,
		SDL_Color{ 230,230,230,255 }
	);

	gameInfoText.init(
		SDL_FPoint{ windowW * 0.5f ,windowH * gameInfoTextY },
		"Round " + std::to_string(gameData.gameRound) + "    Loop " + std::to_string(gameData.gameLoop) + "    Seed=" + std::to_string(gameData.seed),32,
		SDL_Color{ 230,230,230,255 }
	);

	std::mt19937 rnd(time(0));
	mottoText.init(SDL_FPoint{ windowW * 0.5f ,windowH * mottoTextY },mottos[rnd() % mottos.size()],24,SDL_Color{ 230,230,230,255 });
}

void SceneEnd::clean(){
	for (auto sound : sounds) {
		if (sound.second != nullptr) {
			Mix_FreeChunk(sound.second);
		}
	}
}

void SceneEnd::loadAnimation()
{
}
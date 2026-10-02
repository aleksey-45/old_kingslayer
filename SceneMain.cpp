#include "SceneMain.h"
#include "SceneEnd.h"
#include "Game.h"
#include "GameSave.h"

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <random>

using std::string;
using std::to_string;

// gameData.substep named per-phase constants (values unchanged from the original code).
// Suit trigger steps (3/5/7/9/11 and their +1 settle frames) live in cardSteps in Object.h.
namespace plAtk {       // PLAYER_ATTACK
	constexpr int INIT        = 1;
	constexpr int DAMAGE      = 2;
	constexpr int DAMAGE_WAIT = 2001;
	constexpr int DISCARD_FLY = 13;
	constexpr int APPLY_PLAY  = 14;
	constexpr int FINISH      = 15;
}
namespace plDef {       // BOSS_OFFENSIVE
	constexpr int SETUP      = 1;
	constexpr int WAIT_INPUT = 2;
}
namespace plDis {       // PLAYER_DISCARD
	constexpr int FLY    = 1;
	constexpr int SETTLE = 2;
	constexpr int NEXT   = 3;
}
namespace plBossUpd {   // BOSS_UPDATE
	constexpr int PASSIVE    = 1;
	constexpr int CHECK_DEAD = 1001;
	constexpr int TAROT_STEP = 2;
	constexpr int SPAWN_NEXT = 3;
	constexpr int NEXT_TURN  = 4;
}
namespace plStart {     // GAME_START
	constexpr int LOAD = 1;
	constexpr int DEAL = 2;
}
namespace plEnd {       // GAME_END
	constexpr int ANIM   = 1;
	constexpr int SWITCH = 2;
}
namespace plTarot {     // TAROT_CHOOSE
	constexpr int PREP  = 1;
	constexpr int PICK  = 2;
	constexpr int APPLY = 3;
}

SceneMain::SceneMain() : game(Game::getInstance()), sceneName("game"){
}

SceneMain::~SceneMain() {
}


void SceneMain::update(float deltaTime) {
	playerParticle.update(deltaTime);
	bossParticle.update(deltaTime);
	while (!runAnimation(deltaTime)) return;
	
	selectedCards.clear();
	for (Card& x : pokerHand) {
		if (x.isSelect) {
			selectedCards.push_back(CardData(x.suit, x.num));
			selectedCards.back().cardId = x.cardId;
		}
	}
	selectedTarot.clear();
	for (int i = 0; i < (int)tarotChoice.size(); ++i) {
		if (tarotChoice[i].isSelect) {
			selectedTarot.push_back(tarotChoice[i].num);
		}
	}

	switch (gameData.currentStatus) {
	case PLAYER_DEALING:
		playerDealing();
		break;
	case PLAYER_ATTACK:
		playerAttack();
		break;
	case BOSS_OFFENSIVE:
		bossOffensive();
		break;
	case PLAYER_DISCARD:
		playerDiscard();
		break;
	case BOSS_UPDATE:
		bossUpdate();
		break;
	case GAME_START:
		gameStart();
		break;
	case GAME_END:
		gameEnd();
		break;
	case TAROT_CHOOSE:
		tarotChoose();
		break;
	}


	string gameInfoText = "Round " + to_string(gameData.gameRound) + "  Loop " + to_string(gameData.gameLoop);
	string tarotInfoText;
	for (int i = 0; i <= 21; ++i) {
		tarotInfoText = game.translations["tarot" + to_string(i)] + " : " + to_string(gameData.tarotLevel[i]);
		tempTarotInfo[i].text = tarotInfoText;
	}
	tempGameInfo.text = gameInfoText;

	if (gameData.currentStatus == GAME_END) return;
	if (gameData.currentStatus == PLAYER_ATTACK) return;
	if (gameData.currentStatus == PLAYER_DISCARD) return;
	if (gameData.currentStatus == BOSS_UPDATE) return;
	// Set Hand Position
	int handCurCnt = (int)pokerHand.size();
	for (int i = 0; i < handCurCnt; ++i) {
		float cardShiftY = -abs(handCurCnt / 2.0f - i - 0.7f) * abs(handCurCnt / 2.0f - i - 0.7f) * cardTemplate.height * 0.05f / handCurCnt;
		pokerHand[i].width = cardTemplate.width;
		pokerHand[i].height = cardTemplate.height;
		pokerHand[i].position.x = game.getWindowWidth() * ((handLimitRight - handLimitLeft) / (handCurCnt + 1) * (i + 1) + handLimitLeft) - cardTemplate.width / 2.0f;
		pokerHand[i].position.y = game.getWindowHeight() * handLimitTop - cardTemplate.height / 2.0f - cardShiftY;
		pokerHand[i].angle = pokerHand[0].angle + i*0.4f;
	}

	// Set Boss Position
	if (gameData.currentStatus != BOSS_UPDATE && gameData.currentStatus != BOSS_OFFENSIVE && gameData.currentStatus != TAROT_CHOOSE) {
		currentBoss.width = cardWidth * bossScale, currentBoss.height = cardHeight * bossScale;
		currentBoss.position.x = game.getWindowWidth() * 0.5f - currentBoss.width / 2;
		currentBoss.position.y = game.getWindowHeight() * bossTop;
	}
	cardsAnimation(deltaTime);
}

void SceneMain::render() {
	SDL_Rect backgroundRect = { backgroundImg.position.x,backgroundImg.position.y,game.getWindowWidth(),game.getWindowHeight() };
	SDL_RenderCopy(game.getRenderer(), backgroundImg.texture, NULL, &backgroundRect);
	
	tempGameInfo.render(game.getRenderer(), game.textFont,TTF_STYLE_BOLD);
	for (int i = 0; i <= 21; ++i) tempTarotInfo[i].render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);

	for (Pile& k : recyclingCards) k.render(game.getRenderer(), NULL, 1);

	renderTips();
	renderAttackInfo();

	if(game.gameSettings.isParticle) playerParticle.render();
	for (int i = 0; i < (int)pokerHand.size(); ++i) { // Render Poker
		SDL_Rect cardRect = { pokerHand[i].position.x,pokerHand[i].position.y,pokerHand[i].width,pokerHand[i].height };
		SDL_Rect cardCilpRect = cardSrcRects[pokerHand[i].suit][pokerHand[i].num];
		SDL_Point cardRotateCenter;
		if (gameData.currentStatus == PLAYER_DISCARD || gameData.currentStatus == PLAYER_ATTACK) {
			cardRotateCenter = { pokerHand[i].width / 2, pokerHand[i].height / 6 * 5 };
		}
		else cardRotateCenter = { pokerHand[i].width / (int)pokerHand.size() * i, pokerHand[i].height / 6 * 5 };
		pokerHand[i].render(cardRotateCenter, cardSelectShift,game.getRenderer(),cardTemplate.texture,cardCilpRect,cardRect);
	}

	// Card Pile
	//SDL_Log(to_string(discardPileSize).c_str());
	cardPile.render(game.getRenderer(), pileBase.texture, cardPileSize);
	discardPile.render(game.getRenderer(), pileBase.texture, discardPileSize);
	renderCardInfo();

	// Render Btn
	attackBtn.render();
	skipBtn.render();
	defenseBtn.render();

	// Render End Mask
	if (gameData.currentStatus == GAME_END) {
		SDL_Rect endMaskRect = { endLoadMask.position.x,endLoadMask.position.y,game.getWindowWidth(),game.getWindowHeight() };
		SDL_RenderCopy(game.getRenderer(), endLoadMask.texture, NULL, &endMaskRect);
	}

	{// Render Boss
		SDL_Rect cardRect = { currentBoss.position.x,currentBoss.position.y,currentBoss.width,currentBoss.height };
		if (gameData.currentBoss.isDisabled) {
			cardRect.x += cardRect.w * 0.05;
			cardRect.w *= 0.9;
			cardRect.h *= 0.9;
		}
		SDL_Rect cardCilpRect = cardSrcRects[currentBoss.suit][currentBoss.num];
		SDL_Point cardRotateCenter = { cardRect.w / 2 , cardRect.h / 2 };
		currentBoss.render(cardRotateCenter, cardSelectShift, game.getRenderer(), cardTemplate.texture, cardCilpRect, cardRect, gameData.currentStatus == BOSS_OFFENSIVE);

		if (currentBoss.sin != 0) {
			const float cardAngle = ::sin(currentBoss.angle) * 2 + currentBoss.degree;
			float bossScale = 1.0f * currentBoss.width / cardTemplate.width;
			const int shaderMargin = 8 * bossScale;
			SDL_Rect shaderRect = cardRect;
			shaderRect.x -= shaderMargin;
			shaderRect.y -= shaderMargin;
			shaderRect.w += shaderMargin * 2;
			shaderRect.h += shaderMargin * 2;
			SDL_RenderCopyEx(game.getRenderer(), bossShader[currentBoss.sin].texture, NULL, &shaderRect, cardAngle, &cardRotateCenter, SDL_FLIP_NONE);
		}
		if (game.gameSettings.isParticle) bossParticle.render();
		renderBossInfo();
	}

	if (gameData.currentStatus == GAME_START) {
		SDL_Rect loadMaskRect = { startLoadMask.position.x,startLoadMask.position.y,startLoadMask.width,startLoadMask.height };
		SDL_Point center = { startLoadMask.width / 2,startLoadMask.height / 2 };
		SDL_RenderCopyEx(game.getRenderer(), startLoadMask.texture, NULL, &loadMaskRect, startLoadMask.degree, &center, SDL_FLIP_NONE);
	}

	// Render Tarot
	if (gameData.currentStatus == TAROT_CHOOSE) {
		SDL_Rect tarotMaskRect = { tarotMask.position.x,tarotMask.position.y,tarotMask.width,tarotMask.height };
		SDL_RenderCopy(game.getRenderer(), tarotMask.texture, NULL, &tarotMaskRect);
		for (int i = 0; i < (int)tarotChoice.size(); ++i) {
			SDL_Rect cardRect = { tarotChoice[i].position.x,tarotChoice[i].position.y,tarotChoice[i].width,tarotChoice[i].height };
			SDL_Rect cardCilpRect = tarotSrcRects[tarotChoice[i].num];
			SDL_Point cardRotateCenter = { cardRect.w / 2 , cardRect.h / 2 };
			tarotChoice[i].render(cardRotateCenter, cardSelectShift, game.getRenderer(), tarotTemplate.texture, cardCilpRect, cardRect);
		}

		tarotTitle.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
		tarotBtn.render();
		tarotText.render(game.getRenderer(), game.titleFont, TTF_STYLE_BOLD);
		renderTarotTips();
	}

	numberText.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);

}

void SceneMain::handleEvent(SDL_Event* event) {
	if (gameData.currentStatus == GAME_START || gameData.currentStatus == GAME_END) return;
	if (gameData.currentStatus != TAROT_CHOOSE) {
		attackBtn.handleEvent(*event);
		defenseBtn.handleEvent(*event);
		skipBtn.handleEvent(*event);
	}
	else {
		tarotBtn.handleEvent(*event);
	}
	for (int i = (int)pokerHand.size() - 1; i >= 0; --i) {
		if (gameData.currentStatus == PLAYER_DISCARD) continue;
		if (gameData.currentStatus == PLAYER_ATTACK) continue;
		if (gameData.currentStatus == TAROT_CHOOSE) continue;
		pokerHand[i].handleEvent(event, cardSelectShift, sounds["wood_hit"], sounds["tap"]);
		if (pokerHand[i].isClick) break;
	}
	if (gameData.currentStatus == TAROT_CHOOSE) {
		for (int i = (int)tarotChoice.size() - 1; i >= 0; --i) {
			tarotChoice[i].handleEvent(event, cardSelectShift, sounds["wood_hit"], sounds["tap"]);
			if (tarotChoice[i].isClick) break;
		}
	}
}

void SceneMain::init() {
	game.initLang(sceneName);
	sounds["click"] = Mix_LoadWAV("assets/audio/click.mp3");
	sounds["card_shuffle"] = Mix_LoadWAV("assets/audio/card_shuffle.mp3");
	sounds["card_draw"] = Mix_LoadWAV("assets/audio/card_draw.mp3");
	sounds["metal_bars"] = Mix_LoadWAV("assets/audio/metal_bars.wav");
	sounds["wood_hit"] = Mix_LoadWAV("assets/audio/wood_hit.wav");
	sounds["tap"] = Mix_LoadWAV("assets/audio/tap.mp3");
	sounds["paging"] = Mix_LoadWAV("assets/audio/paging.mp3");
	sounds["paging_long"] = Mix_LoadWAV("assets/audio/paging_long.mp3");
	sounds["game_over"] = Mix_LoadWAV("assets/audio/game_over.mp3");

	sounds["XIII_Death"] = Mix_LoadWAV("assets/audio/XIII_Death.wav");
	sounds["IX_The_Hermit"] = Mix_LoadWAV("assets/audio/XIII_Death.wav");
	sounds["I_The_Magician"] = Mix_LoadWAV("assets/audio/XIII_Death.wav");

	sounds["lute0"] = Mix_LoadWAV("assets/audio/captainnfo_lute0.wav");
	sounds["lute1"] = Mix_LoadWAV("assets/audio/captainnfo_lute1.wav");
	sounds["lute2"] = Mix_LoadWAV("assets/audio/captainnfo_lute2.wav");
	sounds["lute3"] = Mix_LoadWAV("assets/audio/captainnfo_lute3.wav");
	sounds["lute4"] = Mix_LoadWAV("assets/audio/captainnfo_lute4.wav");

	sounds["lyre_do"] = Mix_LoadWAV("assets/audio/lyre_do.wav");
	sounds["lyre_sol"] = Mix_LoadWAV("assets/audio/lyre_sol.wav");


	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	bossScale *= game.getWindowScale();
	cardScale *= game.getWindowScale();
	tarotScale *= game.getWindowScale();

	// Load Background Iamge
	backgroundImg.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/background1.png");
	SDL_QueryTexture(backgroundImg.texture, NULL, NULL, &backgroundImg.width, &backgroundImg.height);

	// Load End Game Mask
	endLoadMask.position = { 0,-1.0f * windowH };
	endLoadMask.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/end_load_mask.png");

	// Load Start Game Mask
	startLoadMask.position = { 0,0 };
	startLoadMask.width = windowW * 1.4f;
	startLoadMask.height = windowW * 1.4f;
	startLoadMask.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/start_load_mask.png");

	// Load UI Iamge
	uiHpImage = IMG_LoadTexture(game.getRenderer(), "assets/image/boss_hp.png");
	uiAtkImage = IMG_LoadTexture(game.getRenderer(), "assets/image/boss_atk.png");

	// Load Button
	// PlayerButton
	int handBtnW = 64, handBtnH = 32;
	float btnScale = 2.5f * game.getWindowScale();
	attackBtn.init(windowW * handBtnCenterX - handBtnW * btnScale / 2, windowH * handBtnTop, handBtnW, handBtnH, btnScale, game.getRenderer());
	attackBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/attack_button.png");
	attackBtn.setOnClickCallback([&]()->void {
		attackBtn.setEnabled(false);
		gameData.currentStatus = PLAYER_ATTACK;
		gameData.substep = plAtk::INIT;
		Mix_PlayChannel(-1, sounds["click"], 0);
		});
	skipBtn.init(windowW * handBtnCenterX - handBtnW * btnScale / 2, windowH * handBtnTop, handBtnW, handBtnH, btnScale, game.getRenderer());
	skipBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/skip_button.png");
	skipBtn.setOnClickCallback([&]()->void {
		skipBtn.setEnabled(false);
		gameData.currentStatus = PLAYER_ATTACK;
		gameData.substep = plAtk::INIT;
		Mix_PlayChannel(-1, sounds["click"], 0);
		});
	defenseBtn.init(windowW * handBtnCenterX - handBtnW * btnScale / 2, windowH * handBtnTop, handBtnW, handBtnH, btnScale, game.getRenderer());
	defenseBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/defense_button.png");
	defenseBtn.setOnClickCallback([&]()->void {
		defenseBtn.setEnabled(false);
		bossBackAnimation();
		gameData.currentStatus = PLAYER_DISCARD;
		gameData.substep = plDis::FLY;
		Mix_PlayChannel(-1, sounds["click"], 0);

		});

	// Load Card Piles
	pileBase.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/pile_base.png");
	cardPile.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/card_back.png");
	discardPile.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/discard_back.png");
	SDL_QueryTexture(cardPile.texture, NULL, NULL, &cardPile.width, &cardPile.height);
	SDL_QueryTexture(discardPile.texture, NULL, NULL, &discardPile.width, &discardPile.height);

	cardPile.width *= cardScale;
	cardPile.height *= cardScale;

	cardPile.position.x = windowW * pileLeftRight - cardPile.width / 2;
	cardPile.position.y = windowH * pileBottom - cardPile.height;

	discardPile.width *= cardScale;
	discardPile.height *= cardScale;

	discardPile.position.x = windowW * (1.0f - pileLeftRight) - discardPile.width / 2;
	discardPile.position.y = windowH * pileBottom - discardPile.height;


	// Init card template
	cardTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/cards.png");
	cardTemplate.width = cardWidth, cardTemplate.height = cardHeight;
	currentBoss.width = cardWidth, currentBoss.height = cardHeight;
	cardTemplate.width *= cardScale;
	cardTemplate.height *= cardScale;
	currentBoss.width *= bossScale;
	currentBoss.height *= bossScale;

	currentBoss.position.x = windowW * 0.5f - currentBoss.width / 2;
	currentBoss.position.y = windowH * bossTop;
	calcCardTemplate();


	bossParticle.init(game.getRenderer(), 50);
	playerParticle.init(game.getRenderer(), 150);

	// Init boss Shader
	for (int i = 1; i <= 7; ++i) {
		bossShader[i].texture = IMG_LoadTexture(game.getRenderer(), ("assets/image/sin_" + sinName[i] + ".png").c_str());
	}

	// Init tip
	currentTip.init(game.getRenderer(), game.textFont, 22, 16);

	// Init Number Text
	numberText.init({ 0,0 }, "", 24*game.getWindowScale(), {255,255,255,255});

	// Init tarot
	float tarotBtnScale = 3.0f;
	tarotTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/tarot.png");
	tarotTemplate.width = cardWidth, tarotTemplate.height = cardHeight;
	tarotTemplate.width *= tarotScale;
	tarotTemplate.height *= tarotScale;
	calcTarotTemplate();
	tarotMask.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/tarot_mask.png");
	tarotMask.width = 0;
	tarotMask.height = 0;
	tarotMask.position = { 0,0 };

	tarotBtn.init(windowW * handBtnCenterX - handBtnW * tarotBtnScale / 2, windowH * tarotBtnTop, handBtnW, handBtnH, tarotBtnScale, game.getRenderer());
	tarotBtn.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/tarot_button.png");
	tarotBtn.setOnClickCallback([&]()->void {
		tarotBtn.setEnabled(false);
		++gameData.substep;
		Mix_PlayChannel(-1, sounds["click"], 0);
		});

	tarotTitle.init({ windowW * 0.5f,windowH * (tarotApplyY - 0.2f) }, game.translations["tarotTitle"], 60,
		{ 132,117,153,255 }, { 0,0,0,255 }, { 0,3 });
	tarotText.init({ windowW * 0.5f,windowH * (tarotBtnTop - 0.03f) },  formatString(game.translations["tarotText"], { 1 }), 24,
		{ 132,117,153,255 }, { 0,0,0,255 }, { 0,2 });
	tarotTip.init(game.getRenderer(), game.textFont, 28, 16);

	
	tempGameInfo.init({ windowW * 0.07f,windowH * 0.03f }, "", 20, { 230,230,230,255 });
	for (int i = 0; i <= 21; ++i) tempTarotInfo[i].init({ windowW * 0.94f,windowH * 0.03f + 25 * i }, "", 16, { 230,230,230,255 });

	if (gameType == 0) {
		//Seed Test
		std::random_device rd;
		std::mt19937_64 rnd(rd());
		gameData.init(rnd());
		applyGameData();
		gameData.currentStatus = GAME_START;
		gameData.substep = plStart::LOAD;
	}
	else {
		if (GameSave::loadGame(gameData)) {
			applyGameData();
			gameData.substep = 1; // restored phase: reset to that phase's entry
		}
		else {
			std::random_device rd;
			std::mt19937_64 rnd(rd());
			gameData.init(rnd());
			applyGameData();
			gameData.currentStatus = GAME_START;
			gameData.substep = plStart::LOAD;
		}
		//assert(0);
	}
	//cardDebugTest(); // Test

}

void SceneMain::clean() {
	if (cardPile.texture != nullptr) {
		SDL_DestroyTexture(cardPile.texture);
	}
	if (discardPile.texture != nullptr) {
		SDL_DestroyTexture(discardPile.texture);
	}
	if (cardTemplate.texture != nullptr) {
		SDL_DestroyTexture(cardTemplate.texture);
	}
	if (backgroundImg.texture != nullptr) {
		SDL_DestroyTexture(backgroundImg.texture);
	}
	if (uiHpImage != nullptr) {
		SDL_DestroyTexture(uiHpImage);
	}
	if (uiAtkImage != nullptr) {
		SDL_DestroyTexture(uiAtkImage);
	}
	if (endLoadMask.texture != nullptr) {
		SDL_DestroyTexture(endLoadMask.texture);
	}
	if (startLoadMask.texture != nullptr) {
		SDL_DestroyTexture(startLoadMask.texture);
	}
	if (tarotMask.texture != nullptr) {
		SDL_DestroyTexture(tarotMask.texture);
	}


	for (auto sound : sounds) {
		if (sound.second != nullptr) {
			Mix_FreeChunk(sound.second);
		}
	}

}

void SceneMain::calcCardTemplate(){
	calcCardSrcRects(cardSrcRects, cardWidth, cardHeight);
}

void SceneMain::calcTarotTemplate() {
	// every tarot card cardWidthpx*cardHeightpx
	const int tarotImgW = cardWidth, tarotImgH = cardHeight;
	for (int i = 1; i <= 2; ++i) {
		for (int j = 1; j <= 11; ++j) {
			tarotSrcRects[(i - 1) * 11 + j - 1] = SDL_Rect{ (j - 1) * tarotImgW,(i - 1) * tarotImgH,tarotImgW,tarotImgH };
		}
	}
}

void SceneMain::renderBossInfo() {
	if (gameData.currentStatus == GAME_END) return;
	int imgSize = 48;
	int W = currentBoss.width;
	int H = currentBoss.height;
	int X = currentBoss.position.x;
	int Y = currentBoss.position.y;
	SDL_Rect hpRect = { X - imgSize/2,Y + H - imgSize/2,imgSize,imgSize };
	SDL_Rect atkRect = { X + W - imgSize/2 ,Y + H - imgSize/2,imgSize,imgSize };
	SDL_RenderCopy(game.getRenderer(), uiHpImage, NULL, &hpRect);
	SDL_RenderCopy(game.getRenderer(), uiAtkImage, NULL, &atkRect);

	uiHpText.init({ X*1.0f, Y * 1.0f + H }, to_string(gameData.currentBoss.bossHp), 28, { 0xef, 0xef, 0xef, 0xff });
	uiAtkText.init({ X * 1.0f + W, Y * 1.0f + H }, to_string(gameData.currentBoss.bossAtk), 28, { 0xef, 0xef, 0xef, 0xff });
	uiHpText.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
	uiAtkText.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
}

void SceneMain::renderCardInfo() {

	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	cardPileText.init({ cardPile.position.x + cardPile.width / 2.0f, cardPile.position.y + cardPile.width / 1.8f },
		to_string(cardPileSize), 50 * game.getWindowScale(),
		{ 198,120,65,255 }, { 0,0,0,160 }, { 0,5 }
	);

	discardPileText.init({ discardPile.position.x + discardPile.width / 2.0f, discardPile.position.y + discardPile.width / 1.8f },
		to_string(discardPileSize), 50 * game.getWindowScale(),
		{ 183,57,74,255 }, { 0,0,0,160 }, { 0,5 }
	);
	handText.init(
		{ windowW / 2.0f, windowH * pileBottom - cardTemplate.height * 0.7f },
		to_string(gameData.playerHand.size()) + " / " + to_string(gameData.handMaxCnt), 20 * game.getWindowScale(), 
		{ 0xef,0xef,0xef,255 }
	);

	handText.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
	cardPileText.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
	discardPileText.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);

}

void SceneMain::renderAttackInfo() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();

	// 计算攻击力和剩余手牌的数值
	int power = gameData.calcDeckSum(selectedCards);
	int powerD = gameData.undeath ? gameData.calcDeckSum(selectedCards) + gameData.currentBoss.bossAtk : gameData.calcDeckSum(selectedCards);
	int powerRest = gameData.calcDeckSum(gameData.playerHand) - power;

	// 根据游戏状态更新文本内容
	std::string attackText = std::to_string(power);
	std::string defenseText = std::to_string(powerD);
	std::string totalText = std::to_string(powerRest);

	// 初始化攻击信息文本框
	atkPower.init({ windowW * 0.5f + windowW * 0.07f, windowH * handBtnTop + windowH * 0.02f },
		attackText, 32,
		{ 255, 0, 0, 128 }, { 0, 0, 0, 128 }, { 0, 2 });
	atkPower.position.x += atkPower.width;
	// 初始化防御信息文本框
	defPower.init({ windowW * 0.5f + windowW * 0.07f, windowH * handBtnTop + windowH * 0.02f },
		defenseText, 32,
		{ 0, 0, 255, 128 }, { 0, 0, 0, 128 }, { 0, 2 });
	defPower.position.x += defPower.width;

	// 初始化剩余手牌信息文本框
	restPower.init({ windowW * 0.5f - windowW * 0.07f, windowH * handBtnTop + windowH * 0.02f },
		totalText, 32,
		{ 255, 255, 255, 128 }, { 0, 0, 0, 128 }, { 0, 2 });
	restPower.position.x -= restPower.width;

	// 根据游戏状态渲染文本框
	if (gameData.currentStatus == PLAYER_DEALING) {
		atkPower.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
	}
	else if (gameData.currentStatus == BOSS_OFFENSIVE && gameData.currentBoss.bossHp>0) {
		defPower.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
	}
	restPower.render(game.getRenderer(), game.textFont, TTF_STYLE_BOLD);
}

bool SceneMain::runAnimation(float deltaTime) {
	bool flag = true;
	for (Card& x : pokerHand) {
		if (!x.movements.empty()) {
			x.action(deltaTime);
			flag = false;
		}
	}
	for (Pile& x : recyclingCards) {
		if (!x.movements.empty()) {
			x.action(deltaTime);
			flag = false;
		}
	}
	if (!currentBoss.movements.empty()) {
		currentBoss.action(deltaTime);
		flag = false;
	}
	if (!cardPile.movements.empty()) {
		cardPile.action(deltaTime); 
		flag = false;
	}
	if (!discardPile.movements.empty()) {
		discardPile.action(deltaTime); 
		flag = false;
	}
	if (!endLoadMask.movements.empty()) {
		endLoadMask.action(deltaTime);
		flag = false;
	}
	if (!startLoadMask.movements.empty()) {
		startLoadMask.action(deltaTime);
		flag = false;
	}

	if (gameData.currentStatus == TAROT_CHOOSE) {
		for (Tarot& x : tarotChoice) {
			if (!x.movements.empty()) {
				x.action(deltaTime);
				flag = false;
			}
		}
		if (!tarotMask.movements.empty()) {
			tarotMask.action(deltaTime);
			flag = false;
		}
		if (!tarotTitle.movements.empty()) {
			tarotTitle.action(deltaTime);
			flag = false;
		}
	}

	if (!numberText.movements.empty()) {
		numberText.action(deltaTime);
		flag = false;
	}
	return flag;
}

void SceneMain::applyGameData() {
	currentBoss.suit = gameData.currentBoss.bossType.suit;
	currentBoss.num = gameData.currentBoss.bossType.num;
	currentBoss.cardId = gameData.currentBoss.bossType.cardId;
	currentBoss.sin = gameData.currentBoss.bossType.sin;
	lastHand = pokerHand;
	pokerHand.clear();
	for (int i = 0; i < gameData.playerHand.size(); ++i) {
		Card tmp;
		tmp.cardId = gameData.playerHand[i].cardId;
		tmp.suit = gameData.playerHand[i].suit;
		tmp.num = gameData.playerHand[i].num;
		tmp.width = cardTemplate.width;
		tmp.height = cardTemplate.height;
		tmp.position = { cardPile.position.x,cardPile.position.y - (gameData.playerHand.size() - i) * 4 };
		for (int j = 0; j < lastHand.size(); ++j) {
			if (lastHand[j] == tmp && lastHand[j].cardId == tmp.cardId) {
				tmp.position = lastHand[j].position;
				tmp.isSelect = lastHand[j].isSelect;
				tmp.movements = lastHand[j].movements;
				break;
			}
		}
		pokerHand.push_back(tmp);
	}
	cardPileSize = gameData.cardPile.size();
	discardPileSize = gameData.discardPile.size();
	//pokerHand
}

void SceneMain::renderTips() {
	if (gameData.currentStatus != PLAYER_ATTACK && gameData.currentStatus != PLAYER_DISCARD && gameData.currentStatus != TAROT_CHOOSE) {
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		for (int i = (int)pokerHand.size() - 1; i >= 0; --i) {

			SDL_Rect cardRect = { pokerHand[i].position.x,pokerHand[i].position.y,cardTemplate.width,cardTemplate.height };
			int num = pokerHand[i].num;
			int suit = pokerHand[i].suit;
			int power = gameData.calcDeckSum({ CardData{suit,num} }) - gameData.finalPower;
			string title, content;
			if (suit != 0) {
				title = game.translations["suit" + to_string(suit)] + " " + game.translations["num" + to_string(num)];
				content =
					formatString(game.translations["textPower"], { power }) + '\n' +
					game.translations["textAtk"] + '\n' +
					game.translations["text" + to_string(suit)];

				if (gameData.servantCards.count(num) != 0) content += "\n" + game.translations["textPet"];
				if (gameData.comboLimit / num >= 2) content += '\n' + formatString(game.translations["textCombo"], { gameData.comboLimit / num });
			}
			else
			{
				title = game.translations["suit" + to_string(suit)];
				content =
					formatString(game.translations["textPower"], { power }) + '\n' +
					game.translations["text" + to_string(suit)];
			}
			if (!pokerHand[i].isSelect) {
				if (cardRect.x <= mouseX && cardRect.y <= mouseY &&
					cardRect.x + cardRect.w >= mouseX && cardRect.y + cardRect.h >= mouseY) {
					currentTip.show(title, content, cardRect.x + cardRect.w / 2, cardRect.y, cardRect.w*1.5f);
					break;
				}
			}
			else {
				if (cardRect.x <= mouseX && cardRect.y - cardRect.h * cardSelectShift <= mouseY &&
					cardRect.x + cardRect.w >= mouseX && cardRect.y - cardRect.h * cardSelectShift + cardRect.h >= mouseY) {
					currentTip.show(title, content, cardRect.x + cardRect.w / 2, cardRect.y - cardRect.h * cardSelectShift, cardRect.w*1.5f);
					break;
				}
			}
		}
		SDL_Rect cardRect = { currentBoss.position.x,currentBoss.position.y,currentBoss.width,currentBoss.height };
		string title = game.translations["suit" + to_string(currentBoss.suit)] + " " + game.translations["num" + to_string(currentBoss.num)];

		string content = game.translations["textImmue" + to_string(gameData.currentBoss.isDisabled ? 0 : currentBoss.suit)];
		string contentEnvySin = game.translations["sin" + to_string(gameData.lastSinBossSin)];
		string contentEnvySuit = game.translations["suit" + to_string(gameData.lastSinBossSuit)];
		string contentSin = game.translations["sin" + to_string(currentBoss.sin)];
		if (currentBoss.sin == 5) contentSin = formatString(game.translations["sin" + to_string(currentBoss.sin)], {}, {}, { contentEnvySin ,contentEnvySuit });
		content += "\n\n" + contentSin;

		if (cardRect.x <= mouseX && cardRect.y <= mouseY &&
			cardRect.x + cardRect.w >= mouseX && cardRect.y + cardRect.h >= mouseY) {
			currentTip.show(title, content, cardRect.x + cardRect.w / 2, cardRect.y+cardRect.h, cardRect.w,false);
		}
	}
	
}

void SceneMain::renderTarotTips() {

	if (gameData.currentStatus == TAROT_CHOOSE) {
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		for (int i = (int)tarotChoice.size() - 1; i >= 0; --i) {
			SDL_Rect cardRect = { tarotChoice[i].position.x,tarotChoice[i].position.y,tarotTemplate.width,tarotTemplate.height };
			int num = tarotChoice[i].num;
			string title = game.translations["tarot" + to_string(num)];
			string content = gameData.getTarotDescription(num,game.translations);
			if (cardRect.x <= mouseX && cardRect.y <= mouseY &&
				cardRect.x + cardRect.w >= mouseX && cardRect.y + cardRect.h >= mouseY) {
				tarotTip.show(title, content, cardRect.x + cardRect.w / 2, cardRect.y + cardRect.h, cardRect.w * 1.5f, false);
				break;
			}
		}
	}
}

void SceneMain::playerDealing(){
	defenseBtn.setDisplay(false);
	if (selectedCards.empty()) {
		skipBtn.setDisplay(true);
		attackBtn.setDisplay(false);
	}
	else {
		skipBtn.setDisplay(false);
		attackBtn.setDisplay(true);
	}
	if (gameData.checkPlayCard(selectedCards)) {
		attackBtn.setEnabled(true);
		skipBtn.setEnabled(true);
	}
	else {
		attackBtn.setEnabled(false);
		skipBtn.setEnabled(false);
	}
}

// ---- playerAttack (PLAYER_ATTACK) substep ladder ----
//  INIT(1)        -> empty hand -> FINISH; else deal anim -> DAMAGE(2)
//  DAMAGE(2)      -> applyBossHurt total damage + floating number -> DAMAGE_WAIT(2001)
//  DAMAGE_WAIT    -> waits for the number anim, clears -> cardSteps::JOKER(3)
//  cardSteps 3/5/7/9/11 -> fire each present suit ability (joker/spade/heart/diamond/club);
//     each is followed by a "+1" settle frame that only refreshes the display
//  DISCARD_FLY(13)-> selected cards fly to discard -> APPLY_PLAY(14)
//  APPLY_PLAY(14) -> really remove cards from hand, re-layout -> FINISH(15)
//  FINISH(15)     -> decide next phase: uses left -> PLAYER_DEALING else -> BOSS_OFFENSIVE
void SceneMain::playerAttack() {
	int lackSuit = 1 ^ 2 ^ 3 ^ 4;
	int selectedSuitCnt = 0;
	int handSuitCnt = 0;
	int selectedSuit[5] = { 0,0,0,0,0 };
	int loveSuit[5] = { 0,0,0,0,0 };
	int handSuit[5] = { 0,0,0,0,0 };
	int handCurCnt = (int)pokerHand.size();
	for (Card& x : pokerHand) {
		if (x.isSelect) {
			++selectedSuit[x.suit];
		}
		++handSuit[x.suit];
	}
	for (int i = 1; i <= 4; ++i) {
		if (selectedSuit[i] > 0) {
			++selectedSuitCnt;
			for (int j = 1; j <= 4; ++j) {
				if (gameData.loveGraph[i][j] != 0) {
					++loveSuit[j];
				}
			}
		}
		if (handSuit[i] > 0) {
			++handSuitCnt;
			lackSuit ^= i;
		}
	}
	const int suitStep[5] = { cardSteps::JOKER, cardSteps::SPADE, cardSteps::HEART, cardSteps::DIAMOND, cardSteps::CLUB };
	auto chk = [&](int x) ->bool {
		if (gameData.substep != suitStep[x] || gameData.currentBoss.isImmune(x)) return false;
		if (gameData.checkSin(gameData.currentBoss, 5) && gameData.lastSinBossSuit == x) return false;
		if (selectedSuit[x] > 0) return true;
		if (x == 0) return false;
		if (loveSuit[x] > 0) return true;
		if (gameData.hasHierophant && handSuitCnt == 3 && lackSuit == x) return true;
		return false;
		};
	switch (gameData.substep) {
	case plAtk::INIT:
		if (selectedCards.empty()) {
			gameData.substep = plAtk::FINISH;
			break;
		}
		dealCardAnimation();
		currentBoss.coverColor = { 255,0,0,200 };
		gameData.substep = plAtk::DAMAGE;
		break;
	case plAtk::DAMAGE: {
		Mix_PlayChannel(-1, sounds["lute4"], 0);
		activeAffect(currentBoss);
		for (Card& x : pokerHand) {
			if (x.isSelect) activeAffect(x, 10);
		}
		int val = gameData.applyBossHurt(selectedCards);
		currentBoss.isHurt = true;
		numberText.setDisplay(true);
		numberText.position = { uiHpText.position.x, uiHpText.position.y + uiHpText.height * 4.0f + uiHpText.size };
		numberText.text = std::to_string(val * -1);
		numberText.textColor = { 255,0,0,200 };
		numberText.size = uiHpText.size;
		bossParticle.burst(currentBoss.position.x + currentBoss.width / 2, currentBoss.position.y + currentBoss.height / 2, { currentBoss.coverColor });
		gameData.substep = plAtk::DAMAGE_WAIT;
		break;
	}
	case plAtk::DAMAGE_WAIT:
		numberText.setDisplay(false);
		currentBoss.isHurt = false;
		gameData.substep = cardSteps::JOKER;
		break;

	case cardSteps::SPADE: // SPADE
	case cardSteps::HEART: // HEART
	case cardSteps::DIAMOND: // DIAMOND
	case cardSteps::CLUB: // CLUB
	case cardSteps::JOKER: // JOKER
		if (chk(1)) {// SPADE
			Mix_PlayChannel(-1, sounds["lute1"], 0);
			currentBoss.isHurt = true;
			currentBoss.coverColor = { 0,0,32,200 };
			

			gameData.applySuitAbility(selectedCards, 1);
			playCardAnimation();
			bossParticle.burst(currentBoss.position.x + currentBoss.width / 2, currentBoss.position.y + currentBoss.height / 2, { currentBoss.coverColor });
		}
		else if (chk(2)) {// HEART
			int cnt = gameData.applySuitAbility(selectedCards, 2);
			if (cnt > 0) Mix_PlayChannel(-1, sounds["card_draw"], cnt - 1);
			Mix_PlayChannel(-1, sounds["lute2"], 0);
			recyclingCards.clear();
			recyclingCards.resize(cnt);
			playCardAnimation();
		}
		else if (chk(3)) {// DIAMOND
			cardAdjustAnimation();
			int cnt = gameData.applySuitAbility(selectedCards, 3);
			if (cnt > 0) Mix_PlayChannel(-1, sounds["card_draw"], cnt - 1);
			Mix_PlayChannel(-1, sounds["lute3"], 0);
			applyGameData();
			playCardAnimation();
		}
		else if (chk(4)) {// CLUB
			Mix_PlayChannel(-1, sounds["lute4"], 0);
			currentBoss.isHurt = true;
			currentBoss.coverColor = { 255,0,0,160 };
			gameData.applySuitAbility(selectedCards, 4);
			playCardAnimation();
			bossParticle.burst(currentBoss.position.x + currentBoss.width / 2, currentBoss.position.y + currentBoss.height / 2, { currentBoss.coverColor });
		}
		else if (chk(0)) { // JOKER
			Mix_PlayChannel(-1, sounds["lute0"], 0);
			currentBoss.isHurt = true;
			currentBoss.coverColor = { 255,255,255,160 };
			gameData.applySuitAbility(selectedCards, 0);
			playCardAnimation();
		}
		++gameData.substep;
		break;

	case cardSteps::SPADE + 1:
	case cardSteps::HEART + 1:
	case cardSteps::DIAMOND + 1:
	case cardSteps::CLUB + 1:
	case cardSteps::JOKER + 1:
		currentBoss.isHurt = false;
		applyGameData();
		++gameData.substep;
		break;
	case plAtk::DISCARD_FLY:
		recyclingCards.clear();
		discardAnimation();
		Mix_PlayChannel(-1, sounds["paging"], 0);
		gameData.substep = plAtk::APPLY_PLAY;
		break;
	case plAtk::APPLY_PLAY:
		gameData.applyPlayCard(gameData.playerHand, selectedCards);
		applyGameData();
		cardAdjustAnimation();
		gameData.substep = plAtk::FINISH;
		break;
	case plAtk::FINISH:
		gameData.substep = 1; // seeds next phase: plDef::SETUP if going BOSS_OFFENSIVE; unused by PLAYER_DEALING
		if (gameData.dealingTimesCnt > 1) {
			gameData.dealingTimesCnt -= 1;
			GameSave::saveGame(gameData);
			gameData.currentStatus = PLAYER_DEALING;
		}
		else {
			GameSave::saveGame(gameData);
			gameData.currentStatus = BOSS_OFFENSIVE;
		}
		break;
	}
}

// ---- bossOffensive (BOSS_OFFENSIVE) substep ladder ----
//  SETUP(1)      -> decide whether defense is needed: boss dead / used Joker / hand < atk
//                   jumps to BOSS_UPDATE / GAME_END; otherwise -> WAIT_INPUT(2)
//  WAIT_INPUT(2) -> stay until the player picks cards and presses defense -> PLAYER_DISCARD
void SceneMain::bossOffensive() {
	int sum = gameData.calcDeckSum(gameData.playerHand);
	switch (gameData.substep) {
	case plDef::SETUP:
		gameData.dealingTimesCnt = gameData.dealingTimes;
		if (gameData.currentBoss.bossHp <= 0 || gameData.currentBoss.bossAtk == 0) {
			gameData.currentStatus = BOSS_UPDATE;
			gameData.substep = plBossUpd::PASSIVE;
			break;
		}

		if (gameData.currentBoss.useJoker) {
			gameData.currentBoss.useJoker = false;
			gameData.currentStatus = BOSS_UPDATE;
			gameData.substep = plBossUpd::PASSIVE;
			break;
		}
		if (sum < gameData.currentBoss.bossAtk || (gameData.currentBoss.bossAtk == 0 && sum == 0)) {
			gameData.currentStatus = GAME_END;
			gameData.substep = plEnd::ANIM;
			break;
		}
		if (gameData.deathFortune > gameData.checkRand() % 100) {
			// DOTO: Death Animation
			gameData.undeath = true;
			Mix_PlayChannel(-1, sounds["XIII_Death"], 0);
		}
		defenseBtn.setDisplay(true);
		defenseBtn.setEnabled(false);
		attackBtn.setDisplay(false);
		skipBtn.setDisplay(false);
		bossOffensiveAnimation();
		gameData.substep = plDef::WAIT_INPUT;
		break;
	case plDef::WAIT_INPUT:
		if (gameData.checkDiscard(selectedCards)) {
			defenseBtn.setEnabled(true);
		}
		else {
			defenseBtn.setEnabled(false);
		}
		break;
	}
}

// ---- playerDiscard (PLAYER_DISCARD) substep ladder ----
//  FLY(1)    -> discard animation (+ optional spade weaken) -> SETTLE(2)
//  SETTLE(2) -> applyDiscard, refresh hand -> NEXT(3)
//  NEXT(3)   -> save; hand too small (deathHandCnt) -> GAME_END else -> BOSS_UPDATE
void SceneMain::playerDiscard() {
	switch (gameData.substep) {
	case plDis::FLY:
		playerParticle.burst(game.getWindowWidth() * 0.5f, game.getWindowHeight() * handLimitTop, { { 255,255,255,128 } });
		bossBackAnimation();
		discardAnimation();
		Mix_PlayChannel(-1, sounds["paging_long"], 0);
		gameData.substep = plDis::SETTLE;
		if (gameData.discardSpade != 0 && !gameData.currentBoss.isImmune(1)) {
			weakenedAffect(currentBoss, 5);
			activeAffect(currentBoss, 5);
			Mix_PlayChannel(-1, sounds["lute1"], 0);
			currentBoss.coverColor = { 0,0,32,200 };
			currentBoss.isHurt = true;
		}
		break;
	case plDis::SETTLE:
		currentBoss.isHurt = false;
		gameData.applyDiscard(gameData.playerHand, selectedCards);
		applyGameData();
		cardAdjustAnimation();
		gameData.substep = plDis::NEXT;
		break;
	case plDis::NEXT:
		gameData.substep = 1; // seeds GAME_END(plEnd::ANIM) or BOSS_UPDATE(plBossUpd::PASSIVE)
		GameSave::saveGame(gameData);
		if (gameData.deathHandCnt != 0 && gameData.playerHand.size() < gameData.deathHandCnt) {
			gameData.currentStatus = GAME_END;
		}
		else {
			gameData.currentStatus = BOSS_UPDATE;
		}
		break;
	}
}

// ---- bossUpdate (BOSS_UPDATE) substep ladder ----
//  PASSIVE(1)      -> per-round passives (Temperance redraw, Pride revival) -> CHECK_DEAD(1001)
//  CHECK_DEAD(1001)-> boss dead? capture/kill (Judgement recycle+draw) -> TAROT_STEP(2);
//                     alive -> applyNewRound + save -> PLAYER_DEALING
//  TAROT_STEP(2)   -> tarotCounter--; if 0 -> TAROT_CHOOSE else -> SPAWN_NEXT(3)
//  SPAWN_NEXT(3)   -> spawn next boss (applyNewBoss+applyNewRound) -> NEXT_TURN(4)
//  NEXT_TURN(4)    -> save + Magician penalty sfx -> back to PASSIVE(1) / PLAYER_DEALING
void SceneMain::bossUpdate() {
	switch (gameData.substep) {
	case plBossUpd::PASSIVE:
		if (gameData.temperanceCnt != 0) {
			Mix_PlayChannel(-1, sounds["lute2"], 0);
			Mix_PlayChannel(-1, sounds["lute3"], 0);
			gameData.drawCards(gameData.temperanceCnt);
			recyclingCards.clear();
			recyclingCards.resize(gameData.recycleCards(gameData.temperanceCnt));
			Mix_PlayChannel(-1, sounds["card_draw"], 0);
			applyGameData();
			cardAdjustAnimation();
			recycleAnimation();
		}

		if (gameData.checkSin(gameData.currentBoss, 7) && !gameData.currentBoss.isCapture(gameData.isJudgement && gameData.judgeLevel > 0) && gameData.currentBoss.bossHp <= 0) { //傲慢
			if (gameData.currentBoss.prideTimes == 0) {
				int HP = gameData.currentBoss.bossHp;
				gameData.currentBoss.bossHp += -HP * 7;
				gameData.currentBoss.bossAtk += -HP * 3;
				gameData.currentBoss.prideTimes += 1;
			}
		}
		applyGameData();
		gameData.substep = plBossUpd::CHECK_DEAD;
		break;
	case plBossUpd::CHECK_DEAD:
		if (gameData.currentBoss.bossHp <= 0) {
			if (gameData.currentBoss.isCapture(gameData.isJudgement && gameData.judgeLevel > 0)) {
				Mix_PlayChannel(-1, sounds["lyre_sol"], 0);
				if (gameData.captureTimes != 0) Mix_PlayChannel(-1, sounds["lyre_do"], 0);
			}
			else {
				Mix_PlayChannel(-1, sounds["lyre_do"], 0);
			}
			killBossAnimation(gameData.currentBoss.isCapture(gameData.isJudgement && gameData.judgeLevel > 0));
			if (gameData.isJudgement && gameData.judgeLevel != 0) {
				Mix_PlayChannel(-1, sounds["lyre_sol"], 0);
				Mix_PlayChannel(-1, sounds["lute2"], 0);
				Mix_PlayChannel(-1, sounds["lute3"], 0);
				recyclingCards.clear();
				recyclingCards.resize(gameData.recycleCards(gameData.discardPile.size()));
				gameData.drawCards(gameData.judgeLevel + 1);
				Mix_PlayChannel(-1, sounds["card_draw"], 0);
				applyGameData();
				cardAdjustAnimation();
				recycleAnimation();
			}
			applyGameData();
			gameData.substep = plBossUpd::TAROT_STEP;
		}
		else {
			gameData.applyNewRound();
			applyGameData();
			GameSave::saveGame(gameData);
			gameData.currentStatus = PLAYER_DEALING;
		}
		break;
	case plBossUpd::TAROT_STEP:
		gameData.tarotCounter -= 1;
		if (gameData.tarotCounter != 0) {
			gameData.substep = plBossUpd::SPAWN_NEXT;
		}
		else {
			gameData.substep = plTarot::PREP;
			gameData.currentStatus = TAROT_CHOOSE;
		}
		break;
	case plBossUpd::SPAWN_NEXT:
		gameData.applyNewBoss();
		gameData.applyNewRound();
		newBossAnimation();
		applyGameData();
		gameData.substep = plBossUpd::NEXT_TURN;
		break;
	case plBossUpd::NEXT_TURN:
		GameSave::saveGame(gameData);
		if (gameData.badMagician) {
			gameData.badMagician = false;
			Mix_PlayChannel(-1, sounds["I_The_Magician"], 0);
		}
		gameData.substep = 1; // back to PASSIVE(1) for the next BOSS_UPDATE entry
		gameData.currentStatus = PLAYER_DEALING;
		break;
	}
}

// ---- gameEnd (GAME_END) substep ladder ----
//  ANIM(1)   -> save + defeat animation + end BGM -> SWITCH(2)
//  SWITCH(2) -> stash gameData into gameDataTemplate and switch to SceneEnd
void SceneMain::gameEnd(){
	switch (gameData.substep) {
	case plEnd::ANIM:
		GameSave::saveGame(gameData);
		endGameAnimation();
		activeAffect(currentBoss, 10);
		activeAffect(currentBoss, 10);
		activeAffect(currentBoss, 100);
		Mix_PlayMusic(game.endMusic, 0);
		gameData.substep = plEnd::SWITCH;
		break;
	case plEnd::SWITCH:
		gameData.substep = 1; // scene is about to be replaced; value is irrelevant
		auto sceneEnd = new SceneEnd();
		gameDataTemplate = gameData;
		game.changeScene(sceneEnd);
		return;
	}
}

// ---- gameStart (GAME_START) substep ladder ----
//  LOAD(1) -> start-load animation + show buttons -> DEAL(2)
//  DEAL(2) -> -> PLAYER_DEALING
void SceneMain::gameStart(){
	switch (gameData.substep) {
	case plStart::LOAD:
		GameSave::saveGame(gameData);
		attackBtn.setDisplay(true);
		skipBtn.setDisplay(true);
		skipBtn.setEnabled(false);
		defenseBtn.setDisplay(false);
		loadAnimation();
		gameData.substep = plStart::DEAL;
		break;
	case plStart::DEAL:
		gameData.substep = 1; // PLAYER_DEALING does not use substep
		gameData.currentStatus = PLAYER_DEALING;
		break;
	}
}

// ---- tarotChoose (TAROT_CHOOSE) substep ladder ----
//  PREP(1)  -> draw candidates off-screen + tarot anim -> PICK(2)
//  PICK(2)  -> layout cards, wait for the player to pick one (tarotBtn does ++substep)
//  APPLY(3) -> apply chosen tarot / suit reshuffle / Hermit penalty
//             -> back to BOSS_UPDATE(plBossUpd::SPAWN_NEXT)
void SceneMain::tarotChoose(){
	std::vector<int> choice;
	const int backSubstep = plBossUpd::SPAWN_NEXT; // leaving TAROT_CHOOSE returns to BOSS_UPDATE's SPAWN_NEXT step
	switch (gameData.substep) {
	case plTarot::PREP:
		GameSave::saveGame(gameData);
		gameData.getLove();
		gameData.tarotCounter = gameData.tarotCycle;
		tarotMask.width = game.getWindowWidth();
		tarotMask.height = game.getWindowHeight();
		tarotMask.position = { 0,0 };
		tarotBtn.setDisplay(false);
		tarotBtn.setEnabled(false);
		choice = gameData.applyTarotChoice(gameData.tarotChoiceCnt);
		tarotChoice.resize(choice.size());
		for (int i = 0; i < (int)choice.size(); ++i) {
			tarotChoice[i].num = choice[i];
			tarotChoice[i].position = { game.getWindowWidth() * 0.5f, -1.0f * tarotTemplate.height };
			tarotChoice[i].width = tarotTemplate.width;
			tarotChoice[i].height = tarotTemplate.height;
		}
		tarotTitle.position.y = 0;
		tarotAnimation();
		gameData.substep = plTarot::PICK;
		break;
	case plTarot::PICK:
		tarotBtn.setDisplay(true);
		for (int i = 0; i < (int)tarotChoice.size(); ++i) {
			tarotChoice[i].position = { (float)game.getWindowWidth() * (i + 1) / (tarotChoice.size() + 1) - tarotTemplate.width / 2,game.getWindowHeight() * tarotApplyY - tarotTemplate.height / 2 };
			tarotChoice[i].width = tarotTemplate.width;
			tarotChoice[i].height = tarotTemplate.height;
		}
		if (gameData.checkTarotChoose(selectedTarot)) {
			tarotBtn.setEnabled(true);
		}
		else {
			tarotBtn.setEnabled(false);
		}
		break;
	case plTarot::APPLY:
		for (int x : selectedTarot) gameData.applyTarot(x);
		selectedTarot.clear();
		tarotChoice.clear();
		tarotMask.width = 0;
		tarotMask.height = 0;
		tarotBtn.setDisplay(false);
		if (gameData.suitChangeAfterTarot) {
			for (CardData& x : gameData.playerHand) {
				if (x.suit != 0) {
					x.suit = gameData.checkRand() % 4 + 1;
				}
			}
		}
		if (gameData.badHermit) {
			gameData.badHermit = false;
			Mix_PlayChannel(-1, sounds["IX_The_Hermit"], 0);
		}
		std::stable_sort(gameData.playerHand.begin(), gameData.playerHand.end());
		applyGameData();
		gameData.substep = backSubstep;
		gameData.currentStatus = BOSS_UPDATE;
		break;
	}
}

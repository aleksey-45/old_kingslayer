#pragma once
#include "Scene.h"
#include "Object.h"
#include "GameData.h"
#include "UI.h"
#include "ParticleSystem.h"

#include <vector>
#include <list>
#include <array>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <map>


class Game;

class SceneMain : public Scene {
public:
	SceneMain();
	~SceneMain();
	const std::string sceneName;

	void update(float deltaTime) override;
	void render() override;
	void handleEvent(SDL_Event* event) override;
	void init() override;
	void clean() override;
	
	void calcCardTemplate();
	void calcTarotTemplate();
	void renderBossInfo();
	void renderCardInfo();
	void renderAttackInfo();
	void cardsAnimation(float deltaTime);

	bool runAnimation(float deltaTime);

	// they are in Animation.cpp

	void dealCardAnimation();
	void playCardAnimation();
	void discardAnimation();
	void cardAdjustAnimation();
	void recycleAnimation();
	void killBossAnimation(bool capture);
	void newBossAnimation();
	void endGameAnimation();
	void bossOffensiveAnimation();
	void bossBackAnimation();

	void tarotAnimation();

	void loadAnimation();

	//void cardDebugTest();
	

private:
	Game& game;
	GameData gameData;
	Mix_Music* bgm;
	std::map<std::string, Mix_Chunk*> sounds;
	int gameType = 1;
	
	const int cardWidth = 49;
	const int cardHeight = 65;
	
	Tooltip currentTip;

	SDL_Texture* uiHpImage;
	SDL_Texture* uiAtkImage;
	TextBox uiHpText;
	TextBox uiAtkText;

	Sprite startLoadMask;
	Sprite endLoadMask;
	Background backgroundImg;

	Button attackBtn,defenseBtn,skipBtn;
	
	ParticleSystem bossParticle;
	ParticleSystem playerParticle;

	Pile pileBase;
	Pile cardPile;
	int cardPileSize;
	Pile discardPile;
	int discardPileSize;
	TextBox cardPileText, discardPileText,handText;
	TextBox atkPower, defPower, restPower;
	TextBox numberText;

	Card cardTemplate;
	Card currentBoss;
	Sprite bossShader[8];
	std::vector<CardData> selectedCards;
	std::vector<int> selectedTarot;
	std::vector<Card> pokerHand,lastHand;
	std::vector<Pile> recyclingCards;
	std::array<std::array<SDL_Rect,14>,5> cardSrcRects;

	std::array<SDL_Rect,22> tarotSrcRects;
	Tarot tarotTemplate;
	std::vector<Tarot> tarotChoice;
	Sprite tarotMask;
	Button tarotBtn;
	TextBox tarotText;
	TextBox tarotTitle;
	TextBox tempGameInfo;
	TextBox tempTarotInfo[22];
	Tooltip tarotTip;

	void applyGameData();

	void renderTips();

	void renderTarotTips();

	void playerDealing();
	void playerAttack();
	void bossOffensive();
	void playerDiscard();
	void bossUpdate();
	void gameEnd();
	void gameStart();
	void tarotChoose();

	
	float pileLeftRight = 0.2f;
	float pileBottom = 0.95f;
	float handLimitLeft = 0.25f;
	float handLimitRight = 0.75f;
	float handLimitTop = 0.75f;
	float cardSelectShift = 0.2f;
	float bossTop = 0.1f;

	float handBtnCenterX = 0.5f;
	float handBtnTop = 0.88f;

	float cardScale = 2.0f;
	float bossScale = 3.0f;
	float tarotScale = 2.5f;

	float tarotBtnTop = 0.8f;
	float cardApplyY = 0.55f;
	float tarotApplyY = 0.35f;


};

void activeAffect(Sprite& o, int sleep = 0);
void weakenedAffect(Sprite& o, int sleep = 0);

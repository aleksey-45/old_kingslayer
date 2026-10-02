#ifndef ANIMATION_H
#define ANIMATION_H

#include "Game.h"
#include "Object.h"
#include "SceneMain.h"
#include "SceneIndex.h"
#include "GameData.h"

#include <SDL.h>
#include <SDL_image.h>

void SceneMain::cardsAnimation(float deltaTime) {
	if (!game.gameSettings.cardRotate) return;
	for (int i = 0; i < (int)pokerHand.size(); ++i) {
		if (pokerHand[i].movements.empty()) {
			const float step = deltaTime * pokerHand[i].angleSpeed;
			pokerHand[i].angle += step;
			if (pokerHand[i].angle > M_PI * 2)  pokerHand[i].angle -= M_PI * 2;
		}
	}

	if (!currentBoss.movements.empty()) return;
	const float step = deltaTime * currentBoss.angleSpeed;
	currentBoss.angle += step;
	if (currentBoss.angle > M_PI * 2)  currentBoss.angle -= M_PI * 2;

	if (gameData.currentStatus == TAROT_CHOOSE) {
		for (Tarot& x : tarotChoice) {
			if (x.movements.empty()) {
				SDL_Rect cardRect = { x.position.x,x.position.y,cardTemplate.width,cardTemplate.height };
				const float step = deltaTime * x.angleSpeed;
				x.angle += step;
				if (x.angle > M_PI * 2)  x.angle -= M_PI * 2;

			}
		}
	}
}


void activeAffect(Sprite& o, int sleep) {
	std::vector<Movement> affect;
	int w = o.width;
	int h = o.width;
	affect.resize(4);
	affect[0].rotate = { -10,400.0f };
	affect[1].rotate = { 17,400.0f };
	affect[1].scale = { SDL_FPoint{-w * 0.1f,-h * 0.1f},600.0f };
	affect[2].rotate = { -17,400.0f };
	affect[2].scale = { SDL_FPoint{w * 0.12f,h * 0.12f},600.0f };
	affect[2].move = { SDL_FPoint{1.5,1.5},600.0f };
	affect[3].rotate = { 10,1000.0f };
	affect[3].sleep = sleep;
	for (int i = 0; i < affect.size(); ++i) o.movements.push(affect[i]);
}

void weakenedAffect(Sprite& o, int sleep) {
	std::vector<Movement> affect;
	int w = o.width;
	int h = o.width;
	affect.resize(2);
	//affect[0].rotate = { 17,400.0f };
	affect[0].scale = { SDL_FPoint{-w * 0.3f,-h * 0.3f},600.0f };
	affect[1].scale = { SDL_FPoint{w * 0.32f,h * 0.32f},1800.0f };
	//affect[1].rotate = { -17,400.0f };
	affect[1].move = { SDL_FPoint{1.5,1.5},600.0f };
	affect[1].sleep = sleep;
	for (int i = 0; i < affect.size(); ++i) o.movements.push(affect[i]);
}

void SceneMain::dealCardAnimation() {
	if (selectedCards.size() == 0) return;
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	int cnt = 0, j = 0;
	int w = cardTemplate.width;
	int h = cardTemplate.height;
	for (Card& x : pokerHand) if (x.isSelect) ++cnt;
	for (Card& x : pokerHand) {
		if (x.isSelect) {
			x.angle = 0;
			float shiftY = windowH * cardApplyY - x.position.y - h / 2.0f;
			float shiftX = windowW * 0.5f + (j - (cnt - 1) / 2.0f) * w * 1.2f - x.position.x - w / 2.0f;
			Movement up, wait;
			wait.sleep = j * 10;
			up.move = { SDL_FPoint{shiftX,shiftY} , 1500.0f };
			up.sleep = 10;
			x.movements.push(wait);
			x.movements.push(up);
			++j;
		}
	}


}

void SceneMain::playCardAnimation() {
	int suit[5] = { 0,0,0,0,0 };
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	for (Card& x : pokerHand) {
		if (!x.isSelect) continue;
		++suit[x.suit];
	}//RoFtaCD

	for (Card& x : pokerHand) {
		if (!x.isSelect) continue;
		if (cardSteps::suitStep[x.suit] == gameData.substep && !gameData.currentBoss.isImmune(x.suit)) {
			activeAffect(x, 20);
		}
	}
	switch (gameData.substep) {
	case cardSteps::SPADE:
		weakenedAffect(currentBoss, 20);
		break;
	case cardSteps::HEART:
		recycleAnimation();
		break;
	case cardSteps::DIAMOND:
		break;
	case cardSteps::CLUB:
		activeAffect(currentBoss, 20);
		break;
	case cardSteps::JOKER:
		activeAffect(currentBoss, 20);
		break;
	}

}


void SceneMain::discardAnimation() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	int w = cardTemplate.width;
	int h = cardTemplate.height;
	for (Card& x : pokerHand) {
		if (x.isSelect) {
			x.angle = 0;
			float shiftY = windowH * cardApplyY - x.position.y - h / 2.0f;

			Movement up;
			up.move = { SDL_FPoint{0,shiftY} , 2000.0f };
			if(gameData.currentStatus == PLAYER_DISCARD) up.sleep = 5;
			x.movements.push(up);

			Movement fly1, tmp1, tmp2, tmp3, fly2;
			fly1.move = { SDL_FPoint{discardPile.position.x - x.position.x,0} , 3000.0f };
			fly1.rotate = { 10 , 1000.0f };
			tmp1.rotate = { -20 , 500.0f };
			tmp2.rotate = { 10 , 500.0f };
			fly2.move = { SDL_FPoint{0,discardPile.position.y - x.position.y - shiftY} , 4000.0f };
			x.movements.push(fly1);
			x.movements.push(tmp1);
			x.movements.push(tmp2);
			x.movements.push(fly2);
		}
	}

}

void SceneMain::cardAdjustAnimation() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	int cnt = 0;
	for (Card& x : pokerHand) if (!x.isSelect) ++cnt;
	for (int i = 0, j = 0; j < pokerHand.size(); ++j) {
		if (pokerHand[j].isSelect) continue;
		float cardShiftY = -abs(cnt / 2.0f - i - 0.7f) * abs(cnt / 2.0f - i - 0.7f) * cardTemplate.height * 0.05f / cnt;
		int X = game.getWindowWidth() * ((handLimitRight - handLimitLeft) / (cnt + 1) * (i + 1) + handLimitLeft) - cardTemplate.width / 2.0f;
		int Y = game.getWindowHeight() * handLimitTop - cardTemplate.height / 2.0f - cardShiftY;

		Movement wait, adjust;
		wait.sleep = i;
		adjust.move = { SDL_FPoint{X - pokerHand[j].position.x,Y - pokerHand[j].position.y} , 2500.0f };
		pokerHand[j].movements.push(wait);
		pokerHand[j].movements.push(adjust);
		++i;
	}
}

void SceneMain::recycleAnimation() {
	int windowH = game.getWindowHeight();
	int i = 0;
	for (Pile& k : recyclingCards) {
		k = discardPile;
		k.texture = discardPile.texture;
		Movement wait;
		wait.sleep = 3 * i;
		Movement fly1, fly2, fly3;
		fly1.move = { SDL_FPoint{ (cardPile.position.x - k.position.x)/8.0f*3, (cardPile.position.y - k.position.y)/2.0f - windowH*0.15f} , 2500.0f };
		fly2.move = { SDL_FPoint{ (cardPile.position.x - k.position.x) / 4.0f, 0} , 3500.0f };
		fly3.move = { SDL_FPoint{ (cardPile.position.x - k.position.x)/8.0f*3, (cardPile.position.y - k.position.y)/2.0f + windowH*0.15f} , 2500.0f };
		fly3.sleep = 10;
		k.movements.push(wait);
		k.movements.push(fly1);
		k.movements.push(fly2);
		k.movements.push(fly3);
		++i;
	}

}

void SceneMain::killBossAnimation(bool capture) {
	Movement fly;
	int shiftX = currentBoss.width - cardTemplate.width;
	int shiftY = currentBoss.height - cardTemplate.height;
	if (capture) {
		fly.move = { SDL_FPoint{ -currentBoss.position.x + cardPile.position.x - shiftX, -currentBoss.position.y + cardPile.position.y - shiftY} , 2000.0f };
		fly.rotate = { 720,3000.0f };
	}
	else {
		if (gameData.difficulty == D_SKY) {
			fly.move = { SDL_FPoint{ game.getWindowWidth()*1.5f, -currentBoss.position.y + discardPile.position.y - shiftY } , 2000.0f };
		}
		else {
			fly.move = { SDL_FPoint{ -currentBoss.position.x + discardPile.position.x - shiftX, -currentBoss.position.y + discardPile.position.y - shiftY } , 2000.0f };
		}
		fly.rotate = { -720,3000.0f };
	}
	fly.scale = { SDL_FPoint{(float)-currentBoss.width+cardPile.width,(float)-currentBoss.height + cardPile.height},300.f };
	currentBoss.movements.push(fly);

}
void SceneMain::newBossAnimation() {
	Movement wait,down;
	currentBoss.width = 49 * bossScale, currentBoss.height = 65 * bossScale;
	currentBoss.position.x = game.getWindowWidth() * 0.5f - currentBoss.width / 2;
	currentBoss.position.y = -65 * bossScale;
	int Y = game.getWindowHeight() * bossTop;
	wait.sleep = 90;
	down.move = { SDL_FPoint{0,Y- currentBoss.position.y} , 800.0f };
	currentBoss.movements.push(down);
}

void SceneMain::endGameAnimation() {
	Movement down,bigger,wait;
	wait.sleep = 20;
	down.move = { SDL_FPoint{0,game.getWindowHeight()*1.0f}, 2000.0f};
	bigger.move = { SDL_FPoint{0,game.getWindowHeight() * 0.2f}, 2000.0f };
	bigger.scale = { SDL_FPoint{currentBoss.width*1.0f,currentBoss.height*1.0f}, 2000.0f };
	bigger.sleep = 5;
	endLoadMask.movements.push(down);
	currentBoss.movements.push(wait);
	currentBoss.movements.push(bigger);
}

void SceneMain::bossOffensiveAnimation(){
	Movement down;
	down.move = { SDL_FPoint{0,game.getWindowHeight() * 0.05f}, 800.0f };
	down.scale = { SDL_FPoint{currentBoss.width * 0.2f,currentBoss.height * 0.2f}, 800.0f };
	currentBoss.movements.push(down);
}

void SceneMain::bossBackAnimation() {
	Movement down;
	down.move = { SDL_FPoint{0,-game.getWindowHeight() * 0.025f}, 800.0f };
	down.scale = { SDL_FPoint{49 * bossScale - currentBoss.width,65 * bossScale - currentBoss.height}, 800.0f };
	currentBoss.movements.push(down);
}

void SceneMain::tarotAnimation(){
	int windowH = game.getWindowHeight();
	int i = 0;
	for (Tarot& k : tarotChoice) {
		Movement fly,wait;
		wait.sleep = 10 * i;
		float px = game.getWindowWidth() * (i + 1) / (tarotChoice.size() + 1) - k.width / 2;
		float py = windowH * tarotApplyY - k.height / 2;
		fly.move = { SDL_FPoint{px - k.position.x,py - k.position.y} , 3000.0f };
		fly.rotate = { 720,5000.0f };
		k.movements.push(wait);
		k.movements.push(fly);
		++i;
	}
	Movement down;
	down.move = { SDL_FPoint{0,windowH * (tarotApplyY - 0.2f)}, 1000.0f };
	tarotTitle.movements.push(down);
}

void SceneIndex::loadAnimation() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	loadMask.position = SDL_FPoint{ (float)windowW,(float)windowH };
	Movement fly;
	fly.rotate = {300,400.0f};
	fly.move = { SDL_FPoint{-windowW / 2.0f,-windowH / 2.0f}, 1000.0f};
	fly.scale = { SDL_FPoint{windowW*1.42f,windowW*1.42f}, 3500.0f};
	loadMask.movements.push(fly);
}

void SceneMain::loadAnimation() {
	int windowW = game.getWindowWidth();
	int windowH = game.getWindowHeight();
	startLoadMask.position = SDL_FPoint{ 0,0 };
	Movement fly;
	fly.rotate = { -300,400.0f };
	fly.move = { SDL_FPoint{windowW / 2.0f,windowH / 2.0f}, 1000.0f };
	fly.scale = { SDL_FPoint{-windowW * 1.42f,-windowW * 1.42f}, 3500.0f };
	startLoadMask.movements.push(fly);
}

#endif
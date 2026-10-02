#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <array>
/*
enum CardSuit{
	OTHER,SPADE,HEART,DIAMOND,CLUB
};

enum CardNum {
	JOKER,ACE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE,TEN,JACK,QUEEN,KING
};
*/

// Shared play/attack substep ids (used by SceneMain::playerAttack and Animation.cpp).
// These used to be hand-written twice as 3/5/7/9/11 -- keep them defined only here.
namespace cardSteps {
	constexpr int JOKER = 3;    // suit 0: Joker
	constexpr int SPADE = 5;    // suit 1
	constexpr int HEART = 7;    // suit 2
	constexpr int DIAMOND = 9;  // suit 3
	constexpr int CLUB = 11;    // suit 4
	constexpr int suitStep[5] = { JOKER, SPADE, HEART, DIAMOND, CLUB };  // indexed by suit
}

// Slice the per-card source rects out of cards.png (SceneMain and SceneIndex used to duplicate this).
inline void calcCardSrcRects(std::array<std::array<SDL_Rect, 14>, 5>& out, int w, int h) {
	const int tmp[5] = { 0, 2, 3, 1, 4 };
	for (int i = 1; i <= 4; ++i) {
		for (int j = 1; j <= 13; ++j) {
			out[tmp[i]][j] = SDL_Rect{ (j - 1) * w, (i - 1) * h, w, h };
		}
	}
	out[0][0] = SDL_Rect{ 0, 4 * h, w, h };
	out[0][1] = SDL_Rect{ w, 4 * h, w, h };
}

struct Movement {

	std::pair<SDL_FPoint, float> move = { {0,0} ,0 };
	std::pair<float, float> rotate = { 0,0 };
	std::pair<SDL_FPoint, float> scale = { {0,0},0 };
	int sleep = 0;

	bool isFinished() {
		float eps = 1e-2;
		return move.second < eps && rotate.second < eps && scale.second < eps;
	}
};

struct Sprite {
	SDL_Texture* texture = nullptr;
	SDL_FPoint position = { 0,0 };
	void clean() {
		if (texture != nullptr) {
			SDL_DestroyTexture(texture);
		}
	}

	int width = 0;
	int height = 0;
	float angle = 0.0f;
	float degree = 0.0f;
	bool display = true;
	std::queue<Movement> movements;
	void action(float dt) { // dt -> DeltaTime
		if (movements.empty()) return;
		Movement& cur = movements.front();
		float eps = 1e-2;
		if (cur.move.second > eps) {
			SDL_FPoint& p = cur.move.first;
			float& v = cur.move.second;
			float dis = sqrt(p.x * p.x + p.y * p.y);
			if (dis < v * dt) {
				v = 0;
				position.x += p.x;
				position.y += p.y;
				p.x = 0, p.y = 0;
			}
			else {
				SDL_FPoint s = { p.x / dis,p.y / dis };
				position.x += s.x * v * dt;
				position.y += s.y * v * dt;

				p.x -= s.x * v * dt;
				p.y -= s.y * v * dt;
			}
		}

		if (cur.rotate.second > eps) {
			float& deg = cur.rotate.first;
			float& v = cur.rotate.second;
			if (abs(deg) < v * dt) {
				degree += deg;
				deg = 0;
				v = 0;
			}
			else {
				degree += (deg > 0 ? 1 : -1) * v * dt;
				deg -= (deg > 0 ? 1 : -1) * v * dt;
			}
		}

		if (cur.scale.second > eps) {
			SDL_FPoint& p = cur.scale.first;
			float& v = cur.scale.second;
			float dis = sqrt(p.x * p.x + p.y * p.y);
			if (dis < v * dt) {
				v = 0;
				position.x -= p.x / 2.0f;
				position.y -= p.y / 2.0f;
				width += p.x;
				height += p.y;
				p.x = 0, p.y = 0;

			}
			else {
				SDL_FPoint s = { p.x / dis,p.y / dis };
				position.x -= s.x * v * dt / 2.0f;
				position.y -= s.y * v * dt / 2.0f;
				width += s.x * v * dt;
				height += s.y * v * dt;

				p.x -= s.x * v * dt;
				p.y -= s.y * v * dt;
			}
		}
		if (cur.isFinished()) {
			if (cur.sleep > 0) --cur.sleep;
			else movements.pop();
		}
	}

	void render(SDL_Point& center, SDL_Renderer* renderer) {
		if (!display) return;
		SDL_Rect rect = {position.x,position.y,width,height};
		SDL_RenderCopyEx(renderer, texture, NULL, &rect, cos(degree) * 0.2, &center, SDL_FLIP_NONE);
	}

	void setDisplay(bool o) {
		display = o;
	}
};

struct Card : Sprite{
	~Card() {
		if (texture != nullptr) {
			SDL_DestroyTexture(texture);
		}
	}
	int suit = 0;
	int num = 0;
	int sin = 0;
	int cardId = 0;
	int scale = 0;
	bool isClick = false;
	bool isSelect = false;
	bool isHover = false;
	bool isHurt = false;
	SDL_Color coverColor = { 0,0,0,0 };
	float angleSpeed = 0.7f;

	bool operator <(Card& o)const {
		return suit == o.suit ? num == o.num ? cardId < o.cardId : num < o.num : suit < o.suit;
	}
	bool operator ==(Card& o)const {
		return suit == o.suit && num == o.num && cardId == o.cardId;
	}
	bool operator >(Card& o)const {
		return suit == o.suit ? num == o.num ? cardId > o.cardId : num > o.num : suit > o.suit;
	}

	void render(SDL_Point& center,float shift,SDL_Renderer* renderer,SDL_Texture* texture,SDL_Rect& srcRect, SDL_Rect& dstRect,bool shadow = false) {
		if (!display) return;
		SDL_Rect cardRect = dstRect;
		if(isSelect) cardRect.y -= cardRect.h * shift;
		if (isClick) {
			cardRect.y -= cardRect.h * 0.1f / 2;
			cardRect.h *= 1.1f;
			cardRect.w *= 1.1f;
		}
		const float cardAngle = ::sin(angle) * 1.5f + degree;

		if (shadow) {
			const int shift = 8;
			SDL_Rect shadowRect = cardRect;
			shadowRect.y -= cardRect.h * 0.15;
			shadowRect.x -= shift;
			shadowRect.w += shift*2;
			SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, shadowRect.w, shadowRect.h);
			SDL_SetRenderTarget(renderer, texture);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			int radius = 12;
			roundedBoxRGBA(renderer, 0, 0, shadowRect.w, shadowRect.h, radius, 16, 16, 16, 60);
			SDL_SetRenderTarget(renderer, NULL);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			SDL_RenderCopyEx(renderer, texture, NULL, &shadowRect, cardAngle, &center, SDL_FLIP_NONE);
			SDL_DestroyTexture(texture);
		}

		SDL_Texture* redTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cardRect.w, cardRect.h);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_RenderCopyEx(renderer, texture, &srcRect, &cardRect, cardAngle, &center, SDL_FLIP_NONE);

		if (isHurt) { // render player attack
			SDL_SetRenderTarget(renderer, redTexture);
			SDL_SetRenderDrawColor(renderer, coverColor.r, coverColor.g, coverColor.b, coverColor.a);
			SDL_RenderFillRect(renderer, NULL);
			SDL_SetRenderTarget(renderer, NULL);
			SDL_SetTextureBlendMode(redTexture, SDL_BLENDMODE_BLEND);
			SDL_Rect redRect = { cardRect.x + 2,cardRect.y + 2,cardRect.w - 2 * 2,cardRect.h - 2 * 2 };
			SDL_RenderCopyEx(renderer, redTexture, NULL, &redRect, cardAngle, &center, SDL_FLIP_NONE);
		}

		SDL_DestroyTexture(redTexture);
	}

	void handleEvent(SDL_Event* event, float shift, Mix_Chunk* chunkUp, Mix_Chunk* chunkDown) {
		if (!display) return;
		SDL_Rect cardRect = { position.x,position.y,width,height };
		int mouseX = event->button.x;
		int mouseY = event->button.y;
		switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			if (!isSelect) {
				if (cardRect.x <= mouseX && cardRect.y <= mouseY &&
					cardRect.x + cardRect.w >= mouseX && cardRect.y + cardRect.h >= mouseY) {
					isClick = true;
					Mix_PlayChannel(-1, chunkUp, 0);
				}
			}
			else {
				if (cardRect.x <= mouseX && cardRect.y - cardRect.h * shift <= mouseY &&
					cardRect.x + cardRect.w >= mouseX && cardRect.y - cardRect.h * shift + cardRect.h >= mouseY) {
					isClick = true;
					Mix_PlayChannel(-1, chunkDown, 0);
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (isClick) {
				isSelect = !isSelect;
				isClick = false;
				break;
			}
			break;
		}
	}

};


struct Pile : Sprite{
	void render(SDL_Renderer* render, SDL_Texture* baseTexture,int cnt) {
		if (!display) return;
		SDL_Rect cardPileRect = { position.x,position.y,width,height };
		SDL_RenderCopy(render, baseTexture, NULL, &cardPileRect);
		for (int i = (cnt + 3) / 4 - 1; i >= 0; --i) {
			SDL_Rect tmpRect = cardPileRect;
			tmpRect.y += 3 * i;
			SDL_RenderCopy(render, texture, NULL, &tmpRect);
		}
	}
};

struct Tarot : Sprite {
	~Tarot() {
		if (texture != nullptr) {
			SDL_DestroyTexture(texture);
		}
	}
	int suit = 0;
	int num = 0;
	bool isClick = false;
	bool isSelect = false;
	bool isHover = false;
	float angleSpeed = 0.7f;

	bool operator <(Card& o)const {
		return suit == o.suit ? num < o.num : suit < o.suit;
	}
	bool operator ==(Card& o)const {
		return suit == o.suit && num == o.num;
	}
	bool operator >(Card& o)const {
		return suit == o.suit ? num > o.num : suit > o.suit;
	}
	
	void render(SDL_Point& center, float shift, SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect& srcRect, SDL_Rect& dstRect) {
		if (!display) return;
		SDL_Rect cardRect = dstRect;
		if (isSelect) cardRect.y -= cardRect.h * shift;
		if (isClick) {
			cardRect.y -= cardRect.h * 0.1f / 2;
			cardRect.h *= 1.1f;
			cardRect.w *= 1.1f;
		}
		const float cardAngle = sin(angle) * 2 + degree;
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_RenderCopyEx(renderer, texture, &srcRect, &cardRect, cardAngle, &center, SDL_FLIP_NONE);

	}

	void handleEvent(SDL_Event* event, float shift, Mix_Chunk* chunkUp, Mix_Chunk* chunkDown) {
		if (!display) return;
		int mouseX = event->button.x;
		int mouseY = event->button.y;
		SDL_Rect cardRect = { position.x,position.y,width,height };
		switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			if (!isSelect) {
				if (cardRect.x <= mouseX && cardRect.y <= mouseY &&
					cardRect.x + cardRect.w >= mouseX && cardRect.y + cardRect.h >= mouseY) {
					isClick = true;
					Mix_PlayChannel(-1, chunkUp, 0);
				}
			}
			else {
				if (cardRect.x <= mouseX && cardRect.y - cardRect.h * shift <= mouseY &&
					cardRect.x + cardRect.w >= mouseX && cardRect.y - cardRect.h * shift + cardRect.h >= mouseY) {
					isClick = true;
					Mix_PlayChannel(-1, chunkDown, 0);
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (isClick) {
				isSelect = !isSelect;
				isClick = false;
				break;
			}
			break;
		}
	}
};

struct TextBox : Sprite {
	std::string text;
	int size = 16;
	SDL_Color textColor = SDL_Color{ 255,255,255,255 };
	bool shadow = false;
	SDL_Point shadowShift = { 0,0 };
	SDL_Color shadowColor = SDL_Color{ 0,0,0,255 };

	void init(SDL_FPoint pos, std::string txt,int sze, SDL_Color txtColor){
		text = txt, textColor = txtColor;
		size = sze;
		position = pos;
		shadow = false;
	}
	void init(SDL_FPoint pos, std::string txt,int sze, SDL_Color txtColor, SDL_Color shadowCol, SDL_Point shadowSft){
		text = txt, textColor = txtColor, shadowShift = shadowSft, shadowColor = shadowCol;
		size = sze;
		position = pos;
		shadow = true;
	}

	~TextBox() {
		if (texture != nullptr) {
			SDL_DestroyTexture(texture);
		}
	}

	void render(SDL_Renderer* renderer,TTF_Font* font,int style = TTF_STYLE_NORMAL) {
		if (!display) return;
		auto renderText = [&](auto text, SDL_Color& col, SDL_Rect& rect) ->void {
			TTF_SetFontStyle(font, style);
			TTF_SetFontSize(font, size);
			SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), col);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
			rect.x -= rect.w / 2;
			rect.y -= rect.h / 2;
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		};
		SDL_Rect rect = {position.x,position.y,0,0}, shadowRect;
		if (shadow) {
			shadowRect = rect;
			shadowRect.x += shadowShift.x;
			shadowRect.y += shadowShift.y;
			renderText(text, shadowColor, shadowRect);
		}
		renderText(text, textColor, rect);
	}
};

struct Background {
	SDL_Texture* texture = nullptr;
	SDL_FPoint position = { 0,0 };
	void clean() {
		if (texture != nullptr) {
			SDL_DestroyTexture(texture);
		}
	}
	int width = 0;
	int height = 0;
	float degree = 0.0;
};

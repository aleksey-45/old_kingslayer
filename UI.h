#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <string>
#include <cstring>
#include <iostream>
#include <functional>
#include <vector>
#include <array>
#include <sstream>
#include <iostream>

class Button {
public:
    Button();
    void init(int x, int y, int width, int height,float scale,SDL_Renderer* renderer);

    ~Button();
    void clean();

    void handleEvent(const SDL_Event& event);

    void render();

    void setEnabled(bool enabled);

    void setDisplay(bool display);

    void setOnClickCallback(std::function<void()> callback);

    int x=0, y=0, width=0, height=0;
    float scale = 1.0f;
    SDL_Texture* texture = nullptr;
protected:
    SDL_Renderer* renderer = nullptr;
    bool isEnabled = true;
    bool isPressed = false;
    bool isHidden = false;
    std::function<void()> onClickCallback;
};


class Tooltip {
public:
    void init(SDL_Renderer* renderer_, TTF_Font* font_, int titleSze, int contentSze);
    void show(const std::string& title, const std::string& content, int x, int y, int maxWidth, bool isUp = true, int style = TTF_STYLE_BOLD);
    SDL_Color colorTitle;
    SDL_Color colorContent;
    SDL_Color colorBackground;
    SDL_Color colorBorder;
private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    int titleSize, contentSize;
};

class Menu {
public:
    std::vector<Button*> menuBtn;
    void init(int x, int y, int width, int height, SDL_Renderer* renderer);
    int x = 0, y = 0, width = 0, height = 0;
    void clean();
    void handleEvent(const SDL_Event& event);
    void render();
    void setDisplay(bool display);
    void changeBtn(int x);
    SDL_Color colorBackground = {110,120,120,120};
    SDL_Color colorInner = {50,60,60,120};
    SDL_Color colorBorder = {0,0,0,255};
private:
    int currentBtn;
    bool isHidden = false;
    SDL_Renderer* renderer = nullptr;

};

class TextBoxUI {
public:
    // 成员变量（声明，不赋值）
    SDL_Color colorBackground;
    SDL_Color colorInner;
    SDL_Color colorBorder;
    SDL_Color textColor;
    std::string text;
    int size;
    bool shadow;
    SDL_Point shadowShift;
    SDL_Color shadowColor;
    SDL_FPoint position;

    // 构造函数
    TextBoxUI();

    // 初始化方法
    void init(SDL_FPoint pos, std::string txt, int sze, SDL_Color txtColor);
    void init(SDL_FPoint pos, std::string txt, int sze, SDL_Color txtColor, SDL_Color shadowCol, SDL_Point shadowSft);

    // 渲染方法
    void render(SDL_Renderer* renderer, TTF_Font* font, int style = TTF_STYLE_NORMAL, int radius = 10);
};


class ButtonText : public Button {
public:
    ~ButtonText();
    void init(int x, int y, int width, int height, float scale, SDL_Renderer* renderer, const std::string& text, int sze, SDL_Color textColor = { 255,255,255,255 }, int textOffsetX = 0, int textOffsetY = 0);
    void render(SDL_Renderer* renderer, TTF_Font* font, int style = TTF_STYLE_NORMAL);
    int size;
    std::string text;
    int textOffsetX, textOffsetY;
    bool shadow;
    SDL_Point shadowShift;
    SDL_Color shadowColor, textColor;
};

class BoardUI {
public:
    void init(int x, int y, int width, int height, SDL_Renderer* renderer);
    int x = 0, y = 0, width = 0, height = 0;
    void render();
    void setDisplay(bool display);
    SDL_Color colorBackground = { 110,120,120,120 };
    SDL_Color colorInner = { 50,60,60,120 };
    SDL_Color colorBorder = { 0,0,0,255 };
private:
    bool isHidden = false;
    SDL_Renderer* renderer = nullptr;
};


class DropDownList {
public:
    DropDownList();
    ~DropDownList();
    void init(int x, int y, int width, int height, std::string title, SDL_Renderer* renderer, TTF_Font* font);
    void clean();
    void handleEvent(const SDL_Event& event);
    void render();
    void addItem(const std::string& item);
    void clearItems();
    void setEnabled(bool enabled);
    void setDisplay(bool display);
    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    std::string getSelectedText() const;
    std::string title;
    void setInnerBackgroundColor(SDL_Color color);
    void setTextColor(SDL_Color color);
    void setTextSize(int size);
    void setOnClickCallback(std::function<void()> callback);
private:
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    SDL_Color backgroundColor = { 110,130,120, 255 };
    SDL_Color borderColor = { 0, 0, 0, 255 };
    SDL_Color highlightColor = { 80, 90, 90, 255 };
    SDL_Color innerBackgroundColor = { 50,60,60, 120 };
    SDL_Color textColor = { 255, 255, 255, 255 };
    int textSize = 36;
    int padding = 20;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int itemHeight = 0;
    int selectedItem = -1;
    bool isDropdownOpen = false;
    bool isEnabled = true;
    bool isHidden = false;

    std::vector<std::string> items;
    std::vector<SDL_Texture*> itemTextures;
    std::function<void()> onClickCallback;
    void drawDropdown();
    void drawItems();
    void createItemTextures();
    void updateSelectedItem(int index);
};


class Slider {
public:
    Slider();
    ~Slider();
    void init(int x, int y, int width,int height, std::string title_, SDL_Renderer* renderer, TTF_Font* font);
    void clean();
    void handleEvent(const SDL_Event& event, float& val);
    void render();
    float getValue() const;
    void setValue(float value);
    void setRange(float min, float max);
	void setEnabled(bool enabled);
	void setDisplay(bool display);
	void setTextSize(int size);
    SDL_Color backgroundColor = { 200, 200, 200, 128 };
    SDL_Color foregroundColor = { 103,125,144, 255 };;
    SDL_Color thumbColor = { 63, 85, 104, 255 };
    SDL_Color textColor = { 255, 255, 255, 255 };
private:
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    std::string title;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 20;
    int thumbSize = 20;
	int textSize = 20;
    float value = 0.0f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool isDragging = false;
	bool isHidden = false;
	bool isEnabled = true;
    int thumbX = 0;

    void drawBackground();
    void drawForeground();
    void drawThumb();
    void drawText(const std::string& text, int x, int y);
    void updateThumbPosition();
};
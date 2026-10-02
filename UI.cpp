#include "UI.h"

static float SDL_clampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

namespace UIUtil {
    static void drawRoundedRectangle(SDL_Renderer* renderer, SDL_Rect& rect, SDL_Color& color, int radius, bool filled, int border_thickness = 1) {
        if (filled) {
            roundedBoxRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, radius, color.r, color.g, color.b, color.a);
            return;
        }
        for (int i = 0; i < border_thickness; i++) {
            // 每层扩展i像素，半径同步增加i像素保持比例
            roundedRectangleRGBA(
                renderer,
                rect.x - i,
                rect.y - i,
                rect.x + rect.w + i,
                rect.y + rect.h + i,
                radius + i, // 关键修改：半径随扩展层级递增
                color.r, color.g, color.b, color.a
            );
        }
    }
}

Button::Button() {
}

void Button::init(int x_, int y_, int width_, int height_, float scale_, SDL_Renderer* renderer_){
    x = x_, y = y_, width = width_, height = height_;
    renderer = renderer_;
    scale = scale_;
}

Button::~Button() {
    if (texture !=nullptr) {
        SDL_DestroyTexture(texture);
    }
}

void Button::clean() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
    }
}

void Button::handleEvent(const SDL_Event& event) {
    if (!isEnabled || isHidden) return; // 如果按钮不可用，直接返回

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    bool isMouseInside = mouseX >= x && mouseX <= x + width*scale && mouseY >= y && mouseY <= y + height*scale;

    if (isMouseInside) {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            isPressed = true;
        }
        else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            isPressed = false;
            if (onClickCallback) {
                onClickCallback(); // 调用点击回调函数
            }
        }
    }
}

void Button::render() {
    if (isHidden) return;
    SDL_Rect buttonRect = { x, y, width * scale, height * scale };
    SDL_Rect currentRect;

    if (!isEnabled) {
        currentRect = { 0,0,width,height };
    }
    else if (isPressed) {
        currentRect = { 0,height * 2 ,width,height };
    }
    else {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= x && mouseX <= x + width * scale &&
            mouseY >= y && mouseY <= y + height * scale) {
            currentRect = { 0,height * 3 ,width,height };
        }
        else {
            currentRect = { 0,height ,width,height };
        }
    }
    SDL_RenderCopy(renderer, texture, &currentRect, &buttonRect);
}

void Button::setEnabled(bool enabled) {
    isEnabled = enabled;
}

void Button::setDisplay(bool display) {
    isHidden = !display;
}

void Button::setOnClickCallback(std::function<void()> callback) {
    onClickCallback = callback;
}


void Tooltip::init(SDL_Renderer* renderer_, TTF_Font *font_, int titleSze, int contentSze){
        renderer = renderer_;
    // 初始化字体
    font = font_;
    titleSize = titleSze;
    contentSize = contentSze;
    colorTitle = { 230, 230, 230, 255 }; // 白色
    colorContent = { 42, 68, 44, 255 };
    colorBackground = { 42, 68, 44, 255 };  // 黑色
    colorBorder = { 230, 230, 230, 255 }; // 白色
}

void Tooltip::show(const std::string& title, const std::string& content, int x, int y,int maxWidth, bool isUp, int style) {
    // 渲染标题
    TTF_SetFontSize(font, titleSize);
    TTF_SetFontStyle(font, style);
    SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(font, title.c_str(), colorTitle);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    int titleWidth = titleSurface->w;
    int titleHeight = titleSurface->h;
    SDL_FreeSurface(titleSurface);

    // 渲染内容
    TTF_SetFontSize(font, contentSize);
    TTF_SetFontStyle(font, style);
    SDL_Surface* contentSurface = TTF_RenderUTF8_Solid_Wrapped(font, content.c_str(), colorContent, maxWidth);
    SDL_Texture* contentTexture = SDL_CreateTextureFromSurface(renderer, contentSurface);
    int contentWidth = contentSurface->w;
    int contentHeight = contentSurface->h;
    SDL_FreeSurface(contentSurface);

    // 计算框的大小
    int boxWidth = std::max(titleWidth, contentWidth) + 20; // 内边距
    int boxHeight = titleHeight + contentHeight + 35;      // 标题和内容之间的间距
    int boxX = x - boxWidth / 2;                           // 以 x 为中心
    int boxY = y - boxHeight;                              // 以 y 为底边
    if (!isUp) boxY += boxHeight;
    // 绘制背景和边框
    SDL_Rect backgroundRect = { boxX, boxY, boxWidth, boxHeight };
    UIUtil::drawRoundedRectangle(renderer, backgroundRect, colorBackground, 8, true);
    UIUtil::drawRoundedRectangle(renderer, backgroundRect, colorBorder, 8, false);
    backgroundRect.x += 1;
    backgroundRect.y += 1;
    backgroundRect.w -= 2;
    backgroundRect.h -= 2;
    UIUtil::drawRoundedRectangle(renderer, backgroundRect, colorBorder, 8, false);

    SDL_Rect contentBackgroundRect = { boxX + 5, boxY + 15 + titleHeight, contentWidth + 10, contentHeight + 10 };
    UIUtil::drawRoundedRectangle(renderer, contentBackgroundRect, colorBorder, 4, true);


    // 绘制标题
    int titleX = boxX + (boxWidth - titleWidth) / 2;
    SDL_Rect titleRect = { titleX, boxY + 10, titleWidth, titleHeight };
    SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);

    // 绘制内容
    SDL_Rect contentRect = { boxX + 10, boxY + 20 + titleHeight, contentWidth, contentHeight };
    SDL_RenderCopy(renderer, contentTexture, nullptr, &contentRect);

    // 释放资源
    SDL_DestroyTexture(titleTexture);
    SDL_DestroyTexture(contentTexture);
}

void Menu::init(int x_, int y_, int width_, int height_, SDL_Renderer* renderer_){
    x = x_, y = y_, width = width_, height = height_;
    renderer = renderer_;
}

void Menu::clean(){
    for (Button* x : menuBtn) x->clean();
}

void Menu::handleEvent(const SDL_Event& event) {
    for (Button* x : menuBtn) {
        x->handleEvent(event);
    }
}

void Menu::render() {
    const int margin = 20,padding = 20;
    SDL_Rect rect = { x,y,width,height-12 };
    SDL_Rect rectInner = { x + padding / 2,y + padding / 2,width - padding,height - 12 - padding };
    SDL_Rect rectShadow = { x,y,width,height };
    UIUtil::drawRoundedRectangle(renderer, rectShadow, colorBackground, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rectShadow, colorBorder, 10, false,3);
    UIUtil::drawRoundedRectangle(renderer, rect, colorBackground, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rectInner, colorInner, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rect, colorBorder, 10, false,3);
    
    int lastY = y + margin;
    int lastX = x + margin;
    for (int i = 0; i < menuBtn.size(); ++i) {
        menuBtn[i]->x = lastX;
        menuBtn[i]->y = lastY;
        menuBtn[i]->render();
        lastY += menuBtn[i]->height * menuBtn[i]->scale + margin;
    }
}

void Menu::setDisplay(bool display) {
    isHidden = !display;
}

void Menu::changeBtn(int x){
    for (int i = 0; i < menuBtn.size(); ++i) {
        menuBtn[i]->setEnabled(true);
    }
    menuBtn[x]->setEnabled(false);
}


TextBoxUI::TextBoxUI() :
    colorBackground({ 110, 120, 120, 120 }),
    colorInner({ 50, 60, 60, 120 }),
    colorBorder({ 0, 0, 0, 255 }),
    textColor({ 255, 255, 255, 255 }),
    size(16),
    shadow(false),
    shadowShift({ 0, 0 }),
    shadowColor({ 0, 0, 0, 255 }),
    position({ 0, 0 }) {
}

void TextBoxUI::init(SDL_FPoint pos, std::string txt, int sze, SDL_Color txtColor) {
    text = txt;
    textColor = txtColor;
    size = sze;
    position = pos;
    shadow = false;
}

void TextBoxUI::init(SDL_FPoint pos, std::string txt, int sze, SDL_Color txtColor, SDL_Color shadowCol, SDL_Point shadowSft) {
    text = txt;
    textColor = txtColor;
    size = sze;
    position = pos;
    shadow = true;
    shadowShift = shadowSft;
    shadowColor = shadowCol;
}

void TextBoxUI::render(SDL_Renderer* renderer, TTF_Font* font, int style, int radius) {
    // 计算文本区域
    TTF_SetFontSize(font, size);
    TTF_SetFontStyle(font, style);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect;
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);

    int x = position.x - textRect.w / 2;
    int y = position.y - textRect.h / 2;

    // 圆角矩形的区域
    const int margin = 5, padding = 35;
    SDL_Rect rect = { x,y,textRect.w + padding,textRect.h + padding };
    SDL_Rect rectShadow = { x,y,textRect.w + padding,textRect.h + margin * 2 + padding };
    SDL_Rect rectInner = { x + padding / 2,y + padding / 2,textRect.w,textRect.h };

    UIUtil::drawRoundedRectangle(renderer, rectShadow, colorBackground, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rectShadow, colorBorder, 10, false, 3);
    UIUtil::drawRoundedRectangle(renderer, rect, colorBackground, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rectInner, colorInner, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rect, colorBorder, 10, false, 3);

    // 文字居中
    textRect.x = rect.x + (rect.w - textRect.w) / 2;
    textRect.y = rect.y + (rect.h - textRect.h) / 2;

    // 绘制阴影（如果有）
    if (shadow) {
        SDL_Rect shadowRect = textRect;
        shadowRect.x += shadowShift.x;
        shadowRect.y += shadowShift.y;

        // 绘制阴影文本
        SDL_Surface* surfaceS = TTF_RenderUTF8_Blended(font, text.c_str(), shadowColor);
        SDL_Texture* textureS = SDL_CreateTextureFromSurface(renderer, surfaceS);
        SDL_RenderCopy(renderer, textureS, NULL, &shadowRect);

        SDL_FreeSurface(surfaceS);
        SDL_DestroyTexture(textureS);
    }
    // 绘制文本
    SDL_RenderCopy(renderer, texture, NULL, &textRect);

    // 清理资源
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

ButtonText::~ButtonText() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
    }
}

void ButtonText::init(int x_, int y_, int width_, int height_, float scale_, SDL_Renderer* renderer_, const std::string& txt,int sze, SDL_Color txtColor, int textOffsetX, int textOffsetY) {
    x = x_, y = y_, width = width_, height = height_;
    renderer = renderer_;
    scale = scale_;
    text = txt;
    textColor = txtColor;
    size = sze;
}

void ButtonText::render(SDL_Renderer* renderer, TTF_Font* font, int style) {
    if (isHidden) return;
    SDL_Rect buttonRect = { x, y, width * scale, height * scale };
    SDL_Rect currentRect;

    if (!isEnabled) {
        currentRect = { 0,0,width,height };
    }
    else if (isPressed) {
        currentRect = { 0,height * 2 ,width,height };
    }
    else {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= x && mouseX <= x + width * scale &&
            mouseY >= y && mouseY <= y + height * scale) {
            currentRect = { 0,height * 3 ,width,height };
        }
        else {
            currentRect = { 0,height ,width,height };
        }
    }
    SDL_RenderCopy(renderer, texture, &currentRect, &buttonRect);


    TTF_SetFontSize(font, size);
    TTF_SetFontStyle(font, style);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect;
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);

    textRect.x = buttonRect.x + (buttonRect.w - textRect.w) / 2;
    if (!isEnabled || isPressed) {
        textRect.y = buttonRect.y + (buttonRect.h - textRect.h) / 2 + textOffsetY * scale;
    }
    else {
        textRect.y = buttonRect.y + (buttonRect.h - textRect.h) / 2 ;
    }

    // 绘制阴影（如果有）
    if (shadow) {
        SDL_Rect shadowRect = textRect;
        shadowRect.x += shadowShift.x;
        shadowRect.y += shadowShift.y;

        // 绘制阴影文本
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), shadowColor);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, NULL, &shadowRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

}

void BoardUI::init(int x_, int y_, int width_, int height_, SDL_Renderer* renderer_){
    x = x_, y = y_, width = width_, height = height_;
    renderer = renderer_;
}

void BoardUI::render() {
    const int margin = 20, padding = 20;
    SDL_Rect rect = { x,y,width,height - 12 };
    SDL_Rect rectInner = { x + padding / 2,y + padding / 2,width - padding,height - 12 - padding };
    SDL_Rect rectShadow = { x,y,width,height };
    UIUtil::drawRoundedRectangle(renderer, rectShadow, colorBackground, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rectShadow, colorBorder, 10, false, 3);
    UIUtil::drawRoundedRectangle(renderer, rect, colorBackground, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rectInner, colorInner, 10, true);
    UIUtil::drawRoundedRectangle(renderer, rect, colorBorder, 10, false, 3);
}

void BoardUI::setDisplay(bool display) {
    isHidden = !display;
}



DropDownList::DropDownList() {
}

DropDownList::~DropDownList() {
    clean();
}

void DropDownList::init(int x_, int y_, int width_, int height_,std::string title_, SDL_Renderer* renderer_, TTF_Font* font_) {
    x = x_;
    y = y_;
    width = width_;
    height = height_;
    renderer = renderer_;
    font = font_;
    title = title_;

    itemHeight = height;
    selectedItem = -1;
    isDropdownOpen = false;

    TTF_SetFontSize(font, textSize);
}

void DropDownList::clean() {
    for (auto texture : itemTextures) {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
        }
    }
    itemTextures.clear();
}

void DropDownList::handleEvent(const SDL_Event& event) {
    if (!isEnabled || isHidden) return;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // 检查是否点击了下拉按钮
    bool isMouseInsideButton = mouseX >= x && mouseX <= x + width &&
        mouseY >= y && mouseY <= y + height;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (isMouseInsideButton) {
            isDropdownOpen = !isDropdownOpen;
            createItemTextures();
            onClickCallback();
        }
        else if (isDropdownOpen) {
            // 检查是否点击了某个选项
            for (size_t i = 0; i < items.size(); ++i) {
                int itemY = y + height + i * itemHeight;
                if (mouseX >= x && mouseX <= x + width &&
                    mouseY >= itemY && mouseY <= itemY + itemHeight) {
                    updateSelectedItem(static_cast<int>(i));
                    isDropdownOpen = false;
                    onClickCallback();
                    break;
                }
            }
            if (mouseY < y || mouseY > y + height + items.size() * itemHeight) {
                isDropdownOpen = false;
                onClickCallback();
            }
        }
    }
}

void DropDownList::render() {
    if (isHidden) return;

    drawDropdown();
    if (isDropdownOpen) {
        drawItems();
    }
}

void DropDownList::addItem(const std::string& item) {
    items.push_back(item);
    createItemTextures();
}

void DropDownList::clearItems() {
    items.clear();
    itemTextures.clear();
}

void DropDownList::setEnabled(bool enabled) {
    isEnabled = enabled;
}

void DropDownList::setDisplay(bool display) {
    isHidden = !display;
}

void DropDownList::setSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
        selectedItem = index;
    }
}

int DropDownList::getSelectedIndex() const {
    return selectedItem;
}

std::string DropDownList::getSelectedText() const {
    if (selectedItem >= 0 && selectedItem < static_cast<int>(items.size())) {
        return items[selectedItem];
    }
    return "";
}

void DropDownList::setInnerBackgroundColor(SDL_Color color) {
    innerBackgroundColor = color;
}

void DropDownList::setTextColor(SDL_Color color) {
    textColor = color;
}

void DropDownList::setTextSize(int size) {
    textSize = size;
    TTF_SetFontSize(font, textSize);
    createItemTextures();
}

void DropDownList::drawDropdown() {
    SDL_Rect dropdownRect = { x, y, width, height };
    SDL_Rect dropdownRectShadow = { x, y + 10, width, height };
    UIUtil::drawRoundedRectangle(renderer, dropdownRectShadow, backgroundColor, 10, true);
    UIUtil::drawRoundedRectangle(renderer, dropdownRectShadow, borderColor, 10, false, 2);
    UIUtil::drawRoundedRectangle(renderer, dropdownRect, backgroundColor, 10, true);
    UIUtil::drawRoundedRectangle(renderer, dropdownRect, borderColor, 10, false, 2);

    // 绘制内层圆角矩形
    SDL_Rect innerRect = { x + padding/2, y + padding/2, width - padding, height - padding };
    UIUtil::drawRoundedRectangle(renderer, innerRect, innerBackgroundColor, 10, true);

    if (selectedItem >= 0 && selectedItem < static_cast<int>(items.size())) {
        // 绘制选中的选项文本
        TTF_SetFontSize(font, textSize);
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, items[selectedItem].c_str(), textColor);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect;
        SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);
        textRect.x = x + (width - textRect.w) / 2;
        textRect.y = y + (height - textRect.h) / 2;
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(textSurface);
    }

    TTF_SetFontSize(font, textSize);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, title.c_str(), textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect;
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = x + (width - textRect.w) / 2;
    textRect.y = y - (height - textRect.h * 0.5f) / 2;
    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(textSurface);
}

void DropDownList::drawItems() {
    for (size_t i = 0; i < items.size(); ++i) {
        int itemY = y + height + i * itemHeight;
        SDL_Rect itemRect = { x, itemY, width, itemHeight };

        // 绘制外层圆角矩形
        UIUtil::drawRoundedRectangle(renderer, itemRect, backgroundColor, 10, true);
        UIUtil::drawRoundedRectangle(renderer, itemRect, borderColor, 10, false, 2);

        // 绘制内层圆角矩形
        SDL_Rect innerRect = { x + padding/2, itemY + padding/2, width - padding, itemHeight - padding };
        UIUtil::drawRoundedRectangle(renderer, innerRect, innerBackgroundColor, 10, true);

        // 绘制高亮
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= x && mouseX <= x + width &&
            mouseY >= itemY && mouseY <= itemY + itemHeight) {
            UIUtil::drawRoundedRectangle(renderer, innerRect, highlightColor, 10, true);
        }

        // 绘制文本
        if (i < itemTextures.size() && itemTextures[i] != nullptr) {
            TTF_SetFontSize(font, textSize);
            SDL_Rect textRect;
            SDL_QueryTexture(itemTextures[i], NULL, NULL, &textRect.w, &textRect.h);
            textRect.x = x + (width - textRect.w) / 2;
            textRect.y = (i + 1) * height + y + (height - textRect.h) / 2;
            SDL_RenderCopy(renderer, itemTextures[i], nullptr, &textRect);
        }
    }
}

void DropDownList::createItemTextures() {
    for (auto texture : itemTextures) {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
        }
    }
    itemTextures.clear();

    for (const auto& item : items) {
        TTF_SetFontSize(font, textSize);
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, item.c_str(), textColor);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
        itemTextures.push_back(texture);
    }
}

void DropDownList::updateSelectedItem(int index) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
        selectedItem = index;
    }
}
void DropDownList::setOnClickCallback(std::function<void()> callback) {
    onClickCallback = callback;
}


Slider::Slider() {
}

Slider::~Slider() {
    clean();
}

void Slider::init(int x_, int y_, int width_,int height_,std::string title_, SDL_Renderer* renderer_, TTF_Font* font_) {
    x = x_;
    y = y_;
    width = width_;
    renderer = renderer_;
    font = font_;
    height = height_;
    thumbSize = height_;
    title = title_;
    value = 0.5f;
    minValue = 0.0f;
    maxValue = 1.0f;
    isDragging = false;
    updateThumbPosition();
}

void Slider::clean() {
    // 清理资源
}

void Slider::handleEvent(const SDL_Event& event,float &val) {
	if (!isEnabled || isHidden) return;
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // 检查是否点击了滑块
    bool isMouseInsideThumb = mouseX >= thumbX - thumbSize / 2 && mouseX <= thumbX + thumbSize / 2 &&
        mouseY >= y - thumbSize / 2 && mouseY <= y + thumbSize / 2;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (isMouseInsideThumb) {
            isDragging = true;
        }
        else if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height) {
            // 点击滑道直接移动滑块
            value = static_cast<float>(mouseX - x) / static_cast<float>(width);
            value = (value < 0.0f) ? 0.0f : (value > 1.0f ? 1.0f : value);
            val = value;
            updateThumbPosition();
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        isDragging = false;
    }
    else if (event.type == SDL_MOUSEMOTION && isDragging) {
        // 拖动滑块
        value = static_cast<float>(mouseX - x) / static_cast<float>(width);
        value = (value < 0.0f) ? 0.0f : (value > 1.0f ? 1.0f : value);
		val = value;
        updateThumbPosition();
    }
}

void Slider::render() {
	if (isHidden) return;
    drawBackground();
    drawForeground();
    drawThumb();
    drawText(std::to_string(static_cast<int>(value * 100)), x + width + 10, y - 5);
    drawText(title, x + 20, y - height - 5);
}

float Slider::getValue() const {
    return value * (maxValue - minValue) + minValue;
}

void Slider::setValue(float value_) {
    value = (value_ - minValue) / (maxValue - minValue);
    value = (value < 0.0f) ? 0.0f : (value > 1.0f ? 1.0f : value);
    updateThumbPosition();
}

void Slider::setRange(float min, float max) {
    minValue = min;
    maxValue = max;
}

void Slider::drawBackground() {
    SDL_Rect backgroundRect = { x, y - height / 2, width, height };
    UIUtil::drawRoundedRectangle(renderer, backgroundRect, backgroundColor, 5, true);
	SDL_Color borderColor = { 0, 0, 0, 255 };
    UIUtil::drawRoundedRectangle(renderer, backgroundRect, borderColor, 5, false, 3);
}

void Slider::drawForeground() {
    SDL_Rect foregroundRect = { x, y - height / 2, thumbX-x, height };
    UIUtil::drawRoundedRectangle(renderer, foregroundRect, foregroundColor, 5, true);
}

void Slider::drawThumb() {
    SDL_Rect thumbRect = { thumbX - thumbSize / 2, y - thumbSize / 2, thumbSize, thumbSize };
    UIUtil::drawRoundedRectangle(renderer, thumbRect, thumbColor, 5, true);
    SDL_Color borderColor = { 0, 0, 0, 255 };
}

void Slider::drawText(const std::string& text, int x, int y) {
    TTF_SetFontSize(font, textSize);
	TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { x + 5, y - height / 2 + 2, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(texture);
}

void Slider::updateThumbPosition() {
	int margin = thumbSize / 2;
    thumbX = x + margin + static_cast<int>(value * (width - margin * 2));
}

void Slider::setEnabled(bool enabled) {
	isEnabled = enabled;
}

void Slider::setDisplay(bool display) {
	isHidden = !display;
}

void Slider::setTextSize(int size) {
	textSize = size;
}
#pragma once

#include "Scene.h"
#include "Lang.h"
#include "json.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <filesystem>

using json = nlohmann::json;

struct GameSettings {
    int screenWidth = 1600;
    int screenHeight = 900;
    int FPS = 80;
    float bgmVolume = 0.5f;
    float sfxVolume = 0.5f;
    bool fullscreen = false;
	bool cardRotate = true;
	bool isParticle = true;
	bool CRT = true;
    std::string language = "zh_CN";

    // 保存设置到文件
    void save(const std::string& path) {
        json j;
        j["screenWidth"] = screenWidth;
        j["screenHeight"] = screenHeight;
        j["fullscreen"] = fullscreen;
        j["language"] = language;
        j["FPS"] = FPS;
		j["bgmVolume"] = bgmVolume;
		j["sfxVolume"] = sfxVolume;
		j["cardRotate"] = cardRotate;
		j["CRT"] = CRT;
		j["isParticle"] = isParticle;

        // 确保目录存在
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());

        // 将 JSON 数据写入文件
        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4); // 使用缩进格式化输出
            file.close();
            std::cout << "Settings saved to " << path << std::endl;
        }
        else {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
        }
    }

    // 从文件加载设置
    bool load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "Settings file not found. Creating with default values." << std::endl;
            save(path); // 文件不存在，保存默认设置
            return false;
        }

        json j;
        try {
            file >> j;
            screenWidth = j.value("screenWidth", 800);
            screenHeight = j.value("screenHeight", 600);
            fullscreen = j.value("fullscreen", false);
            language = j.value("language", "zh_CN");
            FPS = j.value("FPS", 80);
			bgmVolume = j.value("bgmVolume", 0.5f);
			sfxVolume = j.value("sfxVolume", 0.5f);
			cardRotate = j.value("cardRotate", true);
			CRT = j.value("CRT", true);
			isParticle = j.value("isParticle", true);
            std::cout << "Settings loaded from " << path << std::endl;
        }
        catch (json::parse_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            save(path); // 解析失败，保存默认设置
            return false;
        }

        return true;
    }
};

class Game{
public:
	static Game& getInstance() {
		static Game instance;
		return instance;
	};
	
	~Game();
	void run();
	void init();
    void updateSetting();
	void clean();
	void changeScene(Scene* scene);

	void handleEvent(SDL_Event *event);
	void update(float deltaTime);
	void render();
	void initLang(const std::string& sceneName);
    float getWindowScale() const { return (windowWidth / 1600.f + windowHeight / 900.0f) / 2.0f; }
	
	SDL_Window* getWindow() { return window; }
    void setWindow(SDL_Window* x) { window = x; }
	SDL_Renderer* getRenderer() { return renderer; }
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }
	TTF_Font* textFont;
	TTF_Font* titleFont;
    GameSettings gameSettings;

	//language currentLanguage = LANG_DOWNSHEY_CN;
	std::map<std::string, std::string> translations;
    bool isRunning = true;
    Mix_Music* backgroundMusic;
    Mix_Music* endMusic;
private:
	Game();
	// delete func
	Game(const Game&) = delete;
	Game& operator =(const Game&) = delete;

	SDL_Texture* scanline = nullptr;
	Scene* currentScene = nullptr;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	int windowWidth = 1600;//1280
	int windowHeight = 900;//720
	int FPS = 80;
	Uint32 frameTime = 0;
	float deltaTime = 0.0f;
};

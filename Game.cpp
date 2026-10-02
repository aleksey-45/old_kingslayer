#include "Game.h"
#include "SceneMain.h"
#include "SceneIndex.h"
#include "CRT.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include <GL/gl.h>

Game::Game() {
}

Game::~Game() {
	clean();
}

void Game::run() { 
	scanline = create_scanline_texture(renderer, windowHeight);
	while (isRunning) {
		auto frameStart = SDL_GetTicks();
		SDL_Event event;
		handleEvent(&event);
		update(deltaTime);
		render();
		auto frameEnd = SDL_GetTicks();
		auto diff = frameEnd - frameStart;
		if (diff < frameTime) {
			SDL_Delay(frameTime - diff);
			deltaTime = frameTime / 1000.0f;
		}
		else {
			deltaTime = diff / 1000.0f;

		}
	}
}


void Game::init() {
    gameSettings.load("data/settings.json");
    updateSetting();
    // SDL Initialization
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"SDL initialize failed. SDL_Error: %s\n",SDL_GetError());
        isRunning = false;
    }
    // Create window

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    if (gameSettings.fullscreen) {
        SDL_Rect rect;
        SDL_GetDisplayBounds(0, &rect);
        windowWidth = rect.w;
        windowHeight = rect.h;
        gameSettings.screenWidth = rect.w;
        gameSettings.screenHeight = rect.h;
    }

    window = SDL_CreateWindow(u8"111真的吗哥", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
    if (gameSettings.fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL window create failed. SDL_Error: %s\n", SDL_GetError());
        isRunning = false;
    }
    //SDL_GLContext context = SDL_GL_CreateContext(window);

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL renderer create failed. SDL_Error: %s\n", SDL_GetError());
        isRunning = false;
    }

    // initial SDL_image
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != (IMG_INIT_PNG | IMG_INIT_JPG)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_image could not initailize! SDL_image Error: %s\n", IMG_GetError());
        isRunning = false;
    }

    // initial SDL_ttf
    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,"SDL_ttf could not initailize! SDL_ttf Error: %s\n", TTF_GetError());
        isRunning = false;
    }
    
    // initial SDL_mixer
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_mixer could not initailize! SDL_mixer Error: %s\n", Mix_GetError());    
        isRunning = false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_mixer could not open audio! SDL_mixer Error: %s\n", Mix_GetError());
        isRunning = false;
    }

    Mix_AllocateChannels(32);
	backgroundMusic = Mix_LoadMUS("assets/audio/theme/theme1.mp3");
	endMusic = Mix_LoadMUS("assets/audio/theme/game_end.mp3");
	if (backgroundMusic == NULL) {
		printf("Mix_LoadMUS: %sn", Mix_GetError());
		isRunning = false;
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME * gameSettings.bgmVolume/3);
	Mix_Volume(-1, MIX_MAX_VOLUME * gameSettings.sfxVolume);
	Mix_PlayMusic(backgroundMusic, -1);
    textFont = TTF_OpenFont("assets/font/namidiansong.ttf", 32);
    titleFont = TTF_OpenFont("assets/font/namidiansong.ttf", 64);
    TTF_SetFontStyle(textFont, TTF_STYLE_BOLD);
    TTF_SetFontStyle(titleFont, TTF_STYLE_BOLD);

    currentScene = new SceneIndex();
    currentScene->init();
}

// ... existing code ...

void Game::updateSetting() {
	windowWidth = gameSettings.screenWidth;
	windowHeight = gameSettings.screenHeight;
	handle_resize(renderer, &scanline, windowHeight);
	FPS = gameSettings.FPS;
	frameTime = 1000 / FPS;
	gameSettings.bgmVolume = (int(gameSettings.bgmVolume * 100)) / 100.0f;
	gameSettings.sfxVolume = (int(gameSettings.sfxVolume * 100)) / 100.0f;
	Mix_VolumeMusic(MIX_MAX_VOLUME * gameSettings.bgmVolume/3);
	Mix_Volume(-1, MIX_MAX_VOLUME * gameSettings.sfxVolume);
}


void Game::clean() {
	if (currentScene != nullptr) {
		currentScene->clean();
		delete currentScene;
	}

	if (textFont != nullptr) {
		TTF_CloseFont(textFont);
	}
	if (titleFont != nullptr) {
		TTF_CloseFont(titleFont);
	}
	// clean SDL_image
	IMG_Quit();

	//claen SDL_ttf
	TTF_Quit();
	Mix_FreeMusic(backgroundMusic);
	Mix_CloseAudio();
	Mix_Quit();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Game::changeScene(Scene* scene) {
	if (currentScene != nullptr) {
		currentScene->clean();
		delete currentScene;
	}
	currentScene = scene;
	currentScene->init();

}


void Game::handleEvent(SDL_Event* event) {
	while (SDL_PollEvent(event)) {
		if (event->type == SDL_QUIT) {
			isRunning = false;
		}
		currentScene->handleEvent(event);
	}
}

void Game::update(float deltaTime) {
	currentScene->update(deltaTime);
}

void Game::render() {
	// Screen clear
	SDL_RenderClear(renderer);
	currentScene->render();
	// Show update
	if(gameSettings.CRT) SDL_RenderCopy(renderer, scanline, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Game::initLang(const std::string& sceneName) {
	try {
		std::string langCode = gameSettings.language;
		// 加载翻译文件
		translations = loadLanguageFile(langCode + "/" + sceneName).get<std::map<std::string, std::string>>();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

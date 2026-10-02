#pragma once
#include "SDL.h"
#include <string>

enum SceneStatus {
	START,
	TO_GAME,
	SCENE_WAITING,
	TO_RESTART,
	TO_HOME,
	TO_SETTING,
	SETTING_LANG,
	SETTING_VIDEO,
	SETTING_IMAGE,
	SETTING_AUDIO
};

class Scene {
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void init() = 0;
	virtual void update(float deltaTime) = 0;
	virtual void render() = 0;
	virtual void clean() = 0;
	virtual void handleEvent(SDL_Event* event) = 0;
};
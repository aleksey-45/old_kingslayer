#pragma once
#include "json.hpp"
#include "GameData.h"
#include <filesystem>
#include <iostream>
#include <fstream>



using json = nlohmann::json;

// 序列化和反序列化方法
static void to_json(json& j, const CardData& card);
static void from_json(const json& j, CardData& card);

static void to_json(json& j, const BossData& boss);
static void from_json(const json& j, BossData& boss);

static void to_json(json& j, const GameData& gameData);
static void from_json(const json& j, GameData& gameData);

class GameSave {
public:
    // 保存和加载游戏数据
    static void saveGame(const GameData& gameData, const std::string& filename = "data/save.json");
    static bool loadGame(GameData& gameData, const std::string& filename = "data/save.json");

    // 文件操作
    static void clearGameSave(const std::string& filename = "data/save.json");
private:
    static bool deleteFile(const std::string& filename);
};


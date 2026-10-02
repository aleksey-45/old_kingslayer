#include "GameSave.h"

void to_json(json& j, const RandWrapper& rng) {
	j = json{
		{"seed", rng.seed},
		{"count", rng.getCount()}
	};
}

void from_json(const json& j, RandWrapper& rng) {
	rng.seed = j["seed"];
	rng.setCount(j["count"]);
}

// 序列化 CardData
void to_json(json& j, const CardData& card) {
    j = json{
        {"suit", card.suit},
        {"num", card.num},
        {"cardId", card.cardId},
        {"sin", card.sin}
    };
}

// 反序列化 CardData
void from_json(const json& j, CardData& card) {
    card.suit = j["suit"];
    card.num = j["num"];
    card.cardId = j["cardId"];
    card.sin = j["sin"];
}

// 序列化 BossData
void to_json(json& j, const BossData& boss) {
    j = json{
        {"bossType", boss.bossType},
        {"bossHp", boss.bossHp},
        {"bossAtk", boss.bossAtk},
        {"isDisabled", boss.isDisabled},
        {"useJoker", boss.useJoker},
        {"initalHP", boss.initalHP},
        {"initalAtk", boss.initalAtk},
        {"prideTimes",boss.prideTimes}
    };
}

// 反序列化 BossData
void from_json(const json& j, BossData& boss) {
    boss.bossType = j["bossType"];
    boss.bossHp = j["bossHp"];
    boss.bossAtk = j["bossAtk"];
    boss.isDisabled = j["isDisabled"];
    boss.useJoker = j["useJoker"];
    boss.initalHP = j["initalHP"];
    boss.initalAtk = j["initalAtk"];
    boss.prideTimes = j["prideTimes"];
}

// 序列化 GameData
void to_json(json& j, const GameData& gameData) {
    j = json{
        {"currentStatus", static_cast<int>(gameData.currentStatus)},
        {"substep", gameData.substep},
        {"gameRound", gameData.gameRound},
        {"gameLoop", gameData.gameLoop},
        {"playerCnt", gameData.playerCnt},
        {"handMaxCnt", gameData.handMaxCnt},
        {"seed", gameData.seed},
        {"difficulty", static_cast<int>(gameData.difficulty)},
        {"cardPile", gameData.cardPile},
        {"discardPile", gameData.discardPile},
        {"bossPile", gameData.bossPile},
        {"playerHand", gameData.playerHand},
        {"currentBoss", gameData.currentBoss},
        {"tarotLevel", gameData.tarotLevel},
        {"tarotCounter", gameData.tarotCounter},
        {"tarotCycle", gameData.tarotCycle},
        {"tarotChoiceCnt", gameData.tarotChoiceCnt},
        {"bossHpBase", gameData.bossHpBase},
        {"bossHpShift", gameData.bossHpShift},
        {"bossAtkBase", gameData.bossAtkBase},
        {"bossAtkShift", gameData.bossAtkShift},
        {"suitHpShift", gameData.suitHpShift},
        {"suitAtkShift", gameData.suitAtkShift},
        {"bossHpMult", gameData.bossHpMult},
        {"bossAtkMult", gameData.bossAtkMult},
        {"suitHpMult", gameData.suitHpMult},
        {"suitAtkMult", gameData.suitAtkMult},
        {"finalPower", gameData.finalPower},
        {"numPower", gameData.numPower},
        {"suitPower", gameData.suitPower},
        {"bossInOrder", gameData.bossInOrder},
        {"suitChangeAfterTarot", gameData.suitChangeAfterTarot},
        {"randomHpAdd", gameData.randomHpAdd},
        {"captureTimes", gameData.captureTimes},
        {"dealingTimes", gameData.dealingTimes},
        {"dealingTimesCnt", gameData.dealingTimesCnt},
        {"comboLimit", gameData.comboLimit},
        {"discardSpade", gameData.discardSpade},
        {"deathHandCnt", gameData.deathHandCnt},
        {"deathFortune", gameData.deathFortune},
        {"temperanceCnt", gameData.temperanceCnt},
        {"badHermit", gameData.badHermit},
        {"badMagician", gameData.badMagician},
        {"isJudgement", gameData.isJudgement},
        {"hasHierophant", gameData.hasHierophant},
        {"judgeLevel", gameData.judgeLevel},
        {"loveGraph", gameData.loveGraph},
        {"nowLove", gameData.nowLove},
        {"killedBoss", gameData.killedBoss},
        {"servantCards", gameData.servantCards},
        {"lastSinBossSuit",gameData.lastSinBossSuit },
        {"lastSinBossSin",gameData.lastSinBossSin },
        {"canStraight",gameData.canStraight },
		{"pokerRand", gameData.pokerRand},
		{"tarotRand", gameData.tarotRand},
		{"checkRand", gameData.checkRand}
    };
}

// 反序列化 GameData
void from_json(const json& j, GameData& gameData) {
    gameData.currentStatus = static_cast<gameStatus>(j["currentStatus"]);
    gameData.substep = j["substep"];
    gameData.gameRound = j["gameRound"];
    gameData.gameLoop = j["gameLoop"];
    gameData.playerCnt = j["playerCnt"];
    gameData.handMaxCnt = j["handMaxCnt"];
    gameData.seed = j["seed"];
    gameData.difficulty = static_cast<gameDifficulty>(j["difficulty"]);
    gameData.cardPile = j["cardPile"];
    gameData.discardPile = j["discardPile"];
    gameData.bossPile = j["bossPile"];
    gameData.playerHand = j["playerHand"];
    gameData.currentBoss = j["currentBoss"];
    gameData.tarotLevel = j["tarotLevel"];
    gameData.tarotCounter = j["tarotCounter"];
    gameData.tarotCycle = j["tarotCycle"];
    gameData.tarotChoiceCnt = j["tarotChoiceCnt"];
    gameData.bossHpBase = j["bossHpBase"];
    gameData.bossHpShift = j["bossHpShift"];
    gameData.bossAtkBase = j["bossAtkBase"];
    gameData.bossAtkShift = j["bossAtkShift"];
    gameData.suitHpShift = j["suitHpShift"];
    gameData.suitAtkShift = j["suitAtkShift"];
    gameData.bossHpMult = j["bossHpMult"];
    gameData.bossAtkMult = j["bossAtkMult"];
    gameData.suitHpMult = j["suitHpMult"];
    gameData.suitAtkMult = j["suitAtkMult"];
    gameData.finalPower = j["finalPower"];
    gameData.numPower = j["numPower"];
    gameData.suitPower = j["suitPower"];
    gameData.bossInOrder = j["bossInOrder"];
    gameData.suitChangeAfterTarot = j["suitChangeAfterTarot"];
    gameData.randomHpAdd = j["randomHpAdd"];
    gameData.captureTimes = j["captureTimes"];
    gameData.dealingTimes = j["dealingTimes"];
    gameData.dealingTimesCnt = j["dealingTimesCnt"];
    gameData.comboLimit = j["comboLimit"];
    gameData.discardSpade = j["discardSpade"];
    gameData.deathHandCnt = j["deathHandCnt"];
    gameData.deathFortune = j["deathFortune"];
    gameData.temperanceCnt = j["temperanceCnt"];
    gameData.badHermit = j["badHermit"];
    gameData.badMagician = j["badMagician"];
    gameData.isJudgement = j["isJudgement"];
    gameData.hasHierophant = j["hasHierophant"];
    gameData.judgeLevel = j["judgeLevel"];
    gameData.loveGraph = j["loveGraph"];
    gameData.nowLove = j["nowLove"];
    gameData.killedBoss = j["killedBoss"];
    gameData.servantCards = j["servantCards"];
    gameData.lastSinBossSuit = j["lastSinBossSuit"];
    gameData.lastSinBossSin = j["lastSinBossSin"];
    gameData.canStraight = j["canStraight"];
	gameData.pokerRand = j["pokerRand"];
	gameData.tarotRand = j["tarotRand"];
	gameData.checkRand = j["checkRand"];
}

// 保存游戏数据到文件
void GameSave::saveGame(const GameData& gameData, const std::string& filename) {
    json j;
    to_json(j, gameData);
    std::ofstream file(filename);
    file << j.dump(4); // 格式化保存 JSON
    file.close();
}

// 从文件加载游戏数据
bool GameSave::loadGame(GameData& gameData, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    json j;
    file >> j;
    file.close();
    from_json(j, gameData);
    return true;
}

// 删除文件
bool GameSave::deleteFile(const std::string& filename) {
    namespace fs = std::filesystem;
    if (fs::exists(filename)) {
        if (fs::remove(filename)) {
            std::cout << "File deleted successfully: " << filename << std::endl;
            return true;
        }
        else {
            std::cerr << "Failed to delete file: " << filename << std::endl;
            return false;
        }
    }
    else {
        std::cerr << "File does not exist: " << filename << std::endl;
        return false;
    }
}

// 清除游戏存档
void GameSave::clearGameSave(const std::string& filename) {
    deleteFile(filename);
}
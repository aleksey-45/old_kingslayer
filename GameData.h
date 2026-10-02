#pragma once
#include <algorithm>
#include <random>
#include <vector>
#include <map>
#include <array>

extern int cardIdCnt;

class RandWrapper {
public:
	RandWrapper() {}
	RandWrapper(unsigned long long seed) : seed(seed), engine(seed), count(0) {}
	void init(unsigned long long seed) {
		this->seed = seed;
		engine.seed(seed);
		count = 0;
	}
	unsigned long long operator()() {
		count++;
		return engine();
	}

	int getCount() const {
		return count;
	}

	void setCount(int x) {
		count = x;
		engine.seed(seed);
		engine.discard(count);
	}
	unsigned long long seed;
private:
	std::mt19937_64 engine;
	int count;
};

struct CardData {
	int suit;
	int num;
	int cardId;
	int sin;
	CardData(int s = 0, int n = 0, int Sin = 0);
	bool operator ==(const CardData& o)const {
		return suit == o.suit && num == o.num && cardId == o.cardId;
	}
	bool operator<(const CardData& o) const {
		if (suit != o.suit) {
			return suit < o.suit;  // 先比较花色
		}
		return num == o.num ? cardId < o.cardId : num < o.num;       // 如果花色相同，再比较数字
	}
};

struct BossData {
	CardData bossType; // 11 12 13
	int bossHp, bossAtk;
	int initalHP, initalAtk;
	bool isDisabled = false;
	bool useJoker = false;
	int prideTimes = 0;
	BossData() :bossHp(0), bossAtk(0), bossType({0,0}) {}
	BossData(CardData type, int Hp, int Atk);
	bool isCapture(bool judge) const;
	bool isImmune(int x) const {
		if (isDisabled) return false;
		if (bossType.suit == x) return true;
		return false;
	}
};

enum dealingCards {
	HAVE_NUMBER_CARDS = 1,
	HAVE_FACE_CARDS = 2,
	HAVE_ACE_CARDS = 4,
	HAVE_JOKER_SMALL = 8,
	HAVE_JOKER_BIG = 16,
	HAVE_JOKERS = 8|16,
};

enum gameStatus {
	GAME_WRONG, PLAYER_DEALING, PLAYER_ATTACK, BOSS_OFFENSIVE, PLAYER_DISCARD, BOSS_UPDATE, GAME_END, GAME_START, TAROT_CHOOSE
};


enum gameDifficulty {
	D_BEAR = 1,
	D_KID = 2,
	D_TEEN = 3,
	D_MAN = 4,
	D_GUYS = 5,
	D_SKY = 6
};

const std::array<std::string, 14> numName = { "JOKER","ACE","TWO","THREE","FOUR","FIVE","SIX","SEVEN","EIGHT","NINE","TEN","JACK","QUEEN","KING"};
const std::array<std::string, 5> suitName = { "","SPADE","HEART","DIAMOND","CLUB"};
const std::array<std::string, 8> sinName = { "","glut","lust","gloom","sloth","envy","wrath","pride"};

class GameData {
public:

	GameData();
	~GameData();
	GameData& operator = (const GameData& o);

	gameStatus currentStatus;
	int substep=1;

	void init(int sd);
	RandWrapper pokerRand,tarotRand,checkRand;
	int gameRound = 0, gameLoop = 1;
	int playerCnt = 1;
	int handMaxCnt = 9;
	unsigned long long seed = 0;
	gameDifficulty difficulty = D_MAN; // BEAR KID TEEN MAN GUYS SKY (1~6)
	std::vector<CardData> cardPile, discardPile, bossPile, playerHand;
	BossData currentBoss;

	int lastSinBossSuit = 1, lastSinBossSin = 7;

	void applyNewLoop();

	void eraseCard(std::vector<CardData>& hand, const std::vector<CardData>& deck);

	bool checkSin(const BossData& boss, const int x) const;

	bool checkTarotChoose(const std::vector<int>& vec);
	
	bool checkPlayCard(std::vector<CardData> vec) const;
	void applyPlayCard(std::vector<CardData>& hand, const std::vector<CardData>& deck);

	bool checkDiscard(const std::vector<CardData>& vec) const;
	void applyDiscard(std::vector<CardData>& hand, const std::vector<CardData>& deck);

	int recycleCards(int x);
	int drawCards(int x,int deckSize=0);

	void applyNewRound();
	void applyNewBoss();
	void applyBossSin(std::vector<CardData>& bosses);

	int applyBossHurt(const std::vector<CardData>& deck);
	int applySuitAbility(const std::vector<CardData>& vec,int suit);


	void dealingMachine(std::vector<CardData>& vec, int op);
	void cardShuffle(std::vector<CardData>& vec);

	int calcBossHp(const CardData& type) const;
	int calcBossAtk(const CardData& type) const;
	int calcDeckSum(const std::vector<CardData>& vec) const;

	std::vector<int> applyTarotChoice(int cnt);
	void applyTarot(int x);

	std::string getTarotDescription(int x,std::map<std::string, std::string>& trans);

	void getLove();

	int getSinBossCount();

	std::array<int, 22> tarotLevel;
	int tarotCounter = 3, tarotCycle = 3, tarotChoiceCnt = 3;

	std::array<int, 3>  bossHpBase = { 4,6,8 }, bossHpShift = { 0,0,0 };
//#define DEBUG_MODE__WEAKEN
#ifdef DEBUG_MODE__WEAKEN
	std::array<int, 3> bossAtkBase = { 0,0,0 }, bossAtkShift = { 0,0,0 };
#else 
	std::array<int, 3> bossAtkBase = { 2,3,4 }, bossAtkShift = { 0,0,0 };
#endif
	std::array<int, 5> suitHpShift = { 0,0,0,0,0 }, suitAtkShift = { 0,0,0,0,0 };
	std::array<float, 3> bossHpMult = { 5,5,5 }, bossAtkMult = { 5,5,5 };
	std::array<float, 5> suitHpMult = { 0,1,1,1,1 }, suitAtkMult = { 0,1,1,1,1 };

	int finalPower = 0;

//#define DEBUG_MODE__POWER
#ifdef DEBUG_MODE__POWER
	const int numBase[14] = { 0,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024 };
#else 
	const int numBase[14] = { 0,1,2,3,4,5,6,7,8,9,10,10,15,20 };
#endif
	std::array<int, 14>  numPower = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	std::array<int, 5>  suitPower = { 0,0,0,0,0 };


	bool canStraight = false;
	bool bossInOrder = true; // 0 The Fool
	bool suitChangeAfterTarot = false; // 10 Wheel of Fortune
	int randomHpAdd = 0; // 1 The Magician
	int captureTimes = 0; // 11 Justice
	int dealingTimes = 1; // 15 The Devil
	int dealingTimesCnt = 1; // 15 The Devil
	int comboLimit = 10; // 1 The Magician
	int discardSpade = 0; // 2 The High Priestess
	int deathHandCnt = 0; // 2 The High Priestess
	int deathFortune = 0; // 13 Death
	bool undeath = false; // 13 Death
	int temperanceCnt = 0; // 14 Temperance
	bool badHermit = false; // 9 The Hermit
	bool badMagician = false; // 1 The Magician
	bool isJudgement = true; // 20 Judgement
	bool hasHierophant = false; // 5 The Hierophant
	int judgeLevel = 0; // 20 Judgement
	std::array<std::array<int, 5>, 5>  loveGraph ={ { {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0} } }; // 6 The Lovers
	std::pair<int,int> nowLove = { 0,0 };
	std::vector<CardData> killedBoss;
	std::map<int, bool> servantCards;

};

extern GameData gameDataTemplate;


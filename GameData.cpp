#include "GameData.h"
#include "SceneMain.h"
#include "GameSave.h"

#include <string>
#include <map>
#include <SDL.h>
#include "Lang.h"
int cardIdCnt = 0;
GameData::GameData() {
}

GameData::~GameData() {
}

GameData gameDataTemplate;

GameData& GameData::operator=(const GameData& o)
{
	currentBoss = o.currentBoss;
	gameLoop = o.gameLoop;
	gameRound = o.gameRound;
	handMaxCnt = o.handMaxCnt;
	seed = o.seed;
	return (*this);
}

void GameData::init(int sd) {
	servantCards[1] = true; // Ace
	cardIdCnt = 0;
	seed = sd;
	pokerRand.init(sd);
	tarotRand.init(sd);
	checkRand.init(sd);
	int i;
	switch (difficulty) {
	case 1:
		handMaxCnt += 1;
		for (i = 0; i <= 2; ++i) bossHpMult[i] -= 1;
		for (i = 0; i <= 2; ++i) bossAtkMult[i] -= 1;
		break;
	case 2:
		handMaxCnt += 1;
		for (i = 0; i <= 2; ++i) bossHpMult[i] -= 1;
		break;
	case 3:
		handMaxCnt += 1;
		break;
	case 4:
		break;
	case 5:
		for (i = 0; i <= 2; ++i) bossHpMult[i] += 1;
		for (i = 0; i <= 2; ++i) bossAtkMult[i] += 1;
		break;
	case 6: // SKY
		for (i = 0; i <= 2; ++i) bossHpMult[i] += 2;
		for (i = 0; i <= 2; ++i) bossAtkMult[i] += 2;
		break;
	}
	if (playerCnt <= 2) dealingMachine(cardPile, HAVE_NUMBER_CARDS | HAVE_ACE_CARDS);
	else if (playerCnt == 3) dealingMachine(cardPile, HAVE_NUMBER_CARDS | HAVE_ACE_CARDS | HAVE_JOKER_SMALL);
	else if (playerCnt == 4) dealingMachine(cardPile, HAVE_NUMBER_CARDS | HAVE_ACE_CARDS | HAVE_JOKERS);

	dealingMachine(bossPile, HAVE_FACE_CARDS);
	applyBossSin(bossPile);
	if(bossInOrder) std::stable_sort(bossPile.begin(), bossPile.end(), [&](CardData x, CardData y) {return x.num > y.num; });
	drawCards(handMaxCnt);
	// get at least one diamond
	int diamondCnt = 0;
	for (CardData x : playerHand) if (x.suit == 3) ++diamondCnt;
	if (diamondCnt == 0) {
		for (CardData& x : cardPile) if (x.suit == 3) std::swap(x, playerHand[0]);
	}
	sort(playerHand.begin(), playerHand.end());
	killedBoss.clear();
	applyNewBoss();
}

void GameData::eraseCard(std::vector<CardData>& hand, const std::vector<CardData>& deck) {
	std::map<CardData, int> deckCount;
	for (const auto& card : deck) {
		deckCount[card]++;
	}

	hand.erase(std::remove_if(hand.begin(), hand.end(), [&deckCount](const CardData& card) {
		if (deckCount[card] > 0) {
			deckCount[card]--;
			return true;
		}
		return false;
		}), hand.end());
}
bool GameData::checkSin(const BossData& boss, const int x) const{
	if (x < 0 || x>7) return false;
	if (x == boss.bossType.sin) return true;
	if (boss.bossType.sin == 5 && x == lastSinBossSin) return true;
	return false;
}
bool GameData::checkTarotChoose(const std::vector<int>& vec) {
	if (vec.size() == 1) return true;
	return false;
}

bool GameData::checkPlayCard(std::vector<CardData> vec) const{
	int sum = 0, servant = 0;
	bool allSame = true;
	for (int i = 0; i < vec.size(); ++i) {
		if (servantCards.count(vec[i].num) != 0) ++servant;
		if (i != 0 && vec[i].num != vec[i - 1].num) allSame = false;
		sum += vec[i].num; // Do you like it?
	}
	if (vec.size() == 0) return true;
	else if (vec.size() == 1) return true;
	else if (vec.size() == 2) {
		if (servant >= 1) return true;
		if (allSame && sum <= comboLimit) return true;
		return false;
	}
	else {
		if (allSame && sum <= comboLimit) return true;
		if (canStraight) {
			if (sum > comboLimit) return false;
			std::sort(vec.begin(), vec.end(), [&](CardData A, CardData B) {return A.num < B.num;});
			bool flg = 1;
			for (int i = 1;i + 1 < vec.size();i++) flg &= vec[i].num + 1 == vec[i+1].num;
			if (!flg) return false;
			if (vec[0].num + 1 == vec[1].num || vec[0].num == 1 && vec.back().num == 13) return true;
		}
		return false;
	}
	return false;
}



void GameData::applyPlayCard(std::vector<CardData>& hand, const std::vector<CardData>& deck){
	eraseCard(hand, deck);
	sort(hand.begin(), hand.end(), [&](CardData x, CardData y) {return x.suit == y.suit ? x.num < y.num : x.suit < y.suit; });
	for (const CardData& k: deck) {
		discardPile.push_back(k);
	}
}

bool GameData::checkDiscard(const std::vector<CardData>& vec) const {
	int sum = calcDeckSum(vec);
	if (undeath) return true;
	if (sum >= currentBoss.bossAtk) return true;
	return false;
}

void GameData::applyDiscard(std::vector<CardData>& hand, const std::vector<CardData>& deck) {
	eraseCard(hand, deck);
	for (CardData x : deck) {
		discardPile.push_back(x);
	}
	currentBoss.bossAtk -= deck.size() * discardSpade;
	currentBoss.bossAtk = std::max(0, currentBoss.bossAtk);
	sort(hand.begin(), hand.end(), [&](CardData x, CardData y) {return x.suit == y.suit ? x.num < y.num : x.suit < y.suit; });
	undeath = false;
}


void GameData::applyNewLoop() {
	dealingMachine(bossPile, HAVE_FACE_CARDS);
	applyBossSin(bossPile);
	if (bossInOrder) std::stable_sort(bossPile.begin(), bossPile.end(), [&](CardData x, CardData y) {return x.num > y.num; });	
	handMaxCnt += 1;
	float baseHpAdd = 1.0f;
	float multHpAdd = 1.0f;
	float baseAtkAdd = 1.0f;
	float multAtkAdd = 0.5f;
	float factor = 0.5f * gameLoop;
	for (int i = 0; i < 3; ++i) {
		bossHpBase[i] += baseHpAdd;
		bossAtkBase[i] += baseAtkAdd;
		bossHpMult[i] += multHpAdd * factor;
		bossAtkMult[i] += multAtkAdd * factor;
	}
	gameLoop += 1;
}

void GameData::applyBossSin(std::vector<CardData>& bosses) {
	const int cnt = bosses.size();
	const int sinBossCount = std::min(cnt, getSinBossCount());
	std::vector<int> arr;
	arr.resize(cnt);
	for (int i = 0; i < cnt; ++i) arr[i] = i;
	for (int i = 0; i < sinBossCount; ++i) {
		int p = pokerRand() % (cnt - i );
		std::swap(arr[i], arr[p]);
	}
	for (int i = 0; i < sinBossCount; ++i) bosses[arr[i]].sin = pokerRand() % 7 + 1;
}

void GameData::applyNewRound() {
	if (bossPile.empty()) applyNewLoop();
	gameRound += 1;
	{
		if (checkSin(currentBoss, 2)) { //色欲
			currentBoss.bossHp += currentBoss.initalHP / 5;
			currentBoss.bossAtk += currentBoss.initalAtk / 5;
		}
		if (checkSin(currentBoss, 6)) { //暴怒
			cardShuffle(playerHand);
			if (!playerHand.empty()) playerHand.pop_back();
			sort(playerHand.begin(), playerHand.end());
		}
		if (checkSin(currentBoss, 3)) { //忧郁
			for (auto& it : playerHand) {
				if (it.num > 10 && checkRand() % 3 == 0) {
					it.num = checkRand() % 10 + 1;
				}
			}
			sort(playerHand.begin(), playerHand.end());
		}
	}
}

void GameData::applyNewBoss() {
	if (killedBoss.empty() || currentBoss.bossHp <= 0) {
		if (!killedBoss.empty()) {
			if (!checkSin(currentBoss, 5) && !checkSin(currentBoss, 0)) {
				lastSinBossSin = currentBoss.bossType.sin;
				lastSinBossSuit = currentBoss.bossType.suit;
			}
			if (currentBoss.isCapture(isJudgement && judgeLevel>0)) {
				cardPile.push_back(currentBoss.bossType);
				for (int i = 1; i <= captureTimes; ++i) {
					discardPile.push_back(currentBoss.bossType);
				}
			}
			else {
				if(difficulty != D_SKY) discardPile.push_back(currentBoss.bossType);
			}
			if (isJudgement && judgeLevel != 0) {
				for (int i = 0;i <= 2;++i) bossAtkShift[i] -= 4;
				drawCards(judgeLevel);
			}
			isJudgement = true;
		}
		// so the first killedBoss is an empty object
		killedBoss.push_back(currentBoss.bossType);
		CardData nowBoss = bossPile.back();
		bossPile.pop_back();
		currentBoss = BossData(nowBoss,calcBossHp(nowBoss),calcBossAtk(nowBoss));
		currentBoss.isDisabled = false;
		if (checkRand() % 4 == 0 && randomHpAdd != 0) {
			badMagician = true;
			currentBoss.bossHp += randomHpAdd;
		}
		if (checkSin(currentBoss, 6)) { //暴怒
			currentBoss.bossAtk *= 1.25;
		}
		currentBoss.initalHP = currentBoss.bossHp;
		currentBoss.initalAtk = currentBoss.bossAtk;
	}
}


int GameData::applySuitAbility(const std::vector<CardData>& vec,int suit) {
	
	int sum = calcDeckSum(vec);
	
	if (checkSin(currentBoss, 1) && vec.size() > 1) sum *= 0.75; //暴食

	int suitCnt[5] = { 0,0,0,0,0 };
	int loveCnt[5] = { 0,0,0,0,0 };

	for (int i = 0; i < vec.size(); ++i) {
		++suitCnt[vec[i].suit];
	}
	for (int i = 1; i <= 4; ++i) {
		for (int j = 1; j <= 4; ++j) {
			if (loveGraph[i][j] != 0) ++loveCnt[j];
		}
	}

	if (suitCnt[0] > 0 && suit == 0) {
		if (checkSin(currentBoss, 7)) { ; } //傲慢
		else {
			currentBoss.isDisabled = true; // JOKER
			currentBoss.useJoker = true;
		}
	}
	else {
		if (hasHierophant) {
			int handSuitTypes = 0;
			int handSuitCnt[5] = { 0,0,0,0,0 };
			for (int i = 0; i < playerHand.size(); ++i) {
				++handSuitCnt[playerHand[i].suit];
			}
			for (int i = 1; i <= 4; ++i) {
				if (handSuitCnt[i] > 0) ++handSuitTypes;
			}
			if (handSuitTypes == 3) {
				for (int i = 1; i <= 4; ++i) if (handSuitCnt[i] == 0) {
					suitCnt[i] = 1;
					break;
				}
			}
		}

		if (suitCnt[currentBoss.bossType.suit] == 0 && loveCnt[currentBoss.bossType.suit] == 0) {
			isJudgement = false;
		}
		if ((suitCnt[1] > 0 || loveCnt[1] > 0) && (currentBoss.bossType.suit != 1 || currentBoss.isDisabled) && suit == 1) { // SPDAE
			if (loveGraph[currentBoss.bossType.suit][1] != 0 && !currentBoss.isDisabled) {
				sum /= 2;
			}
			sum = std::max(0, sum);
			int atk = currentBoss.bossAtk;
			currentBoss.bossAtk -= sum;
			currentBoss.bossAtk = std::max(0, currentBoss.bossAtk);
			return std::min(sum, atk);
		}
		if ((suitCnt[2] > 0 || loveCnt[2] > 0) && (currentBoss.bossType.suit != 2 || currentBoss.isDisabled) && suit == 2) { // HEART
			if (loveGraph[currentBoss.bossType.suit][2] != 0 && !currentBoss.isDisabled) {
				sum /= 2;
			}
			return recycleCards(sum);
		}
		if ((suitCnt[3] > 0 || loveCnt[3] > 0) && (currentBoss.bossType.suit != 3 || currentBoss.isDisabled) && suit == 3) { // DIAMOND
			if (loveGraph[currentBoss.bossType.suit][3] != 0 && !currentBoss.isDisabled) {
				sum /= 2;
			}
			return drawCards(sum, (int)vec.size());
		}
		if ((suitCnt[4] > 0 || loveCnt[4] > 0) && (currentBoss.bossType.suit != 4 || currentBoss.isDisabled) && suit == 4) { // CLUB
			if (loveGraph[currentBoss.bossType.suit][4] != 0 && !currentBoss.isDisabled) {
				sum /= 2;
			}
			currentBoss.bossHp -= sum;
			return sum;
		}
	}
	return 0;
}

CardData::CardData(int s,int n,int Sin) {
	suit = s;
	num = n;
	sin = Sin;
	cardId = ++cardIdCnt;
}

BossData::BossData(CardData type, int Hp, int Atk) {
	bossType = type;
	bossHp = Hp;
	bossAtk = Atk;
}

bool BossData::isCapture(bool judge) const{
	if (judge) return true;
	return bossHp == 0;
}

int GameData::recycleCards(int x) {
	int cnt = std::min(x,(int)discardPile.size());
	cardShuffle(discardPile);
	for (int i = 0; i < cnt; ++i) {
		cardPile.insert(cardPile.begin(), discardPile.back());
		discardPile.pop_back();
	}
	return cnt;
}

int GameData::drawCards(int x, int deckSize) {
	int cnt = std::min({ x, (int)cardPile.size(),(int)(handMaxCnt - playerHand.size() + deckSize) });
	for (int i = 0; i < cnt; ++i) {
		playerHand.push_back(cardPile.back());
		cardPile.pop_back();
	}
	return cnt;
}

int GameData::getSinBossCount() {
	switch (gameLoop) {
	case 1: return 1;
	case 2: return pokerRand() % 3 + 3;
	case 3: return pokerRand() % 4 + 4;
	case 4: return pokerRand() % 4 + 5;
	case 5: return pokerRand() % 6 + 5;
	case 6: return pokerRand() % 5 + 6;
	case 7: return pokerRand() % 7 + 6;
	case 8: return pokerRand() % 5 + 8;
	case 9: return pokerRand() % 4 + 9;
	case 10: return pokerRand() % 3 + 10;
	default: return 12;
	}
}

void GameData::dealingMachine(std::vector<CardData>& vec, int op) {
	vec.clear();
	if (op & HAVE_ACE_CARDS) {
		for (int i = 1; i <= 4; ++i) vec.push_back(CardData(i, 1));
	}
	if (op & HAVE_FACE_CARDS) {
		int nowBossCount = 0;
		for (int i = 1; i <= 4; ++i) for (int j = 0; j <= 2; ++j) {
			vec.push_back(CardData(i, 11 + j));
			++nowBossCount;
		}
	}
	if (op & HAVE_NUMBER_CARDS) {
		for (int i = 1; i <= 4; ++i) for (int j = 2; j <= 10; ++j) vec.push_back(CardData(i, j));
	}
	if (op & HAVE_JOKER_SMALL) vec.push_back(CardData(0, 0));
	if (op & HAVE_JOKER_BIG) vec.push_back(CardData(0, 1));
	cardShuffle(vec);
}


void GameData::cardShuffle(std::vector<CardData>& vec) {
	for (int i = 0; i < (int)vec.size(); ++i) {
		int p = checkRand() % (vec.size() - i) + i;
		std::swap(vec[i], vec[p]);
	}
}

int GameData::calcBossHp(const CardData& type) const {
	int x = std::max(0, (int)ceil(bossHpBase[type.num - 11] * bossHpMult[type.num - 11] * suitHpMult[type.suit] + bossHpShift[type.num - 11] + suitHpShift[type.suit]));
	if (x % 2 == 1) x += 1;
	return x;
}

int GameData::calcBossAtk(const CardData& type) const {
	return std::max(0,(int)ceil(bossAtkBase[type.num - 11] * bossAtkMult[type.num - 11] * suitAtkMult[type.suit] + bossAtkShift[type.num - 11] + suitAtkShift[type.suit]));
}

int GameData::applyBossHurt(const std::vector<CardData>& deck) {
	if (checkSin(currentBoss, 4) && checkRand() % 5 == 0) return 0; // 怠惰
	
	int sum = calcDeckSum(deck);

	if (checkSin(currentBoss, 1)) {
		if (deck.size() > 1) sum *= 0.75f; //暴食
		else currentBoss.bossAtk *= 1.2f;
	}

	currentBoss.bossHp -= sum;
	if (currentBoss.bossHp <= 0) currentBoss.isDisabled = true;
	return sum;
}

int GameData::calcDeckSum(const std::vector<CardData>& vec) const{
	int sum = 0;
	for (int i = 0; i < (int)vec.size(); ++i) {
		if (vec[i].suit != 0) sum += numBase[vec[i].num] + numPower[vec[i].num] + suitPower[vec[i].suit];
	}
	sum += finalPower;
	return sum;
}

std::vector<int> GameData::applyTarotChoice(int cnt){
	std::vector<int> vec;
	vec.resize(22);
	for (int i = 0; i < (int)vec.size(); ++i) vec[i] = i;
	for (int i = 0; i < (int)vec.size(); ++i) {
		int p = tarotRand() % (vec.size() - i) + i;
		if(i!=p) std::swap(vec[i], vec[p]);
	}
	vec.resize(cnt);
	return vec;
}

void GameData::applyTarot(int x){
	tarotLevel[x] += 1;
	int lv = tarotLevel[x];
	switch (x){
	case 0: // LEVEL %d:\n手牌上限+%d\n后续轮回BOSS乱序，每次选择随机花色BOSS攻击力倍率+0.25",
		if (lv == 1) handMaxCnt += 2;
		else handMaxCnt += 1;
		bossInOrder = false;
		suitAtkMult[checkRand() % 4 + 1] += 0.25f;
		break;
	case 1: // LEVEL % d : \n可以打出牌面小于 ? 的连招（J : 11 Q : 12 K : 13）\n后续BOSS攻击力基础 +lv*2-1，有1 / 4概率获得血量 +4 + lv*8"
		if (lv == 1) {
			comboLimit += 10;
			canStraight = true;
			for (int i = 0; i < 3; ++i) bossAtkBase[i] += 1;  
			randomHpAdd += 4;
		}
		else {
			comboLimit += 8;
			for (int i = 0; i < 3; ++i) bossAtkBase[i] += 2;
			randomHpAdd += 8;
		}
		break;
	case 2: // LEVEL % d : \n弃牌时每张弃牌削弱BOSS % d的攻击力（黑桃）\n弃牌后手牌数量少于 % d时直接失败"
		discardSpade += 3;
		if (lv == 1) deathHandCnt += 2;
		else deathHandCnt += 1;
		break;
	case 3: // LEVEL %d:\nQ手牌点数+%d\nQueen攻击力倍率+%d
		bossAtkMult[1] += 2.0f;
		numPower[12] += 8;
		break;
	case 4: //LEVEL %d:\nK手牌点数+%d\n所有King攻击力倍率+%d,生命值倍率+%d,生命值基础+%d
		numPower[13] += 12;
		bossAtkMult[2] += 2.0f;
		bossHpMult[2] += 1.0f;
		bossHpBase[2] += 1;
		break;
	case 5://LEVEL % d : \n如果出牌前手中有三种花色，触发第四种花色效果\n选择时随机两种花色血量倍率 + 0.15, 攻击力倍率 + 0.1
		int s1, s2;
		s1 = checkRand() % 4 + 1;
		s2 = checkRand() % 4 + 1;
		suitHpMult[s1] += 0.15f;
		suitHpMult[s2] += 0.15f;
		suitAtkMult[s1] += 0.1f;
		suitAtkMult[s2] += 0.1f;
		hasHierophant = true;
		break;
	case 6:
		loveGraph[nowLove.first][nowLove.second] = 1;
		break;
	case 7: //LEVEL % d : \nJ可以作为侍从牌打出，点数 + % d\nJack生命值倍率 + % d，攻击力倍率 + % d 3 1 1
		servantCards[11] = true;
		numPower[11] += 3;
		bossHpMult[0] += 1.0f;
		bossAtkMult[0] += 1.0f;
		break;
	case 8:// LEVEL % d : \n所有牌点数 + % d\nBOSS 血量修正 + % d，攻击力修正 + % d 1 4 2
		for (int i = 1; i <= 13; ++i) numPower[i] += 1;
		for (int i = 0; i < 3; ++i) bossHpShift[0] += 4;
		for (int i = 0; i < 3; ++i) bossAtkShift[0] += 2;
		break;
	case 9: // LEVEL %d:\n每次放置1张joker在牌堆顶\n选择本牌时25%概率所有BOSS血量基础+2
		cardPile.push_back(CardData(0, checkRand() % 2)); // JOKER
		if (checkRand() % 4 == 0) {
			badHermit = true;
			for (int i = 0; i < 3; ++i) {
				bossHpBase[i] += 2;
			}
		}
		break;
	case 10: // LEVEL % d : \n塔罗牌选择数 + % d，最终威力 + % d，每次选择塔罗牌后随机更改手牌花色
		if (lv == 1) tarotChoiceCnt += 2;
		else tarotChoiceCnt += 1;
		tarotChoiceCnt = std::min(22, tarotChoiceCnt);
		finalPower += 1;
		suitChangeAfterTarot = true;
		break;
	case 11: // LEVEL % d : \n捕获BOSS后额外生成%d张BOSS到弃牌堆，所有BOSS攻击力倍率+%f lv+1 lv+1
		if (lv == 1) captureTimes += 2;
		else captureTimes += 1;
		if (lv == 1) for (int i = 0; i < 3; ++i) bossAtkMult[i] += 1;
		else for (int i = 0; i < 3; ++i) bossAtkMult[i] += 0.5f;
		break;
	case 12: //LEVEL %d:\n所有BOSS攻击力基础减少%d，攻击力修正减少%d，手牌槽位减少%d
		for (int i = 0; i < 3; ++i) bossAtkBase[i] -= 1;
		for (int i = 0; i < 3; ++i) bossAtkMult[i] -= 1;
		for (int i = 0; i < 3; ++i) bossAtkShift[i] -= 6;
		if (lv == 1) {
			handMaxCnt -= 1;
		}
		else {
			handMaxCnt -= 2;
		}
		break;
	case 13: //LEVEL %d:\n有%d%的概率跳过BOSS攻击\n所有BOSS攻击力修正+%d  std::max(75,20 + lv * 5),lv * 3 
		if (lv == 1) deathFortune += 25;
		else deathFortune += 5;
		deathFortune = std::min(75, deathFortune);
		for (int i = 0; i < 3; ++i) bossAtkShift[i] += 3;
		break;
	case 14: //LEVEL %d:\n每回合开始时回洗%d张牌并抽取%d张牌，红心方片点数-%d lv lv lv*2
		suitPower[2] -= 3;
		suitPower[3] -= 3;
		temperanceCnt += 1;
		break;
	case 15: //LEVEL %d:\n手牌上限-%d，结算点数-%d，每回合出牌次数+%d 2*lv - 1,lv * 3 - 1,lv
		if (lv == 1) handMaxCnt -= 1;
		else handMaxCnt -= 3;
		if (lv == 1) finalPower -= 2;
		else finalPower -= 4;
		dealingTimes += 1;
		dealingTimesCnt = dealingTimes;
		break;
	case 16: //LEVEL %d:\n所有BOSS血量倍率减少%f，血量修正减少%d\n结算点数减少%d 1.5*lv,8*lv,lv
		for (int i = 0; i < 3; ++i) bossHpMult[i] -= 1.5f;
		for (int i = 0; i < 3; ++i) bossHpShift[i] -= 8;
		finalPower -= 1;
		break;
	case 17: 
		suitPower[3] += 4;
		suitHpMult[3] += 0.25f;
		suitAtkMult[3] += 0.15f;
		break;
	case 18:
		suitPower[4] += 4;
		suitHpMult[4] += 0.4f;
		break;
	case 19:
		suitPower[2] += 4;
		suitHpMult[2] += 0.15f;
		break;
	case 20:
		if (lv == 1) judgeLevel += 2;
		else judgeLevel += 1;
		break;
	case 21:
		suitPower[1] += 4;
		suitHpMult[1] += 0.2f;
		suitAtkMult[1] += 0.2f;
		break;

	}
}

std::string GameData::getTarotDescription(int x,std::map<std::string,std::string>& trans) {
	std::string res = trans["tarot" + std::to_string(x) + "Text"];
	int lv = tarotLevel[x]+1;
	res = formatString(res, { lv });
	switch (x) { // from 0 to 21
	case 0:
		res = formatString(res, { 1 + lv });
		break;
	case 1:
		res = formatString(res, { 12 + lv * 8,lv * 2 - 1 ,4 + lv * 8 });
		break;
	case 2:
		res = formatString(res, { lv * 3 ,1 + lv });
		break;
	case 3:
		res = formatString(res, { 8 * lv ,2 * lv });
		break;
	case 4:
		res = formatString(res, { 12 * lv,2 * lv, lv ,lv });
		break;
	case 5:
		break;
	case 6: {
		std::string xx = trans["suit" + std::to_string(nowLove.first)];
		std::string yy = trans["suit" + std::to_string(nowLove.second)];
		res = formatString(res, {}, {}, { xx,yy,xx,yy });
	}
		break;
	case 7:
		res = formatString(res, { 3 * lv,lv,lv });
		break;
	case 8:
		res = formatString(res, { lv,lv * 4,lv * 2 });
		break;
	case 9:
		break;
	case 10:
		res = formatString(res, { lv+1,lv });
		break;
	case 11:
		res = formatString(res, { lv + 1 }, { 0.5f * lv + 0.5f });
		break;
	case 12:
		res = formatString(res, { lv,lv, 6 * lv, lv * 2 - 1 });
		break;
	case 13:
		res = formatString(res, { std::min(75,20 + lv * 5),lv *3 });
		break;
	case 14:
		res = formatString(res, { lv,lv,lv * 3 });
		break;
	case 15:
		res = formatString(res, { 3 * lv - 2,lv * 4 - 2,lv });
		break;
	case 16:
		res = formatString(res, { 8 * lv,lv }, { 1.5f * lv });
		break;
	case 17:
		res = formatString(res, { 4 * lv }, { 0.25f * lv,0.15f * lv });
		break;
	case 18:
		res = formatString(res, { 4 * lv }, { 0.4f * lv });
		break;
	case 19:
		res = formatString(res, { 4 * lv }, { 0.15f * lv });
		break;
	case 20:
		res = formatString(res, { lv + 1 , 4 * lv });
		break;
	case 21:
		res = formatString(res, { 4 * lv }, { 0.2f * lv,0.2f * lv });
		break;
	}
	return res;
}

void GameData::getLove() {
	int x = 0, y = 0;
	std::vector<std::tuple<unsigned long long, int, int>> vec;
	for (int i = 1; i <= 4; ++i) {
		for (int j = 1; j <= 4; ++j) {
			if (i != j && loveGraph[i][j] == 0) vec.push_back({ checkRand(), i,j });
		}
	}
	if (!vec.empty()) {
		std::sort(vec.begin(), vec.end());
		x = std::get<1>(vec[0]);
		y = std::get<2>(vec[0]);
		nowLove = { x,y };
	}
}


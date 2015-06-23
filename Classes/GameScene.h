#ifndef __GAMELAYER__
#define __GAMELAYER__
#include "audio/include/SimpleAudioEngine.h"
#include "cocos2d.h"
using std::string;
USING_NS_CC;
using namespace CocosDenshion;
enum {
    kSpriteBomb,
    kSpriteShockwave,
    kSpriteMeteor,
    kSpriteHealth,
    kSpriteHalo,
    kSpriteSparkle,
    kSpriteRay
};
enum {
    kBackground,
    kMiddleground,
    kForeground
};

class GameScene : public Layer{
private:
	Vector<Sprite*> meteorPool;
	Vector<Sprite*> healthPool;
	Vector<Sprite*> fallObj;
	Vector<Sprite*> clouds;
	int meteorPoolIndex;
	int healthPoolIndex;
	SpriteBatchNode* batchNode;
	Sprite* bomb;
	Sprite* ufo;
	Sprite* shockWave;
	Sprite* introMessage;
	Sprite* gameOverMessage;
	Label* energyDisplay;
	Label* scoreDisplay;
	Action* growBomb;
	Action* rotateSprite;
	Action* shockwaveSequence;
	Action* swingHealth;
	Action* groundHit;
	Action* explosion;
	Size screenSize;
	float meteorInterval;
	float meteorTimer;
	float meteorSpeed;
	float healthInterval;
	float healthTimer;
	float healthSpeed;
	float diffInterval;
	float diffTimer;
	float energy;
	int score;
	int shockwaveHits;
	bool running;
	void resetMeteor();
	void resetHealth();
	void resetGame();
	void stopGame();
	void increaseDifficulty();
	void createGameScreen();
	void createPools();
	void createActions();
	void changeEnergy(float);
	template<class T> string to_string(T);
public:
	virtual ~GameScene(); //destructor
	GameScene(); //constructor
	virtual bool init(); //init
	static cocos2d::Scene* scene();
	void fallingObjDone(Node*);
	void animationDone(Node*);
	void shockwaveDone();
	virtual bool onTouchBegan(Touch*, Event*);
	virtual void update(float);
	CREATE_FUNC(GameScene);
};
#endif __GAMELAYER__
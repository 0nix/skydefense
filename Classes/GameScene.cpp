#include "GameScene.h"
using namespace cocos2d;
//public
template <typename T>
string GameScene::to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}
GameScene::GameScene(){}
GameScene::~GameScene(){ //destructor
	growBomb->release();
	rotateSprite->release();
	shockwaveSequence->release();
	swingHealth->release();
	groundHit->release();
	explosion->release();
	meteorPool.clear();
	healthPool.clear();
	fallObj.clear();
	clouds.clear();
}
bool GameScene::init(){
	if(!Layer::init()) return false;
	screenSize = Director::getInstance()->getWinSize();
	running = false;
	this->createGameScreen();
	this->createPools();
	this->createActions();
	auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    auto backListener = EventListenerKeyboard::create();
	backListener->onKeyReleased = [](EventKeyboard::KeyCode keyCode, Event* event){
    	Director::getInstance()->end();
	};
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(backListener, this);
	SimpleAudioEngine::getInstance()->playBackgroundMusic("background.mp3", true);
	this->scheduleUpdate();
    return true;
}
Scene* GameScene::scene(){
	auto scene = Scene::create();
	auto layer = GameScene::create();
	scene->addChild(layer);
	return scene;
}
void GameScene::fallingObjDone(Node* n){
	fallObj.erase(fallObj.find((Sprite*)n));
	n->stopAllActions();
	n->setRotation(0);
	if(n->getTag() == kSpriteMeteor){
		changeEnergy(-15);
		n->runAction(groundHit->clone());
		SimpleAudioEngine::getInstance()->playEffect("boom.wav");
	} else{
		n->setVisible(false);
		if(energy == 100){
			score += 25;
			scoreDisplay->setString(String::createWithFormat("%i", score)->getCString());
		} else {
			changeEnergy(10);
		}
		SimpleAudioEngine::getInstance()->playEffect("health.wav");
	}
}
void GameScene::animationDone(Node* n){
	n->setVisible(false);
}
void GameScene::shockwaveDone(){
	shockWave->setVisible(false);
}
bool GameScene::onTouchBegan(Touch* touch, Event* e){
	if(!running){
		if(introMessage->isVisible()) introMessage->setVisible(false);
		else if(gameOverMessage->isVisible()){ 
			gameOverMessage->setVisible(false);
			SimpleAudioEngine::getInstance()->stopAllEffects();
		}
		this->resetGame();
		return true;
	}
	if(touch){
		if(bomb->isVisible()){ // yes bomb
			bomb->stopAllActions();
			auto child = (Sprite*) bomb->getChildByTag(kSpriteHalo);
			child->stopAllActions();
			child = (Sprite*) bomb->getChildByTag(kSpriteSparkle);
			child->stopAllActions();
			if(bomb->getScale() > 0.7f){ //ready to blow
				shockWave->stopAllActions();
				shockWave->setScale(0.1f);
				shockWave->setPosition(bomb->getPosition());
				shockWave->setOpacity(100);
				shockWave->setVisible(true);
				shockWave->runAction(ScaleTo::create(0.5,bomb->getScale() * 2.0f));
				shockWave->runAction(shockwaveSequence->clone());
				SimpleAudioEngine::getInstance()->playEffect("bombRelease.wav");
			} 
			else SimpleAudioEngine::getInstance()->playEffect("bombFail.wav"); //bomb not at size
			bomb->setVisible(false);
			shockwaveHits = 0;
		}else{ //no bomb
			Point tap = touch->getLocation();
			bomb->stopAllActions();
			bomb->setScale(0.1f);
			bomb->setPosition(tap);
			bomb->setOpacity(50);
			bomb->setVisible(true);
			bomb->runAction(growBomb->clone());
			auto child = (Sprite*) bomb->getChildByTag(kSpriteHalo);
			child->runAction(rotateSprite->clone());
			child = (Sprite*) bomb->getChildByTag(kSpriteSparkle);
			child->runAction(rotateSprite->clone());
		}
		return true;
	}
}
void GameScene::update(float delta){
	if(!running) return;
	int count, i;
	meteorTimer += delta;
	healthTimer += delta;
	diffTimer += delta;
	 if (meteorTimer > meteorInterval) {
        meteorTimer = 0;
        this->resetMeteor();
    }
    if (healthTimer > healthInterval) {
        healthTimer = 0;
        this->resetHealth();
    }
    if (diffTimer > diffInterval) {
        diffTimer = 0;
        this->increaseDifficulty();
    }
	if(shockWave->isVisible()){
		count =(int)fallObj.size();
		for(i = count - 1; i >= 0; --i){
			auto sprite = fallObj.at(i);
			float diffx = shockWave->getPositionX() - sprite->getPositionX();
			float diffy = shockWave->getPositionY() - sprite->getPositionY();
			if(pow(diffx, 2) + pow(diffy, 2) <= pow(shockWave->getBoundingBox().size.width * 0.5f, 2)){
				sprite->stopAllActions();
				sprite->runAction(explosion->clone());
				SimpleAudioEngine::getInstance()->playEffect("boom.wav");
				if(sprite->getTag() == kSpriteMeteor){
					shockwaveHits++;
					score += shockwaveHits * 13 + shockwaveHits * 2;
				}
				fallObj.erase(i);
			}
		}
		scoreDisplay->setString(to_string(score));
	}
	for(auto sprite : clouds){
		sprite->setPositionX(sprite->getPositionX() + delta * 20);
		if(sprite->getPositionX() > screenSize.width + sprite->getBoundingBox().size.width * 0.5f)
				sprite->setPositionX(- sprite->getBoundingBox().size.width * 0.5f);
	}
	if(bomb->isVisible() && bomb->getScale() > 0.7f && bomb->getOpacity() != 255){
		bomb->setOpacity(255);
	}

}
//private
void GameScene::resetMeteor(){
	if(fallObj.size() > 30) return;
	auto meteor = meteorPool.at(meteorPoolIndex++);
	if(meteorPoolIndex == meteorPool.size()) meteorPoolIndex = 0;
	int meteorX = rand() % (int)(screenSize.width * 0.8f) + screenSize.width * 0.1f;
	int meteorTargetX = rand() % (int)(screenSize.width * 0.8f) + screenSize.width * 0.1f;
	meteor->stopAllActions();
	meteor->setPosition(Vec2(meteorX, screenSize.height + meteor->getBoundingBox().size.height * 0.5));
	auto rotate = RotateBy::create(0.5f, -90);
	auto repeatRotate = RepeatForever::create(rotate);
	auto seq = Sequence::create(
			MoveTo::create(meteorSpeed, Vec2(meteorTargetX, screenSize.height * 0.15f)),
			CallFunc::create(std::bind(&GameScene::fallingObjDone, this, meteor)),
			nullptr);
	meteor->setVisible(true);
	meteor->runAction(repeatRotate);
	meteor->runAction(seq);
	fallObj.pushBack(meteor);

}
void GameScene::resetHealth(){
	if (fallObj.size() > 30) return;
    auto health = healthPool.at(healthPoolIndex);
	healthPoolIndex++;
	if (healthPoolIndex == healthPool.size()) healthPoolIndex = 0;
	int health_x = rand() % (int)(screenSize.width * 0.8f) + screenSize.width * 0.1f;
    int health_target_x = rand() % (int)(screenSize.width * 0.8f) + screenSize.width * 0.1f;

    health->stopAllActions();
    health->setPosition(Vec2(health_x, screenSize.height + health->getBoundingBox().size.height * 0.5));
    //create action
    auto  sequence = Sequence::create(
           MoveTo::create(healthSpeed, 
           		Vec2(health_target_x, screenSize.height * 0.15f)),
				CallFunc::create(std::bind(&GameScene::fallingObjDone, this, health)),
				nullptr);
    health->setVisible(true);
    health->runAction(swingHealth->clone());
    health->runAction(sequence);
    fallObj.pushBack(health);
}
void GameScene::resetGame(){
	score = 0;
	energy = 100;
	meteorInterval = 2.5f;
	meteorSpeed = 10;
	meteorTimer = meteorInterval * 0.99f;
	healthInterval = 20;
	healthTimer = 0;
	healthSpeed = 15;
	diffInterval = 60;
	diffTimer = 0;
	running = true;
	energyDisplay->setString(to_string((int)energy) + "%");
	scoreDisplay->setString(to_string((int)score));
}
void GameScene::stopGame(){
	running = false;
	int i;
	int count = (int)fallObj.size();
	for( i = count - 1; i >= 0; --i){
		auto sprite = fallObj.at(i);
		sprite->stopAllActions();
		sprite->setVisible(false);
		fallObj.erase(i);
	}
	if(bomb->isVisible()){
		bomb->stopAllActions();
		bomb->setVisible(false);
		auto child = (Sprite*) bomb->getChildByTag(kSpriteHalo);
		child->stopAllActions();
		child = (Sprite*) bomb->getChildByTag(kSpriteSparkle);
		child->stopAllActions();
	}
	if(shockWave->isVisible()){
		shockWave->stopAllActions();
		shockWave->setVisible(false);
	}

}
void GameScene::increaseDifficulty(){
	meteorInterval -= 0.15f;
    if (meteorInterval < 0.25f) meteorInterval = 0.25f;
    meteorSpeed -= 1;
    if (meteorSpeed < 4) meteorSpeed = 4;
    healthSpeed -= 1;
    if (healthSpeed < 8) healthSpeed = 8;

}
void GameScene::createGameScreen(){
	auto bg = Sprite::create("bg.png");
	bg->setAnchorPoint(Vec2(0,0));
	bg->setPosition(Vec2(0,0));
	this->addChild(bg,kBackground);
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("sprite_sheet.plist");
	batchNode = SpriteBatchNode::create("sprite_sheet.png");
	this->addChild(batchNode);
	//BG SPRITES SETUP
	for(int i=0; i<2; ++i){
		auto sprite = Sprite::createWithSpriteFrameName("city_dark.png");
		sprite->setAnchorPoint(Vec2(0.5f,0.0f));
		sprite->setPosition(screenSize.width * (0.25f + i * 0.5f), 0);
		batchNode->addChild(sprite, kMiddleground);
		sprite = Sprite::createWithSpriteFrameName("city_light.png");
		sprite->setAnchorPoint(Vec2(0.5f,0.0f));
		sprite->setPosition(Vec2(screenSize.width * (0.25f + i * 0.5f), screenSize.height * 0.1f));
		batchNode->addChild(sprite, kBackground);
	}
	for(int j=0; j<3; ++j){
		auto sprite = Sprite::createWithSpriteFrameName("trees.png");
		sprite->setAnchorPoint(Vec2(0.5f,0.0f));
		sprite->setPosition(Vec2(screenSize.width * (0.2f + j * 0.3f),0));
		batchNode->addChild(sprite, kForeground);
	}
	for(int k=0; k<4; ++k){
		float cloudY = (k % 2 == 0) ? screenSize.height * 0.4f : screenSize.height * 0.5f;
		auto cloud = Sprite::createWithSpriteFrameName("cloud.png");
		cloud->setPosition(Vec2(screenSize.width * 0.1f + k * screenSize.width * 0.3f, cloudY));
		batchNode->addChild(cloud, kBackground);
		clouds.pushBack(cloud);
	}
	//BOMB SETUP
	bomb = Sprite::createWithSpriteFrameName("bomb.png");
	bomb->getTexture()->generateMipmap();
	bomb->setVisible(false);
	auto bombSize = bomb->getContentSize();
	auto sparkle = Sprite::createWithSpriteFrameName("sparkle.png");
	sparkle->setPosition(Vec2(bombSize.width * 0.72f, bombSize.height * 0.72f));
	bomb->addChild(sparkle, kMiddleground, kSpriteSparkle);
	auto halo = Sprite::createWithSpriteFrameName("halo.png");
	halo->setPosition(Vec2(bombSize.width * 0.4f, bombSize.height * 0.4f));
	bomb->addChild(halo,kMiddleground,kSpriteHalo);
	batchNode->addChild(bomb,kForeground);
	shockWave = Sprite::createWithSpriteFrameName("shockwave.png");
	shockWave->getTexture()->generateMipmap();
	shockWave->setVisible(false);
	batchNode->addChild(shockWave, kForeground);
	//LABEL SETUP
	scoreDisplay = Label::createWithBMFont("font.fnt","0");
	scoreDisplay->setAnchorPoint(Vec2(1,0.5));
	scoreDisplay->setPosition(Vec2(screenSize.width * 0.8f, screenSize.height * 0.94f));
	this->addChild(scoreDisplay);
	energyDisplay = Label::createWithBMFont("font.fnt","100%",TextHAlignment::RIGHT);
	energyDisplay->setPosition(Vec2(screenSize.width * 0.3f, screenSize.height * 0.94f));
	this->addChild(energyDisplay);
	auto icon = Sprite::createWithSpriteFrameName("health_icon.png");
	icon->setPosition(Vec2(screenSize.width * 0.15f, screenSize.height * 0.94f));
	batchNode->addChild(icon,kBackground);
	introMessage = Sprite::createWithSpriteFrameName("logo.png");
	introMessage->setPosition(Vec2(screenSize.width * 0.5f, screenSize.height * 0.6f));
	introMessage->setVisible(true);
	this->addChild(introMessage, kForeground);
	gameOverMessage = Sprite::createWithSpriteFrameName("gameover.png");
	gameOverMessage->setPosition(Vec2(screenSize.width * 0.5f, screenSize.height * 0.65f));
	gameOverMessage->setVisible(false);
	this->addChild(gameOverMessage,kForeground);
}
void GameScene::createPools(){
	int i;
	meteorPoolIndex = 0;
	healthPoolIndex = 0;
	for(i = 0; i < 50; ++i){
		auto sprite = Sprite::createWithSpriteFrameName("meteor.png");
		sprite->setVisible(false);
		batchNode->addChild(sprite, kMiddleground, kSpriteMeteor);
		meteorPool.pushBack(sprite);
	}
	for(i = 0; i < 20; ++i){
		auto sprite = Sprite::createWithSpriteFrameName("health.png");
		sprite->setVisible(false);
		sprite->setAnchorPoint(Vec2(0.5f, 0.8f));
		batchNode->addChild(sprite, kMiddleground, kSpriteHealth);
		healthPool.pushBack(sprite);
	}
}
void GameScene::createActions(){
	auto easeSwing = Sequence::create(EaseInOut::create(RotateTo::create(1.2f, -10),2),
									  EaseInOut::create(RotateTo::create(1.2f, 10),2),
									  nullptr);
	swingHealth = RepeatForever::create((ActionInterval*)easeSwing);
	swingHealth->retain();
	shockwaveSequence = Sequence::create(FadeOut::create(1.0f),
										 CallFunc::create(std::bind(&GameScene::shockwaveDone, this)),
										 nullptr);
	shockwaveSequence->retain();
	growBomb = ScaleTo::create(6.0f, 10);
	growBomb->retain();
	rotateSprite = RepeatForever::create(RotateBy::create(0.5f, -90));
	rotateSprite->retain();
	auto animation = Animation::create();
	int i;
	for(i = 1; i <= 10; i++) {
		auto name = String::createWithFormat("boom%i.png", i);
		auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(name->getCString());
		animation->addSpriteFrame(frame);
	}
	//DELAY PER UNIT IS DURATION IN SECS / FRAMES
	animation->setDelayPerUnit(1 / 10.0f);
	animation->setRestoreOriginalFrame(true);
	groundHit = Sequence::create(
			MoveBy::create(0, Vec2(0, screenSize.height * 0.12f)),
			Animate::create(animation),
			CallFuncN::create(CC_CALLBACK_1(GameScene::animationDone, this)),
			nullptr);
	groundHit->retain();
	animation = Animation::create();
	for(int i = 1; i <= 7; ++i){
		auto name = String::createWithFormat("explosion_small%i.png", i);
		auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(name->getCString());
		animation->addSpriteFrame(frame);
	}
	//DELAY PER UNIT IS DURATION IN SECS / FRAMES
	animation->setDelayPerUnit(0.5 / 7.0f);
	animation->setRestoreOriginalFrame(true);
	explosion = Sequence::create(
		Animate::create(animation),
		CallFuncN::create(CC_CALLBACK_1(GameScene::animationDone, this)),
		nullptr);
	explosion->retain();
}
void GameScene::changeEnergy(float value){
	energy += value;
    if (energy <= 0) {
        energy = 0;
        this->stopGame();
        SimpleAudioEngine::getInstance()->playEffect("fire_truck.wav");
        //show GameOver
        gameOverMessage->setVisible(true);
    }
    if (energy > 100) energy = 100;
    energyDisplay->setString(String::createWithFormat("%i%",(int)energy)->getCString());
}
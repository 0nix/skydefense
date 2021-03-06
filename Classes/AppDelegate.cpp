#include "AppDelegate.h"

#include "GameScene.h"
#include "audio/include/SimpleAudioEngine.h"
#include <vector>
using namespace CocosDenshion;
USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// If you want to use packages manager to install more packages, 
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLViewImpl::create("My Game");
        //glview -> setFrameSize(1024, 768);
        director->setOpenGLView(glview);
    }
    auto screenSize = glview -> getFrameSize();
    auto designSize = Size(2048, 1536);
    glview -> setDesignResolutionSize(designSize.width, designSize.height, ResolutionPolicy::EXACT_FIT);
    std::vector<std::string> searchPaths;
    if(screenSize.height > 768){
        searchPaths.push_back("ipadhd");
        director -> setContentScaleFactor(1536/designSize.height);
    }else if(screenSize.height > 320){
        searchPaths.push_back("ipad");
        director -> setContentScaleFactor(768/designSize.height);
    }else{
        searchPaths.push_back("iphone");
        director -> setContentScaleFactor(380/designSize.height);
    }
    auto utils = FileUtils::getInstance();
    utils -> setSearchPaths(searchPaths);
    auto audioEngine = SimpleAudioEngine::getInstance();
    audioEngine->preloadBackgroundMusic(utils->fullPathForFilename("background.mp3").c_str());
    audioEngine->preloadEffect(utils->fullPathForFilename("bombFail.wav").c_str() );
    audioEngine->preloadEffect(utils->fullPathForFilename("bombRelease.wav").c_str() );
    audioEngine->preloadEffect(utils->fullPathForFilename("boom.wav").c_str() );
    audioEngine->preloadEffect(utils->fullPathForFilename("health.wav").c_str() );
    audioEngine->preloadEffect(utils->fullPathForFilename("pew.wav").c_str() );
    audioEngine->preloadEffect(utils->fullPathForFilename("fire_truck.wav").c_str() );
    audioEngine->setBackgroundMusicVolume(0.5f);
    audioEngine->setEffectsVolume(0.5f);
    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    register_all_packages();

    // create a scene. it's an autorelease object
    auto scene = GameScene::scene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

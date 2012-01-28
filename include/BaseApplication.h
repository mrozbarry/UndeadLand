
#ifndef __OgreApplication_h_
#define __OgreApplication_h_

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include "Gorilla.h"
#include "OgreConsoleForGorilla.h"
#include "terrain.hpp"
#include "main.hpp"

class UndeadLand : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
    UndeadLand(void);
    virtual ~UndeadLand(void);

    virtual void go(void);
    
    bool LoadImage(const Ogre::String& texture_name, const Ogre::String& texture_path);
    
    void updateHud();

protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
    virtual void createScene(void);
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    // Ogre::WindowEventListener
    //Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw);
    //Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw);

    Ogre::Root *mRoot;
    Ogre::Camera *mCamera;
    Ogre::Viewport *mViewport;
    Ogre::SceneManager *mSceneMgr;
    Ogre::RenderWindow *mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
    
    TerrainEngine *terrain;
    bool mNoClip;

    // OgreBites
    OgreBites::SdkCameraMan *mCameraMan;       // basic camera controller
    bool mCursorWasVisible;                    // was cursor visible before dialog appeared
    bool mShutDown;

    //OIS Input devices
    OIS::InputManager *mInputManager;
    OIS::Mouse *mMouse;
    OIS::Keyboard *mKeyboard;
    
    // Gorilla
    Gorilla::Silverback*    mGorilla;
    Gorilla::Screen*        mScreen;
    Gorilla::Screen*        mHud;
    OgreConsole*            mConsole;
    int                     mFilterMode;
};

/* Console Callbacks */

void version(Ogre::StringVector&);

#endif // #ifndef __OgreApp_h_

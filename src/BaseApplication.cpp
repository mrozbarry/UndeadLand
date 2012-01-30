
#include "main.hpp"
#include "BaseApplication.h"

//-------------------------------------------------------------------------------------
UndeadLand::UndeadLand(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mCameraMan(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    mFilterMode(0),
    mNoClip(false)
{
  soundeng = irrklang::createIrrKlangDevice();
  if( !soundeng ) Ogre::LogManager::getSingletonPtr()->logMessage( "Could not initialize sound engine" );
}

//-------------------------------------------------------------------------------------
UndeadLand::~UndeadLand(void)
{
  if( mGorilla ) delete mGorilla;
  if( mCameraMan ) delete mCameraMan;

  Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
  windowClosed(mWindow);
  delete mRoot;
}

//-------------------------------------------------------------------------------------
bool UndeadLand::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Procedural World Test");

        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void UndeadLand::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void UndeadLand::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}
//-------------------------------------------------------------------------------------
void UndeadLand::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mRoot->addFrameListener(this);

    mGorilla = new Gorilla::Silverback();
    mGorilla->loadAtlas("dejavu");
    mScreen = mGorilla->createScreen( mViewport, "dejavu");
    //mHud = mGorilla->createScreen( mViewport, "dejavu" );
    
    mConsole = new OgreConsole();
    mConsole->init(mScreen);
    mConsole->addCommand("version", version);
    mConsole->setVisible( false );
}
//-------------------------------------------------------------------------------------
void UndeadLand::createScene(void)
{
  mCamera->setPosition(Ogre::Vector3(1683, 50, 2116));
  mCamera->lookAt(Ogre::Vector3(1963, 50, 1660));
  mCamera->setNearClipDistance(0.1);
  mCamera->setFarClipDistance(50000);

  if( mRoot->getRenderSystem()->getCapabilities()->hasCapability( Ogre::RSC_INFINITE_FAR_PLANE ) )
    mCamera->setFarClipDistance(0);
    
  Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
  lightdir.normalise();

  Ogre::Light* light = mSceneMgr->createLight("tstLight");
  light->setType(Ogre::Light::LT_DIRECTIONAL);
  light->setDirection(lightdir);
  light->setDiffuseColour(Ogre::ColourValue::White);
  light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));

  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
  
  if( soundeng ) {
    //soundeng->play2D( "dist/media/sounds/Pong_Beat_100_bpm_www.loopartists.com.wav", true );
    soundeng->play2D( "dist/media/sounds/looperman-loop-00500098-00050813-shortbusmusic-piano-in-bm7-a-e.wav", true );
  }

  terrain = new TerrainEngine( "this is a ridiculously long string", mRoot, mSceneMgr, mCamera, light );
}
//-------------------------------------------------------------------------------------
void UndeadLand::destroyScene(void)
{
  if( soundeng ) soundeng->drop();
  if( terrain ) delete terrain;
}
//-------------------------------------------------------------------------------------
void UndeadLand::createViewports(void)
{
    // Create one viewport, entire window
    mViewport = mWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void UndeadLand::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void UndeadLand::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void UndeadLand::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    
    Ogre::Image combined;
    
    // Terrain Dirt
    LoadImage( "terrain-dirt-ds", "dist/media/materials/textures/terrain_dirt_diffusespecular.png" );
    LoadImage( "terrain-dirt-nh", "dist/media/materials/textures/terrain_dirt_normalheight.png" );
    LoadImage( "terrain-grass-ds", "dist/media/materials/textures/terrain_grass_diffusespecular.png" );
    LoadImage( "terrain-grass-nh", "dist/media/materials/textures/terrain_grass_normalheight.png" );
    LoadImage( "terrain-rock-ds", "dist/media/materials/textures/terrain_rock_diffusespecular.png" );
    LoadImage( "terrain-rock-nh", "dist/media/materials/textures/terrain_rock_normalheight.png" );
    /*if( LoadImage( "terrain-dirt-ds", "terrain_dirt_diffusespecular.png" ) == false ) {
      combined.loadTwoImagesAsRGBA("terrain_dirt.jpg", "terrain_dirt_SPEC.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
      combined.save("terrain_dirt_diffusespecular.png");
    }
    if( LoadImage( "terrain-dirt-nh", "terrain_dirt_normalheight.png" ) == false ) {
      combined.loadTwoImagesAsRGBA("terrain_dirt_NORM.tga", "terrain_dirt_DISP.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
      combined.save("terrain_dirt_normalheight.png");
    }
    
    // Terrain Grass
    if( LoadImage( "terrain-grass-ds", "terrain_grass_diffusespecular.png" ) == false ) {
      combined.loadTwoImagesAsRGBA("terrain_grass.jpg", "terrain_grass_SPEC.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
      combined.save("terrain_grass_diffusespecular.png");
    }
    if( LoadImage( "terrain-grass-nh", "terrain_grass_normalheight.png" ) == false ) {
      combined.loadTwoImagesAsRGBA("terrain_grass_NORM.tga", "terrain_grass_DISP.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
      combined.save("terrain_grass_normalheight.png");
    }
    
    // Terrain Rock
    if( LoadImage( "terrain-rock-ds", "terrain_rock_diffusespecular.png" ) == false ) {
      combined.loadTwoImagesAsRGBA("terrain_rock.jpg", "terrain_rock_SPEC.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
      combined.save("terrain_rock_diffusespecular.png");
    }
    if( LoadImage( "terrain-rock-nh", "terrain_rock_normalheight.png" ) == false ) {
      combined.loadTwoImagesAsRGBA("terrain_rock_NORM.tga", "terrain_rock_DISP.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
      combined.save("terrain_rock_normalheight.png");
    }*/
}
//-------------------------------------------------------------------------------------
void UndeadLand::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool UndeadLand::setup(void)
{
    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    
    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
};
//-------------------------------------------------------------------------------------
bool UndeadLand::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    mKeyboard->capture();
    mMouse->capture();

    mCameraMan->frameRenderingQueued(evt);
    
    terrain->onFrameRenderingQueued( );
    
    if( !mNoClip ) terrain->fixCameraTerrain( mCamera, 40.0f );
    
    return true;
}
//-------------------------------------------------------------------------------------
bool UndeadLand::keyPressed( const OIS::KeyEvent &arg )
{
    if( mConsole->isVisible() ) {
      mConsole->onKeyPressed( arg );
      return true;
    }
    
    if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        if( ++mFilterMode >= 4 ) mFilterMode = 0;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mFilterMode)
        {
        case 1:
            //newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            OgreConsole::getSingleton().print("Filter Mode:Trilinear");
            break;
        case 2:
            //newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            OgreConsole::getSingleton().print("Filter Mode:Anisotropic");
            break;
        case 3:
            //newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            OgreConsole::getSingleton().print("Filter Mode:None");
            break;
        default:
            //newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
            OgreConsole::getSingleton().print("Filter Mode:Bilinear");
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        //Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            //newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            OgreConsole::getSingleton().print("Polygon Mode:Wireframe");
            break;
        case Ogre::PM_WIREFRAME:
            //newVal = "Points";
            pm = Ogre::PM_POINTS;
            OgreConsole::getSingleton().print("Polygon Mode:Points");
            break;
        default:
            //newVal = "Solid";
            pm = Ogre::PM_SOLID;
            OgreConsole::getSingleton().print("Polygon Mode:Solid");
        }

        mCamera->setPolygonMode(pm);
        //mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_V)   // refresh all textures
    {
        mNoClip = !mNoClip;
        std::stringstream msg;
        msg << "No-Clip: " << ( mNoClip ? "On" : "Off" );
        OgreConsole::getSingleton().print( msg.str() );
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    mCameraMan->injectKeyDown(arg);
    return true;
}
//-------------------------------------------------------------------------------------
bool UndeadLand::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    if( arg.key == OIS::KC_F1 )
    {
      mConsole->setVisible(!mConsole->isVisible());
      return true;
    }
    return true;
}
//-------------------------------------------------------------------------------------
bool UndeadLand::mouseMoved( const OIS::MouseEvent &arg )
{
    if( !mConsole->isVisible() ) mCameraMan->injectMouseMove(arg);
    //target = mCameraMan->getCamera()->getCameraToViewportRay( arg.state.X.abs, arg.state.Y.abs );
    return true;
}
//-------------------------------------------------------------------------------------
bool UndeadLand::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    /*selected = mTerrainGroup->rayIntersects( target, 0 );
    std::stringstream ts;
    ts<< "Terrain Selection: (" << selected.position.x << ", " << selected.position.y << ", " << selected.position.z << ") "
      << " Hit a terrain? " << (selected.hit == true ? "yes" : "no" );
    OgreConsole::getSingleton().print( ts.str() );*/
    return true;
}
//-------------------------------------------------------------------------------------
bool UndeadLand::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //if (mTrayMgr->injectMouseUp(arg, id)) return true;
    if( !mConsole->isVisible() ) mCameraMan->injectMouseUp(arg, id); //else mGui->injectMouse( arg.state.X.abs, arg.state.Y.abs, arg.state.buttonDown( OIS::MB_Left ) );
    mCameraMan->injectMouseUp(arg, id);
    return true;
}
//-------------------------------------------------------------------------------------
//Adjust mouse clipping area
void UndeadLand::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
//-------------------------------------------------------------------------------------
//Unattach OIS before window shutdown (very important under Linux)
void UndeadLand::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}
//-------------------------------------------------------------------------------------
bool UndeadLand::LoadImage(const Ogre::String& texture_name, const Ogre::String& texture_path)
{
  bool image_loaded = false;
  std::ifstream ifs( texture_path.c_str(), std::ios::binary|std::ios::in );
  if ( ifs.is_open( ) )
  {
    Ogre::String tex_ext;
    Ogre::String::size_type index_of_extension = texture_path.find_last_of('.');
    if ( index_of_extension != Ogre::String::npos )
    {
      tex_ext = texture_path.substr( index_of_extension + 1 );
      Ogre::DataStreamPtr data_stream( new Ogre::FileStreamDataStream( texture_path, &ifs, false ) );
      Ogre::Image img;
      img.load( data_stream, tex_ext );
      Ogre::TextureManager::getSingleton().loadImage( texture_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, img, Ogre::TEX_TYPE_2D, 0, 1.0f );
      image_loaded = true;
    }
    ifs.close();
  }
  return image_loaded;
}

//================================================================================================================
//  Console Functions
//================================================================================================================
void version(Ogre::StringVector&)
{
  std::stringstream udlv;
  udlv << UDL_WORKING_TITLE << " v" << UDL_VER_MAJ << "." << UDL_VER_MIN << "." << UDL_VER_REV << " " << UDL_VER_STAGE;
  OgreConsole::getSingleton().print( udlv.str() );
  std::stringstream s;
  s << "Ogre v" << OGRE_VERSION_MAJOR << "." << OGRE_VERSION_MINOR << "." << OGRE_VERSION_PATCH << " '" << OGRE_VERSION_NAME << "'";
  OgreConsole::getSingleton().print(s.str());
}


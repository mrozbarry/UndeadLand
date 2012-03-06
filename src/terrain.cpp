
#include "terrain.hpp"

#ifdef USE_PAGEMANAGER
float smoothNoise( Perlin *mPerlin, float x, float y, float scale )
{
  float corners = ( mPerlin->Get(x-scale, y-scale) + mPerlin->Get(x+scale, y-scale) + mPerlin->Get(x-scale, y+scale) + mPerlin->Get(x+scale, y+scale) ) / 16;
  float sides   = ( mPerlin->Get(x-scale, y) + mPerlin->Get(x+scale, y) + mPerlin->Get(x, y-scale) + mPerlin->Get(x, y+scale) ) /  8;
  float center  =  mPerlin->Get(x, y) / 4;
  
  return corners + sides + center;
}
#endif

TerrainEngine::TerrainEngine( Ogre::String seed, Ogre::Root *root, Ogre::SceneManager *scenemgr, Ogre::Camera *cam, Ogre::Light* light, Ogre::String file_prefix, Ogre::String file_suffix )
  : mRoot(root)
  , mTerrainGroup(0)
  , mTerrainPos(1000,0,5000)
  , mLayerEdit(1)
  , mTerrainsImported(false)
  , mSceneMgr(scenemgr)
  , mCamera(cam)
  , mSlotX(0)
  , mSlotY(0)
  , mUpdateTerrains(true)
  , mLockTerrains(false)
#if USE_PAGING
  , mTerrainPageProvider( hashString(seed) )
#else
  , mPerlin(16, 0.0001, 900, seed)
#endif
{
  terrainSelectClear();
  
  mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

  mTerrainGroup = OGRE_NEW Ogre::TerrainGroup( mSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE );
  
  std::stringstream fpx;
  
  mTerrainGroup->setFilenameConvention( file_prefix, file_suffix );
  mTerrainGroup->setOrigin( mTerrainPos );

  configureTerrainDefaults( light );
  
  //mTerrainGroup->convertWorldPositionToTerrainSlot( cam->getPosition(), &mSlotX, &mSlotY );
  mSlotX = mSlotY = 0;
  cam->setPosition( mTerrainGroup->getTerrainSlotPosition( 0, 0 ) );
  
#if USE_PAGING
  mPageManager = OGRE_NEW Ogre::PageManager();
  mPageManager->setPageProvider(&mTerrainPageProvider);
  mPageManager->addCamera(mCamera);
  mTerrainPaging = OGRE_NEW Ogre::TerrainPaging(mPageManager);
  Ogre::PagedWorld* world = mPageManager->createWorld();
  mTerrainPaging->createWorldSection(world, mTerrainGroup, 1024, 2048, 
    TERRAIN_PAGE_MIN_X, TERRAIN_PAGE_MIN_Y, 
    TERRAIN_PAGE_MAX_X, TERRAIN_PAGE_MAX_Y);
#endif
    
  mTerrainGroup->freeTemporaryResources();
}

TerrainEngine::~TerrainEngine( )
{
  mTerrainGroup->saveAllTerrains( true, true );
  terrainSelectClear();
  OGRE_DELETE mTerrainGroup;
  OGRE_DELETE mTerrainGlobals;
#if USE_PAGING
  OGRE_DELETE mTerrainPaging;
  OGRE_DELETE mPageManager;
#endif
}

void TerrainEngine::terrainSelect( Ogre::Terrain *terrain, Ogre::Vector3 position, Ogre::Real radius )
{
  mSelected.terrain = terrain;
  mSelected.position = position;
  mSelected.radius = radius;
}

TerrainEngine::TerrainSelect *TerrainEngine::terrainSelect( void ) {
  return &mSelected;
}

bool TerrainEngine::terrainSelected( void )
{
  return ( mSelected.terrain == NULL ? false : true );
}

void TerrainEngine::terrainSelectClear( void )
{
  mSelected.terrain = NULL;
  mSelected.position = Ogre::Vector3(0,0,0);
  mSelected.radius = 0.01;
}

void TerrainEngine::changeTerrainHeight( Ogre::Real scale )
{
  if( mSelected.terrain == NULL ) {
    // Log something in the console?
    return; // Get the hell out of dodge if we don't have a terrain selected
  }
  
  Ogre::Real terrainSize = ( mSelected.terrain->getSize() - 1 );
  long startx = ( mSelected.position.x - mSelected.radius ) * terrainSize;
  long starty = ( mSelected.position.y - mSelected.radius ) * terrainSize;
  long endx = ( mSelected.position.x + mSelected.radius ) * terrainSize;
  long endy= ( mSelected.position.y + mSelected.radius ) * terrainSize;
  startx = std::max(startx, 0L);
  starty = std::max(starty, 0L);
  endx = std::min(endx, (long)terrainSize);
  endy = std::min(endy, (long)terrainSize);
  for (long y = starty; y <= endy; ++y)
  {
    for (long x = startx; x <= endx; ++x)
    {
      Ogre::Real tsXdist = (x / terrainSize) - mSelected.position.x;
      Ogre::Real tsYdist = (y / terrainSize)  - mSelected.position.y;

      Ogre::Real weight = std::min((Ogre::Real)1.0, std::sqrt(tsYdist * tsYdist + tsXdist * tsXdist) / Ogre::Real(0.5 * mSelected.radius));
      weight = 1.0 - (weight * weight);

      float addedHeight = weight * 250.0 * scale; // * timeElapsed;
      float newheight = mSelected.terrain->getHeightAtPoint(x, y) + addedHeight;
      mSelected.terrain->setHeightAtPoint(x, y, newheight);

    }
  }
}

void TerrainEngine::changeTerrainTexture( Ogre::Real rate )
{
  if( mSelected.terrain == NULL ) {
    // Log something in the console?
    return; // Get the hell out of dodge if we don't have a terrain selected
  }
  
  Ogre::TerrainLayerBlendMap *layer = mSelected.terrain->getLayerBlendMap( mLayerEdit );
  Ogre::Real imgSize = mSelected.terrain->getLayerBlendMapSize();
  long startx = ( mSelected.position.x - mSelected.radius ) * imgSize;
  long starty = ( mSelected.position.y - mSelected.radius ) * imgSize;
  long endx = ( mSelected.position.x + mSelected.radius ) * imgSize;
  long endy= ( mSelected.position.y + mSelected.radius ) * imgSize;
  startx = std::max(startx, 0L);
  starty = std::max(starty, 0L);
  endx = std::min(endx, (long)imgSize);
  endy = std::min(endy, (long)imgSize);
  for (long y = starty; y <= endy; ++y)
  {
    for (long x = startx; x <= endx; ++x)
    {
      Ogre::Real tsXdist = (x / imgSize) -  mSelected.position.x;
      Ogre::Real tsYdist = (y / imgSize)  -  mSelected.position.y;

      Ogre::Real weight = std::min( (Ogre::Real)1.0, std::sqrt( tsYdist * tsYdist + tsXdist * tsXdist ) / Ogre::Real( 0.5 * mSelected.radius ) );
      weight = 1.0 - (weight * weight);

      float paint = weight * rate; // * timeElapsed;
      size_t imgY = imgSize - y;
      float val = layer->getBlendValue( x, imgY ) + paint;
      //val = Math::Clamp(val, 0.0f, 1.0f);
      val = std::max( std::min( 1.0f, val ), 0.0f );
      layer->setBlendValue( x, imgY, val );
    }
  }
  layer->update();
}

void TerrainEngine::changeTextureLayer( Ogre::uint8 layer )
{
  if( mSelected.terrain == NULL ) {
    // Log something in the console?
    return; // Get the hell out of dodge if we don't have a terrain selected
  }
  
  mLayerEdit = ( ( layer < 1 ) ? 1 : layer );
}

void TerrainEngine::onFrameRenderingQueued( void )
{
  long int csx, csy;
  mTerrainGroup->convertWorldPositionToTerrainSlot( mCamera->getPosition(), &csx, &csy );
  bool slotChange = ( ( csx != mSlotX ) || ( csy != mSlotY ) )
  bool loads = (qload.size() == 0);
  bool unloads = (qunload.size() == 0);
  if( !slotChange || loads || unloads || mLockTerrains ) return;
  mSlotX = csx;
  mSlotY = csy;
  
#ifndef USE_PAGING
  if ( !mTerrainGroup->isDerivedDataUpdateInProgress() ) {
    // Load/unload the next piece
    for( int i = 0; i < 3; i++ ) {
      TerrainQueue *tq = terrainQueueNext();
      if( tq ) {
        loadTerrain( tq->x, tq->y, !tq->load );
        terrainQueuePop();
        std::stringstream liq;
        liq << "Terrain chunks left in queue: " << terrainQueue.size();
        Ogre::LogManager::getSingletonPtr()->logMessage(liq.str());
      } else { break; }
    }
  }
  

  if( slotChange || mUpdateTerrains ) {
    
    std::stringstream tus;
    tus << "Updating terrain load/unload list; Camera Position=" << mCamera->getPosition() << "; mSlotX,Y=(" << mSlotX << ", " << mSlotY << "); CameraSlotX,Y=("<< csx <<","<< csy <<")";
    Ogre::LogManager::getSingletonPtr()->logMessage(tus.str());
    mSlotX = csx;
    mSlotY = csy;
    mUpdateTerrains = false;
    
    // Load Center first
    /*TerrainQueue tq;
    tq.terrain = mTerrainGroup->getTerrain( mSlotX, mSlotY );
    tq.x = mSlotX;
    tq.y = mSlotY;
    tq.load = true;
    terrainQueuePush( tq, true ); // Set as next priority!
    */
    
    loadTerrain( mSlotX, mSlotY );

    // Loop through distances
    for( long int dist = 0; dist < TERRAIN_DIST; dist++ ) {
      terrainQueueAtDistance( dist, true );
    }
    //terrainQueueAtDistance( TERRAIN_DIST, false );
    
  }
#endif
}

bool TerrainEngine::getTerrainLocked( void )
{
  return mLockTerrains;
}

void TerrainEngine::setTerrainLocked( bool lock )
{
  mLockTerrains = lock;
}

void TerrainEngine::fixCameraTerrain( Ogre::Camera *cam, float height )
{
  Ogre::Terrain *activeTerrain = mTerrainGroup->getTerrain( mSlotX, mSlotY );
  if( !activeTerrain ) return;
  Ogre::Vector3 tpos, cpos;
  cpos = mCamera->getPosition();
  //activeTerrain->getTerrainPosition( cpos, &tpos );
  cpos.y = height + activeTerrain->getHeightAtWorldPosition( cpos );
  mCamera->setPosition( cpos );
}

bool TerrainEngine::defineTerrain( long x, long y )
{
  Ogre::String filename = mTerrainGroup->generateFilename(x, y);
  if( Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename) ) {
    mTerrainGroup->defineTerrain(x, y);
    return false;
  }
  mTerrainGroup->defineTerrain( x, y, 0.0f );
  return true;
}

void TerrainEngine::initBlendMaps( Ogre::Terrain *terrain )
{
  Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap( 1 );
  Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap( 2 );
  Ogre::Real minHeight0 = 70;
  Ogre::Real fadeDist0 = 40;
  Ogre::Real minHeight1 = 70;
  Ogre::Real fadeDist1 = 15;
  float* pBlend1 = blendMap0->getBlendPointer();
  for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
  {
    for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
    {
      Ogre::Real tx, ty;

      blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
      Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
      Ogre::Real val = (height - minHeight0) / fadeDist0;
      val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);

      val = (height - minHeight1) / fadeDist1;
      val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
      *pBlend1++ = val;
    }
  }
  blendMap0->dirty();
  blendMap1->dirty();
  blendMap0->update();
  blendMap1->update();
}

void TerrainEngine::configureTerrainDefaults( Ogre::Light *light )
{
  mTerrainGlobals->setMaxPixelError(8);
  mTerrainGlobals->setCompositeMapDistance(3000);

  mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
  mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
  mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

  Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
  defaultimp.terrainSize = TERRAIN_SIZE;
  defaultimp.worldSize = TERRAIN_WORLD_SIZE;
  defaultimp.inputScale = 600;
  defaultimp.minBatchSize = 65;
  defaultimp.maxBatchSize = 129;
  
  defaultimp.layerList.resize(3);
  defaultimp.layerList[0].worldSize = 200;
  defaultimp.layerList[0].textureNames.push_back("terrain-grass-ds");
  defaultimp.layerList[0].textureNames.push_back("terrain-grass-nh");
  defaultimp.layerList[1].worldSize = 300;
  defaultimp.layerList[1].textureNames.push_back("terrain-dirt-ds");
  defaultimp.layerList[1].textureNames.push_back("terrain-dirt-nh");
  defaultimp.layerList[2].worldSize = 50;
  defaultimp.layerList[2].textureNames.push_back("terrain-rock-ds");
  defaultimp.layerList[2].textureNames.push_back("terrain-rock-nh");
}

void TerrainEngine::loadTerrain( long int x, long int y, bool unload )
{
  std::stringstream _log;
  Ogre::Terrain *terrain = mTerrainGroup->getTerrain( x, y );
  if( !unload ) {
    _log << "Loading terrain (" << x << ", " << y << ")";
    Ogre::LogManager::getSingletonPtr()->logMessage( _log.str() );
    if( !terrain ) {
      Ogre::LogManager::getSingletonPtr()->logMessage(" - Needs definition");
      if( defineTerrain( x, y ) ) {
        mTerrainGroup->loadTerrain( x, y, true );
        terrain = mTerrainGroup->getTerrain( x, y );
        int sz = terrain->getSize();
        for( long tx = 0; tx < sz; tx++ ) {
          for( long tz = 0; tz < sz; tz++ ) {
            Ogre::Vector3 worldpos;
            terrain->getPoint( tx, tz, &worldpos );
            terrain->setHeightAtPoint( tx, tz, smoothNoise( worldpos.x, worldpos.z, 0.01f ) );
          }
        }
        terrain->update();
        terrain->dirty();
        mTerrainGroup->update();
      } else {
        mTerrainGroup->loadTerrain( x, y, true ); // May need to be true?
      }
    }
  } else {
    if( terrain ) {
      _log << "Unloading terrain (" << x << ", " << y << ")";
      Ogre::LogManager::getSingletonPtr()->logMessage( _log.str() );
      if( terrain->isModified() ) {
        Ogre::LogManager::getSingletonPtr()->logMessage(" - Saving modifications");
        terrain->save( mTerrainGroup->generateFilename( x, y ) );
      }
      Ogre::LogManager::getSingletonPtr()->logMessage(" - Unloading from TerrainGroup");
      mTerrainGroup->unloadTerrain( x, y );
    }
  }
  Ogre::LogManager::getSingletonPtr()->logMessage(" - Done");
}

void TerrainEngine::loadTerrainRadius( int radius )
{
  if( radius == 0 ) {
    loadTerrainQueue( mSlotX, mSlotY );
  } else {
    long int lx, ly;
    long int minx, miny, maxx, maxy;
    minx = mSlotX-radius;
    maxx = mSlotX+radius;
    miny = mSlotY-radius;
    maxy = mSlotY+radius;
    for( lx = minx; lx < maxx; lx++ ) loadTerrainQueue( lx, miny );
    for( ly = miny; ly < maxy; ly++ ) loadTerrainQueue( maxx, ly );
    for( lx = maxx; lx > minx; lx-- ) loadTerrainQueue( lx, maxy );
    for( ly = maxy; ly > miny; ly-- ) loadTerrainQueue( minx, ly );
  }
}

void TerrainEngine::unloadTerrainRadius( int radius )
{
  if( radius == 0 ) {
    unloadTerrainQueue( mSlotX, mSlotY );
  } else {
    long int lx, ly;
    long int minx, miny, maxx, maxy;
    minx = mSlotX-radius;
    maxx = mSlotX+radius;
    miny = mSlotY-radius;
    maxy = mSlotY+radius;
    for( lx = minx; lx < maxx; lx++ ) unloadTerrainQueue( lx, miny );
    for( ly = miny; ly < maxy; ly++ ) unloadTerrainQueue( maxx, ly );
    for( lx = maxx; lx > minx; lx-- ) unloadTerrainQueue( lx, maxy );
    for( ly = maxy; ly > miny; ly-- ) unloadTerrainQueue( minx, ly );
  }
}

bool TerrainEngine::nextLoad( long int& x, long int& y )
{
  if( qload.size() == 0 ) return false;
  x = qload[0].x;
  y = qload[0].y;
  qload.pop_front();
  return true;
}

bool TerrainEngine::nextUnLoad( long int& x, long int& y )
{
  if( qunload.size() == 0 ) return false;
  x = qunload[0].x;
  y = qunload[0].y;
  qunload.pop_front();
  return true;
}

bool TerrainEngine::loadTerrainQueue( long int x, long int y )
{
  if( loadIsQueued(x,y) || unloadIsQueued(x,y) || slotLoaded(x,y) ) return false;
  TerrainQueueItem item;
  item.x = x; item.y = y;
  qload.push_back( item );
  return true;
}

bool TerrainEngine::unloadTerrainQueue( long int x, long int y )
{
  if( loadIsQueued(x,y) || unloadIsQueued(x,y) || slotLoaded(x,y) ) return false;
  TerrainQueueItem item;
  item.x = x; item.y = y;
  qunload.push_back( item );
  return true;
}

bool TerrainEngine::loadIsQueued( long int x, long int y )
{
  for( unsigned int c = 0; c < qload.size(); c++ ) if( qload[c].x == x && qload[c].y == y ) { return true; }
  return false;
}

bool TerrainEngine::unloadIsQueued( long int x, long int y )
{
  for( unsigned int c = 0; c < qunload.size(); c++ ) if( qunload[c].x == x && qunload[c].y == y ) { return true; }
  return false;
}

bool TerrainEngine::slotLoaded( long int x, long int y )
{
  return mTerrainGroup->getTerrain( x, y ) == NULL ? false : true;
}

float TerrainEngine::smoothNoise( float x, float y, float scale )
{
  float corners = ( mPerlin.Get(x-scale, y-scale) + mPerlin.Get(x+scale, y-scale) + mPerlin.Get(x-scale, y+scale) + mPerlin.Get(x+scale, y+scale) ) / 16;
  float sides   = ( mPerlin.Get(x-scale, y) + mPerlin.Get(x+scale, y) + mPerlin.Get(x, y-scale) + mPerlin.Get(x, y+scale) ) /  8;
  float center  =  mPerlin.Get(x, y) / 4;
  
  return corners + sides + center;
}

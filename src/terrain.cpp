
#include "terrain.hpp"

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
  , mPerlin(16, 0.0001, 900, hashString( seed ))
{
  terrainSelectClear();
  
  mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

  mTerrainGroup = OGRE_NEW Ogre::TerrainGroup( mSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE );
  
  std::stringstream fpx;
  
  mTerrainGroup->setFilenameConvention( file_prefix, file_suffix );
  mTerrainGroup->setOrigin( mTerrainPos );

  configureTerrainDefaults( light );
  
  mTerrainGroup->convertWorldPositionToTerrainSlot( cam->getPosition(), &mSlotX, &mSlotY );

  // Make sure 0,0 (or wherever the camera is placed) loads first
  TerrainQueue tq;
  tq.terrain = NULL;
  tq.x = 0;
  tq.y = 0;
  tq.load = true;
  terrainQueuePush( tq );
}

TerrainEngine::~TerrainEngine( )
{
  mTerrainGroup->saveAllTerrains( true );
  terrainSelectClear();
  OGRE_DELETE mTerrainGroup;
  OGRE_DELETE mTerrainGlobals;
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
  if ( !mTerrainGroup->isDerivedDataUpdateInProgress() ) {
    TerrainQueue *tq = terrainQueueNext();
    if( tq != NULL ) {
      std::stringstream _log;
      
      if( tq->load == true ) {
      
        _log << "Loading terrain (" << tq->x << ", " << tq->y << ")";
        Ogre::LogManager::getSingletonPtr()->logMessage( _log.str() );
        if( !tq->terrain ) {
          Ogre::LogManager::getSingletonPtr()->logMessage(" - Needs definition");
          if( defineTerrain( tq->x, tq->y ) ) {
            mTerrainGroup->loadTerrain( tq->x, tq->y, true );
            tq->terrain = mTerrainGroup->getTerrain( tq->x, tq->y );
            int sz = tq->terrain->getSize();
            for( long tx = 0; tx < sz; tx++ ) {
              for( long tz = 0; tz < sz; tz++ ) {
                Ogre::Vector3 worldpos;
                tq->terrain->getPoint( tx, tz, &worldpos );
                tq->terrain->setHeightAtPoint( tx, tz, smoothNoise( worldpos.x, worldpos.z, 0.01f ) );
              }
            }
            tq->terrain->update();
            tq->terrain->dirty();
            mTerrainGroup->update();
          } else {
            mTerrainGroup->loadTerrain( tq->x, tq->y, true );
          }
        }
        
      } else if( tq->load == false ) {
      
        if( tq->terrain ) {
          _log << "Unloading terrain (" << tq->x << ", " << tq->y << ")";
          Ogre::LogManager::getSingletonPtr()->logMessage( _log.str() );
          if( tq->terrain->isModified() ) {
            Ogre::LogManager::getSingletonPtr()->logMessage(" - Saving modifications");
            tq->terrain->save( mTerrainGroup->generateFilename( tq->x, tq->y ) );
          }
          Ogre::LogManager::getSingletonPtr()->logMessage(" - Unloading from TerrainGroup");
          mTerrainGroup->unloadTerrain( tq->x, tq->y );
        }
        
      }
      //terrainQueue.pop_back();
      terrainQueuePop();
      Ogre::LogManager::getSingletonPtr()->logMessage(" - Done");
    }
  }

  long int csx, csy;
  mTerrainGroup->convertWorldPositionToTerrainSlot( mCamera->getPosition(), &csx, &csy );
  if( ( ( ( csx != mSlotX ) || ( csy != mSlotY ) ) || mUpdateTerrains ) && !mLockTerrains ) {
    
    std::stringstream tus;
    tus << "Updating terrain load/unload list; Camera Position=" << mCamera->getPosition() << "; mSlotX,Y=(" << mSlotX << ", " << mSlotY << "); CameraSlotX,Y=("<< csx <<","<< csy <<")";
    Ogre::LogManager::getSingletonPtr()->logMessage(tus.str());
    mSlotX = csx;
    mSlotY = csy;
    mUpdateTerrains = false;
    
    // Load Center first
    TerrainQueue tq;
    tq.terrain = mTerrainGroup->getTerrain( mSlotX, mSlotY );
    tq.x = mSlotX;
    tq.y = mSlotY;
    tq.load = true;
    terrainQueuePush( tq, true ); // Set as next priority!

    // Loop through distances
    for( long int dist = 1; dist < TERRAIN_DIST; dist++ ) {
      terrainQueueAtDistance( dist, true );
    }
    terrainQueueAtDistance( TERRAIN_DIST, false );
    
  }
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
  activeTerrain->getTerrainPosition( cpos, &tpos );
  cpos.y = height + activeTerrain->getHeightAtWorldPosition( cpos );
  mCamera->setPosition( cpos );
}

bool TerrainEngine::defineTerrain( long x, long y )
{
  bool needsWork = false;
  Ogre::String filename = mTerrainGroup->generateFilename(x, y);
  if( Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename) ) {
    mTerrainGroup->defineTerrain(x, y);
  } else {
    mTerrainGroup->defineTerrain( x, y, 0.0f );
    needsWork = true;
  }
  return needsWork;
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

float TerrainEngine::smoothNoise( float x, float y, float scale )
{
  float corners = ( mPerlin.Get(x-scale, y-scale) + mPerlin.Get(x+scale, y-scale) + mPerlin.Get(x-scale, y+scale) + mPerlin.Get(x+scale, y+scale) ) / 16;
  float sides   = ( mPerlin.Get(x-scale, y) + mPerlin.Get(x+scale, y) + mPerlin.Get(x, y-scale) + mPerlin.Get(x, y+scale) ) /  8;
  float center  =  mPerlin.Get(x, y) / 4;
  
  return corners + sides + center;
}

void TerrainEngine::terrainQueuePush( TerrainQueue& tq, bool inFront )
{
  if( terrainQueue.size() ) {
    std::deque<TerrainQueue>::iterator it;
    for ( it = terrainQueue.begin(); it < terrainQueue.end(); it++ ) {
      TerrainQueue& t = *it;
      if( ( t.x == tq.x ) && ( t.y == tq.y ) ) {
        if( t.load == tq.load ) {
          if( inFront ) {
            terrainQueue.erase( it );
          } else {
            std::stringstream reject;
            reject << "Rejecting (" << tq.x << ", " << tq.y << ")[load=" << tq.load << "] from queue: Duplicate";
            Ogre::LogManager::getSingletonPtr()->logMessage( reject.str() );
            return;
          }
        }
        /*if( t.load != tq.load ) { // They would cancel each other out
          std::stringstream reject;
          reject << "Rejecting (" << tq.x << ", " << tq.y << ")[load=" << tq.load << "] and Removing (" << t.x << ", " << t.y << ")[load=" << t.load << "] from queue: Actions would cancel each other out";
          Ogre::LogManager::getSingletonPtr()->logMessage( reject.str() );
          terrainQueue.erase( it );
          return;
        }*/
      }
    }
  }
  if( inFront == true ) {
    terrainQueue.push_front( tq );
  } else {
    terrainQueue.push_back( tq );
  }
}

TerrainEngine::TerrainQueue *TerrainEngine::terrainQueueNext( void )
{
  if( terrainQueue.size() == 0 ) return NULL;
  return (TerrainQueue *)&terrainQueue.front();
}

void TerrainEngine::terrainQueuePop( void )
{
  if( terrainQueue.size() > 0 ) terrainQueue.pop_front();
}

void TerrainEngine::terrainQueueAtDistance( long int d, bool load )
{
  Ogre::Terrain *t;
  TerrainQueue tq;
  
  long int xmin = mSlotX - d;
  long int xmax = mSlotX + d;
  long int ymin = mSlotY - d;
  long int ymax = mSlotY + d;
  
  // Top and Bottom rows
  for( long int _x = xmin; _x <= xmax; _x++ ) {
    // Top
    t = mTerrainGroup->getTerrain( _x, ymin );
    tq.terrain = t;
    tq.x = _x;
    tq.y = ymin;
    tq.load = load;
    terrainQueuePush( tq );
    
    // Bottom
    t = mTerrainGroup->getTerrain( _x, ymax );
    tq.terrain = t;
    tq.x = _x;
    tq.y = ymax;
    tq.load = load;
    terrainQueuePush( tq );
  }
  
  // Left and Right rows
  for( long int _y = ymin+1; _y < ymax; _y++ ) {
    // Left
    t = mTerrainGroup->getTerrain( xmin, _y );
    tq.terrain = t;
    tq.x = xmin;
    tq.y = _y;
    tq.load = load;
    terrainQueuePush( tq );
    
    // Right
    t = mTerrainGroup->getTerrain( xmax, _y );
    tq.terrain = t;
    tq.x = xmax;
    tq.y = _y;
    tq.load = load;
    terrainQueuePush( tq );
  }
}

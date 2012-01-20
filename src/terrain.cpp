
#include "terrain.hpp"

TerrainEngine::TerrainEngine( Ogre::SceneManager *scenemgr, Ogre::Camera *cam, Ogre::Light* light, bool usePaging, Ogre::String file_prefix, Ogre::String file_suffix )
  : mTerrainGroup(0)
  , mTerrainPaging(0)
  , mTerrainPos(1000,0,5000)
  , mPageManager(0)
  , mLayerEdit(1)
  , mTerrainsImported(false)
  , mSceneMgr(scenemgr)
  , mUsePaging( usePaging )
{
  terrainSelectClear();
  
  mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

  //mEditMarker = mSceneMgr->createEntity( "editMarker", "sphere.mesh" );
  //mEditNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  //mEditNode->attachObject( mEditMarker );
  //mEditNode->setScale( mSelected.radius, mSelected.radius, mSelected.radius );
  
  mTerrainGroup = OGRE_NEW Ogre::TerrainGroup( mSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE );
  mTerrainGroup->setFilenameConvention( file_prefix, file_suffix );
  mTerrainGroup->setOrigin( mTerrainPos );

  if( mUsePaging ) {
    mPageManager = OGRE_NEW Ogre::PageManager( );
    mPageManager->setPageProvider( &mTerrainPageProvider );
    mPageManager->addCamera( cam );
    
    mTerrainPaging = OGRE_NEW Ogre::TerrainPaging( mPageManager );
    Ogre::PagedWorld* world = mPageManager->createWorld( );
    mTerrainPaging->createWorldSection( world, mTerrainGroup, 2000, 3000, TERRAIN_PAGE_MIN_X, TERRAIN_PAGE_MIN_Y, TERRAIN_PAGE_MAX_X, TERRAIN_PAGE_MAX_Y );
  } else {
    for (long x = TERRAIN_PAGE_MIN_X; x <= TERRAIN_PAGE_MAX_X; ++x)
      for (long y = TERRAIN_PAGE_MIN_Y; y <= TERRAIN_PAGE_MAX_Y; ++y)
        defineTerrain(x, y, true);
    // sync load since we want everything in place when we start
    mTerrainGroup->loadAllTerrains(true);
  }
  
  configureTerrainDefaults( light );
  
  if ( mTerrainsImported ) {
    Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
    while(ti.hasMoreElements())
    {
      Ogre::Terrain* t = ti.getNext()->instance;
      initBlendMaps(t);
    }
  }
  
  mTerrainGroup->freeTemporaryResources();
}

TerrainEngine::~TerrainEngine( )
{
  terrainSelectClear();
  OGRE_DELETE mTerrainPaging;
  OGRE_DELETE mPageManager;
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
  // I have no idea what this is about
  //if (mHeightUpdateCountDown == 0)
  //        mHeightUpdateCountDown = mHeightUpdateRate;
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
    if (mTerrainsImported)
    {
      mTerrainGroup->saveAllTerrains(true);
      mTerrainsImported = false;
    }
  }
}

void TerrainEngine::defineTerrain( long x, long y, bool flat )
{
  if (flat) {
    mTerrainGroup->defineTerrain( x, y, 0.0f );
    mTerrainsImported = true;
  } else {
    Ogre::String filename = mTerrainGroup->generateFilename(x, y);
    if( Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename) && !flat ) {
      mTerrainGroup->defineTerrain(x, y);
    } else {
      //Ogre::Image img;
      //getTerrainImage(x % 2 != 0, y % 2 != 0, img);
      //mTerrainGroup->defineTerrain(x, y, &img);
      mTerrainsImported = true;
      mTerrainGroup->defineTerrain( x, y, 0.0f );
    }
  }
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
  defaultimp.minBatchSize = 33;
  defaultimp.maxBatchSize = 65;
  
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

bool TerrainEngine::TerrainPageProvider::prepareProceduralPage( Ogre::Page* page, Ogre::PagedWorldSection* section )
{
  OgreConsole::getSingleton().print("TerrainEngine::TerrainPageProvider::prepareProceduralPage");
  return false;
}

bool TerrainEngine::TerrainPageProvider::loadProceduralPage( Ogre::Page* page, Ogre::PagedWorldSection* section )
{
  OgreConsole::getSingleton().print("TerrainEngine::TerrainPageProvider::loadProceduralPage");
  return false;
}

bool TerrainEngine::TerrainPageProvider::unloadProceduralPage( Ogre::Page* page, Ogre::PagedWorldSection* section )
{
  OgreConsole::getSingleton().print("TerrainEngine::TerrainPageProvider::unloadProceduralPage");
  return true;
}

bool TerrainEngine::TerrainPageProvider::unprepareProceduralPage( Ogre::Page* page, Ogre::PagedWorldSection* section )
{
  OgreConsole::getSingleton().print("TerrainEngine::TerrainPageProvider::unprepareProceduralPage");
  return true;
}
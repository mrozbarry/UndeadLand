
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreCamera.h>
#include <Paging/OgrePage.h>
#include <Paging/OgrePageManager.h>
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include <Terrain/OgreTerrainPagedWorldSection.h>
#include <Terrain/OgreTerrainPaging.h>
#include <Terrain/OgreTerrainQuadTreeNode.h>
#include <Terrain/OgreTerrainMaterialGeneratorA.h>

#include <cmath>
#include <algorithm>
#include <deque>

#include "Gorilla.h"
#include "OgreConsoleForGorilla.h"
#include "perlin.h"
#include "main.hpp"

#ifndef __terrain_hpp__
#define __terrain_hpp__ 1

#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE  1025

#define TERRAIN_PAGE_MIN_X  0
#define TERRAIN_PAGE_MIN_Y  0
#define TERRAIN_PAGE_MAX_X  0
#define TERRAIN_PAGE_MAX_Y  0

float smoothNoise( Perlin *mPerlin, float x, float y, float scale );

class TerrainPP : public Ogre::PageProvider {
public:
  TerrainPP( int seed )
    : mPerlin(16, 0.0001, 900, seed)
  { }
  
  bool prepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) {
    Ogre::TerrainGroup* pGroup = ((Ogre::TerrainPagedWorldSection*)section)->getTerrainGroup();
    long x, y; pGroup->unpackIndex(page->getID(), &x, &y);
    pGroup->defineTerrain( x, y, 0.0f );
    return true;
  }
  bool loadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) {
    Ogre::TerrainGroup* pGroup = ((Ogre::TerrainPagedWorldSection*)section)->getTerrainGroup();
    long x, y; pGroup->unpackIndex(page->getID(), &x, &y);
    
    Ogre::String filename = pGroup->generateFilename(x, y);
    if( Ogre::ResourceGroupManager::getSingleton().resourceExists(pGroup->getResourceGroup(), filename) ) {
      pGroup->loadTerrain( x, y, true );
    } else {
      Ogre::Terrain *terrain = pGroup->getTerrain( x, y );
      if( terrain ) {
        Ogre::Vector3 topleft, bottomright;
        terrain->getPosition( 0, 0, 0, &topleft );
        terrain->getPosition( terrain->getWorldSize(), 0, terrain->getWorldSize(), &bottomright );
        std::stringstream bbdata;
        bbdata << "Bounding box[" << x << "," << y << "]: (" << topleft.x << ", " << topleft.z << "), (" << bottomright.x << ", " << bottomright.z << ")";
        Ogre::LogManager::getSingletonPtr()->logMessage(bbdata.str());
        for( long tx = topleft.x; tx < bottomright.x; tx++ ) {
          for( long tz = topleft.z; tz < bottomright.z; tz++ ) {
            //Ogre::Vector3 p;
            //terrain->getTerrainPosition( (Ogre::Real)tx, (Ogre::Real)0, (Ogre::Real)tz, &p );
            terrain->setHeightAtPoint( tx, tz, smoothNoise( &mPerlin, tx, tz, 0.01f ) );
          }
        }
        terrain->update();
        pGroup->update();
      }
    }
    return true;
  }
  bool unloadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) {
    return true;
  }
  bool unprepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) {
    return true;
  }
  
  Perlin mPerlin;  
};

class TerrainEngine
{
public:
  TerrainEngine( Ogre::String seed, Ogre::Root *root, Ogre::SceneManager *scenemgr, Ogre::Camera *cam, Ogre::Light* light, Ogre::String file_prefix = "undeadland", Ogre::String file_suffix = "dat" );
  ~TerrainEngine( );
  
  typedef struct TerrainSelect {
    Ogre::Terrain   *terrain;
    Ogre::Vector3   position;
    Ogre::Real      radius;
  } TerrainSelect;
  
  void terrainSelect( Ogre::Terrain *terrain, Ogre::Vector3 position, Ogre::Real radius = 10.0f );
  TerrainSelect *terrainSelect( void );
  bool terrainSelected( void );
  void terrainSelectClear( void );
  
  void changeTerrainHeight( Ogre::Real scale = 1.0f );
  void changeTerrainTexture( Ogre::Real rate = 1.0f );
  void changeTextureLayer( Ogre::uint8 layer );
  
  void onFrameRenderingQueued( );
  
  bool getTerrainLocked( void );
  void setTerrainLocked( bool lock );
  
  void fixCameraTerrain( Ogre::Camera *cam, float height = 35.0f );

protected:
  bool defineTerrain( long x, long y );
  void initBlendMaps( Ogre::Terrain *terrain );
  void configureTerrainDefaults( Ogre::Light *light );
  
  //void loadTerrain( long int x, long int y, bool unload = false );
  
private:
  //float smoothNoise( float _x, float _y, float scale = 1.0f );
  
  Ogre::Root *mRoot;
  
  Ogre::Camera *mCamera;
  
  Ogre::TerrainGlobalOptions* mTerrainGlobals;
  Ogre::TerrainGroup* mTerrainGroup;
  Ogre::Vector3 mTerrainPos;
  
  Ogre::SceneNode* mEditNode;
  Ogre::Entity* mEditMarker;
  
  TerrainSelect mSelected;
  
  Ogre::SceneManager *mSceneMgr;

  Ogre::uint8 mLayerEdit;
  //Ogre::SceneNode* mEditNode;
  bool mTerrainsImported;
  
  long int mSlotX, mSlotY;
  bool mUpdateTerrains;
  
  bool mLockTerrains;
  
  TerrainPP mTerrainPageProvider;
  Ogre::TerrainPaging *mTerrainPaging;
  Ogre::PageManager *mPageManager;
  
  //typedef std::list<Ogre::Entity*> EntityList;
};

#endif /* __terrain_hpp__ */

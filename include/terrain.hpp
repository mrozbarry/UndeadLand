
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include <Terrain/OgreTerrainQuadTreeNode.h>
#include <Terrain/OgreTerrainMaterialGeneratorA.h>
#include <Terrain/OgreTerrainPaging.h>

#include <cmath>
#include <algorithm>

#include "Gorilla.h"
#include "OgreConsoleForGorilla.h"

#ifndef __terrain_hpp__
#define __terrain_hpp__ 1

#define TERRAIN_PAGE_MIN_X 0
#define TERRAIN_PAGE_MIN_Y 0
#define TERRAIN_PAGE_MAX_X 0
#define TERRAIN_PAGE_MAX_Y 0

#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE 129

class TerrainEngine {
public:
  TerrainEngine( Ogre::SceneManager *scenemgr, Ogre::Camera *cam, Ogre::Light* light, bool usePaging = false, Ogre::String file_prefix = "undeadland", Ogre::String file_suffix = "dat" );
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
  
protected:
  void defineTerrain( long x, long y, bool flat = false );
  void initBlendMaps( Ogre::Terrain *terrain );
  void configureTerrainDefaults( Ogre::Light *light );

  Ogre::TerrainGlobalOptions* mTerrainGlobals;
  Ogre::TerrainGroup* mTerrainGroup;
  //bool mPaging;
  Ogre::TerrainPaging* mTerrainPaging;
  Ogre::PageManager* mPageManager;
  Ogre::Vector3 mTerrainPos;
  
  Ogre::SceneNode* mEditNode;
  Ogre::Entity* mEditMarker;
  
  TerrainSelect mSelected;
  
  Ogre::SceneManager *mSceneMgr;

  Ogre::uint8 mLayerEdit;
  //Ogre::SceneNode* mEditNode;
  bool mTerrainsImported;
  
  bool mUsePaging;

  class TerrainPageProvider : public Ogre::PageProvider
  {
  public:
    bool prepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section);
    bool loadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section);
    bool unloadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section);
    bool unprepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section);
  };
  TerrainPageProvider mTerrainPageProvider;
  
  //typedef std::list<Ogre::Entity*> EntityList;
};

#endif /* __terrain_hpp__ */

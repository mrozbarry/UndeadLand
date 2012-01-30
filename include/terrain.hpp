
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreCamera.h>
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
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
#define TERRAIN_SIZE  513
#define TERRAIN_DIST  2

#define USE_QUEUE 1

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
  
  typedef struct TerrainQueue {
    Ogre::Terrain   *terrain;
    long int        x, y;
    bool            load;
  } TerrainQueue;
  
#if 0
  class TerrainHandler : public Ogre::WorkQueue::RequestHandler, public Ogre::WorkQueue::ResponseHandler
  {
    Ogre::WorkQueue::Response* handleRequest(const Ogre::WorkQueue::Request* req, const Ogre::WorkQueue* srcQ)
    {
      TerrainQueue *tq = req->getData();
      
    }
    
    void handleResponse(const Ogre::WorkQueue::Response* res, const Ogre::WorkQueue* srcQ)
    {
    }
    
    bool canHandleRequest(const Ogre::WorkQueue::Request* req, const Ogre::WorkQueue* srcQ)
    {
      return RequestHandler::canHandleRequest(req, srcQ);
    }
    
    bool canHandleResponse(const Ogre::WorkQueue::Response* res, const Ogre::WorkQueue* srcQ)
    {
      return true;
    }
  }
#endif
  
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
  
  void loadTerrain( long int x, long int y, bool unload = false );
  
private:
  float smoothNoise( float _x, float _y, float scale = 1.0f );
  
#ifdef USE_QUEUE
  void terrainQueuePush( TerrainQueue& tq, bool inFront = false );
  TerrainQueue *terrainQueueNext( void );
  void terrainQueuePop( void );
#endif
  
  void terrainQueueAtDistance( long int d, bool load = true );
  
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
  
  Perlin mPerlin;
  
  std::deque< TerrainQueue > terrainQueue;
  
  //typedef std::list<Ogre::Entity*> EntityList;
};

#endif /* __terrain_hpp__ */

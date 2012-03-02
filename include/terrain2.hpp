
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreCamera.h>
#include <OgreManualObject.h>

#include <OgrePageManager.h>

#include <PolyVoxCore/MaterialDensityPair.h>
#include <PolyVoxCore/CubicSurfaceExtractorWithNormals.h>
#include <PolyVoxCore/SurfaceExtractor.h>
#include <PolyVoxCore/SurfaceMesh.h>
#include <PolyVoxCore/LargeVolume.h>

#include <cmath>
#include <algorithm>
#include <deque>

#include "OgreConsoleForGorilla.h"
#include "perlin.h"
#include "main.hpp"

#ifndef __terrain2_hpp__
#define __terrain2_hpp__

class TerrainPageProvider : public PageProvider
{
public:
  TerrainPageProvider();
  ~TerrainPageProvider();
  
  virtual bool prepareProceduralPage( Ogre::Page *page, Ogre::PagedWorldSection *section );
  virtual bool loadProceduralPage( Ogre::Page *page, Ogre::PagedWorldSection *section );
  virtual bool unloadProceduralPage( Ogre::Page *page, Ogre::PagedWorldSection *section );
  virtual bool unprepareProceduralPage( Ogre::Page *page, Ogre::PagedWorldSection *section );
  
  /* Eventually, for saving and loading, I'll need to write these...
  virtual Ogre::StreamSerialiser *readWorldStream( const Ogre::String &filename );
  virtual Ogre::StreamSerialiser *writeWorldStream( const Ogre::String &filename );
  virtual Ogre::StreamSerialiser *readPageStream( Ogre::PageID pageID, Ogre::PagedWorldSection *section );
  virtual Ogre::StreamSerialiser *writePageStream( Ogre::PageID pageID, Ogre::PagedWorldSection *section );*/
};

class TerrainChunk
{
public:
  // Dummy constructor
  TerrainChunk( void );
  // Construct based on noise
  TerrainChunk( const PolyVox::Region& region, Perlin *noise, Ogre::SceneManager *scenemgr );
  // Construct based on file
  TerrainChunk( std::string path, Ogre::SceneManager *scenemgr );
  // Construct a flat plane <height> high
  TerrainChunk( int height, Ogre::SceneManager *scenemgr );
  ~TerrainChunk();
  
  void loadFromNoise( Perlin *noise );
  void loadFromSolid( int height );
  void loadFromFile( std::string path );
  
  bool isModified( void );
  
  void saveAs( std::string path );
  void save( void );
  
protected:
  PolyVox::Region             mRegion;
  PolyVox::Volume<Material16> mVolume;
    
private:
  bool                        mModified;
  std::string                 mLastPath;
  
  Ogre::SceneManager          *mSceneMgr;
  Ogre::ManualObject          *mMesh;
  Ogre::SceneNode             *mNode;
};

class TerrainEngine2
{
public:
  TerrainEngine2( Ogre::String seed, Ogre::Root *root, Ogre::SceneManager *scenemgr, Ogre::Camera *cam, Ogre::Light* light );
  ~TerrainEngine2();
  
  void onFrameRenderingQueued( );
  
  void fixCameraTerrain( Ogre::Camera *cam, float height = 35.0f );

protected:
  
private:
  Ogre::Root *mRoot;
  Ogre::Camera *mCamera;
  Ogre::SceneManager *mSceneMgr;

  std::list< TerrainChunk > *chunks;
};

#endif /* __terrain2_hpp__ */

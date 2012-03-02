
#include "terrain2.hpp"

/* Global instance of perlin noise...polyvox is sort of messy that way :( */
Perlin *gPerlin = NULL;




/****************************************************/
/* Terrain Chunk                                    */
/****************************************************/
TerrainChunk::TerrainChunk( void )
  : mModified(false)
  , mLastPath("")
  , mSceneMgr(NULL)
  , mMesh(NULL)
  , mNode(NULL)
{  }

TerrainChunk::TerrainChunk( const PolyVox::Region& region, Perlin *noise, Ogre::SceneManager *scenemgr )
  : mModified(false)
  , mLastPath("")
  , mSceneMgr(scenemgr)
  , mMesh(NULL)
  , mNode(NULL)
{
  mRegion = region;
  loadFromNoise( noise );
}

TerrainChunk::TerrainChunk( std::string path, Ogre::SceneManager *scenemgr )
  : mModified(false)
  , mLastPath(path)
  , mSceneMgr(scenemgr)
  , mMesh(NULL)
  , mNode(NULL)
{
  loadFromFile( path );
}

TerrainChunk::TerrainChunk( int height, Ogre::SceneManager *scenemgr )
  : mModified(false)
  , mLastPath("")
  , mSceneMgr(scenemgr)
  , mMesh(NULL)
  , mNode(NULL)
{
  loadFromSolid( height );
}

TerrainChunk::~TerrainChunk()
{
  
}

void TerrainChunk::loadFromNoise( Perlin *noise );
void TerrainChunk::loadFromSolid( int height );
void TerrainChunk::loadFromFile( std::string path )
{
  /* TODO: determine format to save chunks */
}
bool TerrainChunk::isModified( void );
void TerrainChunk::saveAs( std::string path );
void TerrainChunk::save( void );


/****************************************************/
/* TerrainEngine2                                   */
/****************************************************/
TerrainEngine2::TerrainEngine2( Ogre::String seed, Ogre::Root *root, Ogre::SceneManager *scenemgr, Ogre::Camera *cam, Ogre::Light* light )
  : mRoot(root)
  , mSceneMgr(scenemgr)
  , mCamera(cam)
{
  /* Setup Perlin noise if it isn't already, and if it is, reconfirm values */
  if( gPerlin ) delete gPerlin;
  gPerlin = new Perlin( 16, 0.0001, 900, hashString( seed ) );
}

TerrainEngine2::~TerrainEngine2()
{
  if( gPerlin ) { delete gPerlin; gPerlin = NULL; }
}

void TerrainEngine2::onFrameRenderingQueued( )
{
}

void TerrainEngine2::fixCameraTerrain( Ogre::Camera *cam, float height = 35.0f )
{
}

static void TerrainEngine2::load(const ConstVolumeProxy<MaterialDensityPair44>& volume, const PolyVox::Region& reg)
{
	for(int x = reg.getLowerCorner().getX(); x <= reg.getUpperCorner().getX(); x++)
	{
		for(int y = reg.getLowerCorner().getY(); y <= reg.getUpperCorner().getY(); y++)
		{
			float perlinVal = gPerlin->Get(x / static_cast<float>(255-1), y / static_cast<float>(255-1));
			perlinVal += 1.0f;
			perlinVal *= 0.5f;
			perlinVal *= 255;
			for(int z = reg.getLowerCorner().getZ(); z <= reg.getUpperCorner().getZ(); z++)
			{
				MaterialDensityPair44 voxel;
				if(z < perlinVal)
				{
					const int xpos = 50;
					const int zpos = 100;
					if((x-xpos)*(x-xpos) + (z-zpos)*(z-zpos) < 200) {
						// tunnel
						voxel.setMaterial(0);
						voxel.setDensity(VoxelTypeTraits<MaterialDensityPair44>::MinDensity);
					} else {
						// solid
						voxel.setMaterial(245);
						voxel.setDensity(VoxelTypeTraits<MaterialDensityPair44>::MaxDensity);
					}
				}
				else
				{
					voxel.setMaterial(0);
					voxel.setDensity(VoxelTypeTraits<MaterialDensityPair44>::MinDensity);
				}

				volume.setVoxelAt(x, y, z, voxel);
			}
		}
	}
}

static void TerrainEngine2::unload(const ConstVolumeProxy<MaterialDensityPair44>& volume, const PolyVox::Region& reg)
{
}

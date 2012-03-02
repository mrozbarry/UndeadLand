
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#include <cmath>
#include <algorithm>
#include <list>

#include "global.hpp"
#include "stats.hpp"

#ifndef __creatures_hpp__
#define __creatures_hpp__ 1

class Creature;

typedef struct Relations {
  Creature    *who;
  float       trust;
  float       companion;
  long int    proximity_time;
} Relations;

class Creature // : public OIS::KeyListener, public OIS::MouseListener
{
public:
  Creature( Ogre::String name, Ogre::Entity *entity );
  ~Creature( );
  
protected:
  Ogre::String        name;
  WorldPosition       position;
  Ogre::Entity        *ent;
  
  float               infection;
  float               infection_rate;
  float               health;
  float               regenerate_rate;
  
  bool                autonomous;
  
  std::list<Relations> relations;
};

class LocalPlayer : public OIS::KeyListener, public OIS::MouseListener
{
public:
  LocalPlayer( Ogre::String handle, WorldPosition pos );
  ~LocalPlayer( );
  
  Ogre::Vector3 getRealPosition( void ) const;
  WorldPosition getWorldPosition( void ) const;
  
  bool hasInput( void );
  void grabInput( bool toggle );
  
protected:
  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );
  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

private:
  WorldPosition   position;
  bool            mHasInput;
};

#endif  /* __terrain_hpp__ */

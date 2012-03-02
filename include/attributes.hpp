
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#include "creatures.hpp"

#ifndef __attributes_hpp__
#define __attributes_hpp__ 1

typedef struct Attributes {
  bool            immunity;
  float           infection_rate;
  float           maximum_health;
  float           health_regeneration;
} Attributes;

/*class Perk
{
public:
  Perk( Ogre::String name, Ogre::String desc );
  ~Perk( );
};*/

#endif  /* __attributes_hpp__ */

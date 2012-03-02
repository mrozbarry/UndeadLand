
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#ifndef __inventory_hpp__
#define __inventory_hpp__ 1

class Item
{
public:
  Item( Ogre::String name, Ogre::String desc, float mass, bool canEquip = false );
  ~Item( );
};

class Inventory
{
public:
  Inventory( bool backpack );
  ~Inventory( );

};

#endif  /* __inventory_hpp__ */

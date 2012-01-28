

#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreOverlay.h>
#include <OgreFontManager.h>

#include <string>
#include <list>

#ifndef __hud_hpp__
#define __hud_hpp__ 1

class HudLayer, Ogre::FrameListener
{
public:
  HudLayer();
  ~HudLayer();
  
  void init( Ogre::Root *root );
  
  void update( void );
  
  virtual bool frameStarted(const Ogre::FrameEvent &evt);
  virtual bool frameEnded(const Ogre::FrameEvent &evt);
  
protected:
  
private:
};

#endif /* __hud_hpp__ */

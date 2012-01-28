
#include "hud.hpp"

#if OGRE_VERSION < 67584 // 1.8.0
template<> Hud* Ogre::Singleton<Hud>::ms_Singleton=0;
#else
template<> Hud* Ogre::Singleton<Hud>::msSingleton=0;
#endif

HudLayer::HudLayer()
{
}

HudLayer::~HudLayer()
{
}

void HudLayer::init( Ogre::Root *root )
{
}

void HudLayer::update( void )
{
}

virtual bool HudLayer::frameStarted(const Ogre::FrameEvent &evt)
{
}

virtual bool HudLayer::frameEnded(const Ogre::FrameEvent &evt)
{
}

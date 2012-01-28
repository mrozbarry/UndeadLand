
#include "main.hpp"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{

  UndeadLand app;
  
  try {
    app.go();
  } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    std::cerr << "An exception has occured: " <<
    e.getFullDescription().c_str() << std::endl;
#endif
  }

  return 0;
}

int hashString( std::string value )
{
  int hash = 0;
  int n = (int)value.size();
  for( int i = 0; i < n; i++ ) {
    hash += ((int)value[i])*31^(n-(i+1));
  }
  return hash;
}

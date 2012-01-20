
/*********************************************************************************

  Undead Land Main Header

 *********************************************************************************/

#ifndef __main_hpp__
#define __main_hpp__  1

#include <string>

#ifndef UDL_WORKING_TITLE

#define UDL_WORKING_TITLE "Undead Land"
#define UDL_VER_MAJ   0
#define UDL_VER_MIN   0
#define UDL_VER_REV   9
#define UDL_VER_STAGE "alpha"

#endif

#include "BaseApplication.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT );
#else
int main(int argc, char *argv[]);
#endif


#endif /* __main_hpp__ */

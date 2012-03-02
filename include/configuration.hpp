
// TODO: finish me

#include <OgreRoot.h>
#include <OgreConfigFile.h>

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#ifndef __configuration_hpp__
#define __configuration_hpp__ 1

enum ControlDefinition {
  CD_FORWARD        = 0,
  CD_BACKWARD,
  CD_LEFT,
  CD_RIGHT,
  CD_RUN,
  CD_JUMP,
  CD_INTERACT,
  
  CD_LOOKUP,
  CD_LOOKDOWN,
  CD_LOOKLEFT,
  CD_LOOKRIGHT,
  CD_LOOKRESET,
  
  CD_TOGGLEWEAPON,
  CD_TOGGLEPRECISION,
  CD_TRIGGER1,
  CD_TRIGGER2,
  
  CD_INVENTORY,
  CD_MENU,
  
  CD_TEXTCHAT,
  CD_VOICECHAT,
  CD_TOGGLECONSOLE,
  
  CD_MAX_DEFINITION
};

enum ControlType {
  CT_KEYBOARD       = 0,
  CT_MOUSE_BUTTON,
  CT_MOUSE_MOVE,
  CT_JOYSTICK,
  
  CT_MAX_TYPES
};

typedef struct ControlButtonSetting {
  bool          disabled;
  bool          can_hold;
  OIS::KeyCode  key;
} ControlSetting;

class ControlConfig {
public:
  ControlConfig( void );
  ControlConfig( Ogre::String name );
  ~ControlConfig( void );
  
  bool loadConfiguration( Ogre::String name );
  void resetConfiguration( void );
  
protected:
  controls[CD_MAX_DEFINITION];
  float   sensitivity[2];
  float   hold_time;
};

#endif /* __configuration_hpp__ */

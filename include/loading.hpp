
#ifndef __loading_hpp__
#define __loading_hpp__ 1

#include <iostream>
#include <fstream>

#include <OgreString.h>
#include <OgreFontManager.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#ifndef FONT_UNICODE
# define FONT_UNICODE 1
#endif

/*******************************************************************
  Base Classes for loading resources
 *******************************************************************/

class LoadTask {
public:
  LoadTask( Ogre::String task_name )
  : m_task(task_name), m_state(0), m_message(Ogre::String::BLANK), m_ran(false)
  { }
  
  ~LoadTask( )
  { }
  
  bool hasRun( void ) { return this->m_ran; }
  
  void setState( unsigned int state ) { this->m_state = state; }
  void setMessage( Ogre::String message ) { this->m_messasge = message; }
  
  virtual void setStateSuccess( Ogre::String msg = L"No Errors" ) { this->setState( 0 ); this->setMessage( msg ); }
  virtual void setStateFailure( Ogre::String msg = L"Error" ) { this->setState( 1 ); this->setMessage( msg ); }
  
  Ogre::String getTaskName( void ) { return this->m_task; }
  int getState( Ogre::String& message ) { message = m_message; return this->m_state; }
  
  bool runTask( void ) { bool value = this->doTask(); this->m_ran = true; return value; }
  
private:
  virtual bool doTask() = 0; // Needs to be overloaded!
  
  Ogre::String  m_task;
  unsigned int  m_state;
  Ogre::String  m_message;
  bool          m_ran;
};

class LoadTaskGroup {
public:
  LoadTaskGroup( Ogre::String task_group )
  : m_group( task_group ), m_completed(0.0), m_failed(0), m_task_index(-1)
  { }
  
  ~LoadTaskGroup( )
  {
    for( unsigned int i = 0; i < this->m_tasks.size(); i++ ) delete this->m_tasks[i];
  }
  
  LoadTaskGroup operator << ( LoadTask *task ) { this->m_tasks.push_back( task ); }
  
  LoadTask *doNextTask( void )
  {
    if( ++this->m_task_index >= (int)this->m_tasks.size() ) return NULL;
    LoadTask *task = this->m_tasks[this->m_task_index];
    bool ret = task->runTask( );
    if( ret == false ) this->m_failed++;
    this->m_completed++;
    return task;
  }
  
  Ogre::String getGroupName( void ) { return this->m_group; }
  float getCompleted( void )
  {
    float count = (float)this->m_task_index + 1.0f;
    float max = (float)this->m_tasks.size();
    return (count / max) * 100;
  }
  
private:
  Ogre::String            m_group;
  float                   m_completed;
  int                     m_failed;
  std::vector<LoadTask *> m_tasks;
  int                     m_task_index;
};

/*******************************************************************
  Helper Classes for loading resources
 *******************************************************************/

/* This code was adapted from http://www.ogre3d.org/tikiwiki/RenderTTFFontToTexture&structure=Cookbook (on Jan 2, 2012) */
class LoadFont : public LoadTask {
public:
  static const Ogre::UTFString cacheSuffix = "_cache";

  LoadFont( Ogre::String font )
  : LoadTask( "Font::" + font ), m_font( font ), m_error(false);
  { }
  
  ~LoadFont( )
  { }
  
  bool isCached( void )
  {
    Ogre::FontPtr font_org = Ogre::FontManager::getSingleton().getByName( this->m_font );
    Ogre::FontPtr font_cache = Ogre::FontManager::getSingleton().getByName( this->m_font + LoadFont::cacheSuffix );

    if( font_org.isNull() )
    {
      this->setStateFailure( L"Font does not exist!" );
      return false;
    }
    if(font_org->getType() != Ogre::FT_TRUETYPE)
    {
      this->setStateFailure( L"Font is not TrueType!" );
      return false;
    }

    if( font_cache.isNull() ) return false;
    return true;
  }
  
  bool buildFontCache( Ogre::String fontName, bool load = false )
  {
    Ogre::FontPtr font = Ogre::FontManager::getSingleton().getByName( this->m_font );
    if( font.isNull() ) { this->setStateFailure( L"Font resource not found!" ); return false; }
 
    if( font->getType() != Ogre::FT_TRUETYPE ) { this->setStateFailure( L"Font is not truetype!" ); return false; }
 
    if( !font->isLoaded() )
        font->load();
 
    Ogre::String texname = font->getMaterial()->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
 
 
    Ogre::TexturePtr fontTexture = Ogre::TextureManager::getSingleton().getByName( texname );
    Ogre::HardwarePixelBufferSharedPtr fontBuffer = fontTexture->getBuffer();
 
    if( fontBuffer.isNull() ) { this->setStateFailure( L"Font buffer cannot be created!" ); return false; }
 
    // create a remporary buffer that holds the font
    size_t nBuffSize = fontBuffer->getSizeInBytes();
    Ogre::uint8* buffer = (Ogre::uint8*)calloc( nBuffSize, sizeof( Ogre::uint8 ) ); 
    Ogre::PixelBox fontPb( fontBuffer->getWidth(), fontBuffer->getHeight(), fontBuffer->getDepth(), fontBuffer->getFormat(), buffer );          
    fontBuffer->blitToMemory( fontPb );
 
    // create new render texture
    Ogre::String texNameManual = "FontTextureRTT_" + font->getName();
    Ogre::TexturePtr t = Ogre::TextureManager::getSingleton().createManual( texNameManual, "General", Ogre::TEX_TYPE_2D, fontBuffer->getWidth(), fontBuffer->getHeight(), fontBuffer->getDepth(), 1, fontBuffer->getFormat(), Ogre::TU_RENDERTARGET );
    Ogre::TexturePtr fontTextureRTT = Ogre::TextureManager::getSingleton().getByName( texNameManual );
    fontTextureRTT->getBuffer()->blitFromMemory( fontPb );        
 
    // now render to file
    Ogre::RenderTexture* pRenderTexture = fontTextureRTT->getBuffer()->getRenderTarget();
    pRenderTexture->update();
    Ogre::String outImageName = font->getName()+ cache_name + ".png";
    pRenderTexture->writeContentsToFile( outImageName );
 
    // free stuff
    free(buffer);
    Ogre::TextureManager::getSingleton().remove( texNameManual );
 
    // save texture font infos
    Ogre::UTFString out_text = font->getName() + cache_name + "\n{\n\ttype\timage\n\tsource\t" + outImageName + "\n";
 
    std::vector<Ogre::Font::CodePointRange> ranges = font->getCodePointRangeList();
    for(std::vector<Ogre::Font::CodePointRange>::iterator it = ranges.begin(); it != ranges.end(); it++)
    {
      for(Ogre::uint32 i=it->first;i<=it->second;i++)
      {
          Ogre::Font::GlyphInfo gi(0, Ogre::Font::UVRect(), 0);
          try { gi = font->getGlyphInfo(i); }
          catch(...) { continue; }
          wchar_t tmp[20];
          swprintf(tmp, 20, L"%c", i);
          out_text.append( "\tglyph " + Ogre::UTFString( tmp ) + " " + Ogre::StringConverter::toString( gi.uvRect.left ) + " " + Ogre::StringConverter::toString( gi.uvRect.top ) + " " + Ogre::StringConverter::toString( gi.uvRect.right ) + " " + Ogre::StringConverter::toString( gi.uvRect.bottom ) + "\n" );
      }
    }
    out_text.append( "}\n" );
 
    Ogre::String defFileName = font->getName() + cache_name + ".fontdef";
    std::ofstream f;
    f.open( defFileName.c_str() );
      f << out_text;
    f.close();
    Ogre::LogManager::getSingleton().logMessage( "generated font cache for font " + fontName + " (" + outImageName + ", " + defFileName + ")" );
 
    if( load )
    {
      Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
      Ogre::FontPtr cached_font = Ogre::FontManager::getSingleton().getByName( fontName + cache_name );
      if( cached_font.isNull() ) { this->setStateFailure( L"Something went wrong, and the font cache couldn't be loaded" ); return false; }
      cached_font->load();
      return true;
    }
    return true;
  }
  
private:
  virtual bool doTask()
  {
    bool error = ( this->getState( ) != 0 ) ? true : false;
    if( !this->isCached() && !error ) return this->buildFontCache( this->m_font, true );
    if( this->isCached() ) return true;
  }
  
  Ogre::FontPtr                       m_font;
};

#endif

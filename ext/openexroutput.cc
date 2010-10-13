/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007   Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <IexThrowErrnoExc.h>
#include <ImfChannelList.h>
#include <ImfIO.h>
#include <ImfOutputFile.h>
#include <errno.h>
#include "rubytools.hh"
#if (RUBYVM_VERSION >= 10901)
#include <ruby/io.h>
#else
#include <rubyio.h>
#endif
#include "openexroutput.hh"

using namespace Imf;
using namespace std;

VALUE OpenEXROutput::cRubyClass = Qnil;

class ImfOStreamWrapper: public Imf::OStream
{
public:
  ImfOStreamWrapper( int fd, const char fileName[] ):
    OStream( fileName ), m_fd(fd) {}
  virtual void write( const char c[], int n );
  virtual Int64 tellp(void);
  virtual void seekp( Int64 pos );
protected:
  int m_fd;
};

void ImfOStreamWrapper::write( const char c[], int n )
{
  if ( n != ::write( m_fd, c, n ) ) {
    if ( errno )
      Iex::throwErrnoExc();
    else
      throw Iex::InputExc( "Error writing to file." );
  };
}

Int64 ImfOStreamWrapper::tellp(void)
{
  return lseek( m_fd, 0, SEEK_CUR );
}

void ImfOStreamWrapper::seekp( Int64 pos )
{
  lseek( m_fd, pos, SEEK_SET );
}
  
OpenEXROutput::OpenEXROutput( int fd ):
  m_fd(fd), m_status(true)
{
  assert( m_fd != 0 );
}

void OpenEXROutput::write( FramePtr frame ) throw (Error)
{
  try {
    ERRORMACRO( m_status, Error, , "OpenEXR-image was written already." );
    if ( frame->rgb() ) {
      Header header( frame->width(), frame->height() );
      header.channels().insert( "R", Channel(Imf::FLOAT) );
      header.channels().insert( "G", Channel(Imf::FLOAT) );
      header.channels().insert( "B", Channel(Imf::FLOAT) );
      ImfOStreamWrapper s( m_fd, "OpenEXR output file" );
      OutputFile f( s, header );
      FrameBuffer frameBuffer;
      frameBuffer.insert( "R",
                          Slice( Imf::FLOAT,
                                 (char *)frame->data(),
                                 sizeof(float) * 3,
                                 sizeof(float) * 3 * frame->width(),
                                 1, 1, 0.0 ) );
      frameBuffer.insert( "G",
                          Slice( Imf::FLOAT,
                                 (char *)frame->data() +
                                 sizeof(float),
                                 sizeof(float) * 3,
                                 sizeof(float) * 3 * frame->width(),
                                 1, 1, 0.0 ) );
      frameBuffer.insert( "B",
                          Slice( Imf::FLOAT,
                                 (char *)frame->data() +
                                 2 * sizeof(float),
                                 sizeof(float) * 3,
                                 sizeof(float) * 3 * frame->width(),
                                 1, 1, 0.0 ) );
      f.setFrameBuffer( frameBuffer );
      f.writePixels( frame->height() );
    } else {
      Header header( frame->width(), frame->height() );
      header.channels().insert( "Y", Channel(Imf::FLOAT) );
      ImfOStreamWrapper s( m_fd, "OpenEXR output file" );
      OutputFile f( s, header );
      FrameBuffer frameBuffer;
      frameBuffer.insert( "Y",
                          Slice( Imf::FLOAT,
                                 (char *)frame->data(),
                                 sizeof(float),
                                 sizeof(float) * frame->width(),
                                 1, 1, 0.0 ) );
      f.setFrameBuffer( frameBuffer );
      f.writePixels( frame->height() );
    };
    m_status = false;
  } catch ( Iex::BaseExc &e ) {
    ERRORMACRO( false, Error, ,e.what() );
  }
}

string OpenEXROutput::inspect(void) const
{
  return string( "OpenEXROutput-object" );
}

VALUE OpenEXROutput::registerRubyClass( VALUE module )
{
  cRubyClass = rb_define_class_under( module, "OpenEXROutput",
                                      rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 1 );
  rb_define_method( cRubyClass, "inspect",
                    RUBY_METHOD_FUNC( wrapInspect ), 0 );
  rb_define_method( cRubyClass, "write", RUBY_METHOD_FUNC( wrapWrite ), 1 );
  return cRubyClass;
}

void OpenEXROutput::deleteRubyObject( void *ptr )
{
  delete (OpenEXROutputPtr *)ptr;
}

VALUE OpenEXROutput::wrapNew( VALUE rbClass, VALUE rbFile )
{
  VALUE retVal = Qnil;
  try {
    rb_check_type( rbFile, T_FILE );
#if (RUBYVM_VERSION >= 10901)
    int fd = RFILE( rbFile )->fptr->fd;
#else
    FILE *f = RFILE( rbFile )->fptr->f;
    int fd = fileno( f );
#endif
    ERRORMACRO( fd != 0, Error, , "File descriptor is zero" );
    OpenEXROutputPtr ptr( new OpenEXROutput( fd ) );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new OpenEXROutputPtr( ptr ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE OpenEXROutput::wrapInspect( VALUE rbSelf )
{
  OpenEXROutputPtr *self; Data_Get_Struct( rbSelf, OpenEXROutputPtr, self );
  string retVal( (*self)->inspect() );
  return rb_str_new( retVal.c_str(), retVal.length() );
}

VALUE OpenEXROutput::wrapWrite( VALUE rbSelf, VALUE rbFrame )
{
  try {
    OpenEXROutputPtr *self; Data_Get_Struct( rbSelf, OpenEXROutputPtr, self );
    FramePtr frame( new Frame( rbFrame ) );
    (*self)->write( frame );
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbFrame;
}


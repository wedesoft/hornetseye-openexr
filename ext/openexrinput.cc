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
#include <OpenEXR/IexThrowErrnoExc.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfIO.h>
#include <cerrno>
#include "rubytools.hh"
#if (RUBYVM_VERSION >= 10901)
#include <ruby/io.h>
#else
#include <rubyio.h>
#endif
#include <unistd.h>
#include "openexrinput.hh"

using namespace boost;
using namespace Imf;
using namespace Imath;
using namespace std;

VALUE OpenEXRInput::cRubyClass = Qnil;

class ImfIStreamWrapper: public Imf::IStream
{
public:
  ImfIStreamWrapper( int fd, const char fileName[] ):
    IStream( fileName ), m_fd(fd) {}
  virtual bool read( char c[], int n );
  virtual Int64 tellg(void);
  virtual void seekg( Int64 pos );
  virtual void clear(void);
protected:
  int m_fd;
};

bool ImfIStreamWrapper::read( char c[], int n )
{
  if ( n != ::read( m_fd, c, n ) ) {
    if ( errno )
      Iex::throwErrnoExc();
    else
      throw Iex::InputExc( "Unexpected end of file." );
  }
  return true;
}

Int64 ImfIStreamWrapper::tellg(void)
{
  return lseek( m_fd, 0, SEEK_CUR );
}

void ImfIStreamWrapper::seekg( Int64 pos )
{
  lseek( m_fd, pos, SEEK_SET );
}

void ImfIStreamWrapper::clear(void)
{
  errno = 0;
}

OpenEXRInput::OpenEXRInput( int fd ):
  m_fd(fd), m_status(true)
{
  assert( m_fd != 0 );
}

FramePtr OpenEXRInput::read(void) throw (Error)
{
  FramePtr frame;
  try {
    ERRORMACRO( m_status, Error, , "OpenEXR-image was read already." );
    ImfIStreamWrapper fileWrapper( m_fd, "OpenEXR input file" );
    Int64 pos = fileWrapper.tellg();
    InputFile imfFile( fileWrapper );
    Box2i box( imfFile.header().dataWindow() );
    const ChannelList &channels = imfFile.header().channels();
    string typecode;
    if ( channels.findChannel( "Y" ) ) {
      if ( channels.findChannel( "RY" ) && channels.findChannel( "BY" ) )
        typecode = "YCA";
      else
        typecode = "SFLOAT";
    } else
      if ( channels.findChannel( "R" ) && channels.findChannel( "G" ) &&
           channels.findChannel( "B" ) )
        typecode = "SFLOATRGB";
      else {
        ERRORMACRO( false, Error, ,
                    "Unknown combination of channels in EXR file" );
      };
    int
      width  = box.max.x - box.min.x + 1,
      height = box.max.y - box.min.y + 1;
#ifndef NDEBUG
    cerr << "OpenEXRInput of " << width << 'x' << height << "-image" << endl;
#endif
    FrameBuffer frameBuffer;
    if ( typecode == "SFLOAT" ) {
      frame = FramePtr( new Frame( "SFLOAT", width, height ) );
      float *offset = (float *)frame->data() - box.min.x - box.min.y * width;
      frameBuffer.insert( "Y",
                          Slice( Imf::FLOAT,
                                 (char *)offset,
                                 sizeof(float),
                                 sizeof(float) * width, 1, 1, 0.0 ) );
      imfFile.setFrameBuffer( frameBuffer );
      imfFile.readPixels( box.min.y, box.max.y );
    } else if ( typecode == "SFLOATRGB" ) {
      frame = FramePtr( new Frame( "SFLOATRGB", width, height ) );
      float *offset = (float *)frame->data() -
                      3 * ( box.min.x + box.min.y * width );
      frameBuffer.insert( "R",
                          Slice( Imf::FLOAT,
                                 (char *)offset,
                                 3 * sizeof(float),
                                 3 * width * sizeof(float), 1, 1, 0.0 ) );
      frameBuffer.insert( "G",
                          Slice( Imf::FLOAT,
                                 (char *)( offset + 1 ),
                                 3 * sizeof(float),
                                 3 * width * sizeof(float), 1, 1, 0.0 ) );
      frameBuffer.insert( "B",
                          Slice( Imf::FLOAT,
                                 (char *)( offset + 2 ),
                                 3 * sizeof(float),
                                 3 * width * sizeof(float), 1, 1, 0.0 ) );
      imfFile.setFrameBuffer( frameBuffer );
      imfFile.readPixels( box.min.y, box.max.y );
    } else {
      assert( typecode == "YCA" );
      // This colourspace is hard to convert (see ImfRgbaFile.cpp) and
      // requires correlation with a filter. OpenEXR is used for doing this.
      fileWrapper.seekg( pos );
      RgbaInputFile imgRgbaFile( fileWrapper );
      int size = width * height;
      shared_array< Rgba > buffer( new Rgba[ size ] );
      imgRgbaFile.setFrameBuffer( buffer.get() -
                                  box.min.x - box.min.y * width, 1, width );
      imgRgbaFile.readPixels( box.min.y, box.max.y );
      frame = FramePtr( new Frame( "SFLOATRGB", width, height ) );
      const Rgba *p = buffer.get();
      float *q = (float *)frame->data();
      for ( int i=0; i<size; i++ ) {
        q[0] = p->r;
        q[1] = p->g;
        q[2] = p->b;
        p++;
        q += 3;
      };
    };
    m_status = false;
  } catch ( Iex::BaseExc &e ) {
    ERRORMACRO( false, Error, ,e.what() );
  };
  return frame;
}

string OpenEXRInput::inspect(void) const
{
  return string( "OpenEXRInput-object" );
}

VALUE OpenEXRInput::registerRubyClass( VALUE module )
{
  cRubyClass = rb_define_class_under( module, "OpenEXRInput", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 1 );
  rb_define_method( cRubyClass, "inspect",
                    RUBY_METHOD_FUNC( wrapInspect ), 0 );
  rb_define_method( cRubyClass, "read", RUBY_METHOD_FUNC( wrapRead ), 0 );
  rb_define_method( cRubyClass, "status?", RUBY_METHOD_FUNC( wrapStatus ), 0 );
  return cRubyClass;
}

void OpenEXRInput::deleteRubyObject( void *ptr )
{
  delete (OpenEXRInputPtr *)ptr;
}

VALUE OpenEXRInput::wrapNew( VALUE rbClass, VALUE rbFile )
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
    OpenEXRInputPtr ptr( new OpenEXRInput( fd ) );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new OpenEXRInputPtr( ptr ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE OpenEXRInput::wrapInspect( VALUE rbSelf )
{
  OpenEXRInputPtr *self; Data_Get_Struct( rbSelf, OpenEXRInputPtr, self );
  string retVal( (*self)->inspect() );
  return rb_str_new( retVal.c_str(), retVal.length() );
}

VALUE OpenEXRInput::wrapRead( VALUE rbSelf )
{
  VALUE retVal = Qnil;
  try {
    OpenEXRInputPtr *self; Data_Get_Struct( rbSelf, OpenEXRInputPtr, self );
    FramePtr frame( (*self)->read() );
    retVal = frame->rubyObject();
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE OpenEXRInput::wrapStatus( VALUE rbSelf )
{
  OpenEXRInputPtr *self; Data_Get_Struct( rbSelf, OpenEXRInputPtr, self );
  return (*self)->status() ? Qtrue : Qfalse;
}


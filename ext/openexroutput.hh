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
#ifndef HORNETSEYE_OPENEXROUTPUT_HH
#define HORNETSEYE_OPENEXROUTPUT_HH

#include "error.hh"
#include "frame.hh"

class OpenEXROutput
{
public:
  OpenEXROutput( int fd );
  virtual void write( FramePtr frame ) throw (Error);
  virtual bool status(void) const { return m_status; }
  std::string inspect(void) const;
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE module );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbFile );
  static VALUE wrapInspect( VALUE rbSelf );
  static VALUE wrapWrite( VALUE rbSelf, VALUE rbFrame );
protected:
  int m_fd;
  bool m_status;
};

typedef boost::shared_ptr< OpenEXROutput > OpenEXROutputPtr;

#endif

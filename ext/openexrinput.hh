/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007, 2008, 2009, 2010   Jan Wedekind
   
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
#ifndef HORNETSEYE_OPENEXRINPUT_HH
#define HORNETSEYE_OPENEXRINPUT_HH

#include "error.hh"
#include "frame.hh"

class OpenEXRInput
{
public:
  OpenEXRInput( int fd );
  virtual FramePtr read(void) throw (Error);
  virtual std::string inspect(void) const;
  virtual bool status(void) const { return m_status; }
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE module );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbFile );
  static VALUE wrapInspect( VALUE rbSelf );
  static VALUE wrapRead( VALUE rbSelf );
  static VALUE wrapStatus( VALUE rbSelf );
protected:
  int m_fd;
  bool m_status;
};

typedef boost::shared_ptr< OpenEXRInput > OpenEXRInputPtr;

#endif
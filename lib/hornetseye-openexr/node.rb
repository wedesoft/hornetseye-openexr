# hornetseye-rmagick - RMagick integration for Hornetseye
# Copyright (C) 2010 Jan Wedekind
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
module Hornetseye

  class Node

    def save_openexr( file )
      output = OpenEXROutput.new file
      output.write self
      output.close
      self
    end

    def save_sfloat( file )
      to_type( SFLOAT ).save_openexr file
    end

    def save_dfloat( file )
      to_type( DFLOAT ).save_openexr file
    end

    def save_sfloatrgb( file )
      to_type( SFLOATRGB ).save_openexr file
    end

    def save_dfloatrgb( file )
      to_type( DFLOATRGB ).save_openexr file
    end

  end

end


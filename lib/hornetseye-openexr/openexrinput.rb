# hornetseye-openexr - Loading and saving images using OpenEXR
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

  class OpenEXRInput

    class << self

      public

      alias_method :orig_new, :new

      def new( file )
        if file.is_a? File
          retval = orig_new file
        else
          file = File.new file, 'rb'
          retval = orig_new file
        end
        class << retval
          def file=( file )
            @file = file
          end
        end
        # Prevent garbage-collector from closing file.
        retval.file = file
        retval
      end

    end

    def close
      @file.close if @file
      @file = nil
    end

  end

end


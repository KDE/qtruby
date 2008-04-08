=begin
/***************************************************************************
                          plasma.rb  -  Plasma applet helper code
                             -------------------
    begin                : Sat April 5 2008
    copyright            : (C) 2008 by Richard Dale
    email                : richard.j.dale@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
=end

module Plasma
  class Applet
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Containment
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GLApplet
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class LineEdit
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class PackageMetadata
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class PackageStructure
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class SearchContext
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class SearchMatch
    def type(*args)
      method_missing(:type, *args)
    end
  end
end
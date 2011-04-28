=begin
     Copyright 2009-2011 by Richard Dale <richard.j.dale@gmail.com>

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU Library General Public License as
     published by the Free Software Foundation; either version 2, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details

     You should have received a copy of the GNU Library General Public
     License along with this program; if not, write to the
     Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
=end

module Qt
  class Application < Qt::Base
    def initialize(*args)
      if args.length == 1 && args[0].kind_of?(Array)
        super(args[0].length + 1, [$0] + args[0])
      else
        super(*args)
      end
      $qApp = self
    end

    # Delete the underlying C++ instance after exec returns
    # Otherwise, rb_gc_call_finalizer_at_exit() can delete
    # stuff that Qt::Application still needs for its cleanup.
    def exec
      method_missing(:exec)
      self.dispose
      Qt::Internal.application_terminated = true
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end
end

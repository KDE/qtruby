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

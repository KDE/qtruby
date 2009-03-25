=begin
/***************************************************************************
                          soprano.rb  -  Soprano SPARQL queries over DBus
                             -------------------
    begin                : Fri March 14 2008
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

module Soprano
  module Internal
    def self.init_all_classes
#      Qt::Internal::add_normalize_proc(Proc.new do |classname|
#        if classname =~ /^Soprano/
#          now = classname.sub(/^Soprano?(?=[A-Z])/,'Soprano::')
#        end
#        now
#      end)
      getClassList.each do |c|
        classname = Qt::Internal::normalize_classname(c)
        id = Qt::Internal::findClass(c);
        Qt::Internal::insert_pclassid(classname, id)
        Qt::Internal::cpp_names[classname] = c
        klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Soprano) \
                                           : Qt::Internal::create_qt_class(classname, Soprano)
        Qt::Internal::classes[classname] = klass unless klass.nil?
      end
    end
  end

  class Node < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end

    def inspect
      str = super
      case type.to_i
      when Soprano::Node::EmptyNode:
        str.sub(/>$/, " %s>" % "(empty)")
      when Soprano::Node::ResourceNode:
        str.sub(/>$/, " <%s>>" % uri.toString)
      when Soprano::Node::LiteralNode:
        if literal.isString && language
          str.sub(/>$/, ' "%s"@%s>' % [literal.toString, language])
        else 
          str.sub(/>$/, ' "%s"^^<%s>>' % [literal.toString, literal.dataTypeUri.toString])
        end
      when Soprano::Node::BlankNode:
        str.sub(/>$/, " _:%s>" % identifier)
      end
    end
		
    def pretty_print(pp)
      str = to_s
      case type.to_i
      when Soprano::Node::EmptyNode:
        pp.text str.sub(/>$/, " %s>" % "(empty)")
      when Soprano::Node::ResourceNode:
        pp.text str.sub(/>$/, " <%s>>" % uri.toString)
      when Soprano::Node::LiteralNode:
        if literal.isString && language
          pp.text str.sub(/>$/, ' "%s"@%s>' % [literal.toString, language])
        else 
          pp.text str.sub(/>$/, ' "%s"^^<%s>>' % [literal.toString, literal.dataTypeUri.toString])
        end
      when Soprano::Node::BlankNode:
        pp.text str.sub(/>$/, " _:%s>" % identifier)
      end
    end

    def self.demarshall(arg)
      arg.beginStructure
      type = Qt::Integer.new
      value = ""
      language = ""
      dataTypeUri = ""
      arg >> type >> value >> language >> dataTypeUri

      case type.to_i
      when Soprano::Node::EmptyNode:
        node = Soprano::Node.new
      when Soprano::Node::ResourceNode:
        node = Soprano::Node.new(Qt::Url.new(value))
      when Soprano::Node::LiteralNode:
        node = Soprano::Node.new(Soprano::LiteralValue.fromString(value, Qt::Url.new(dataTypeUri)), language)
      when Soprano::Node::BlankNode:
        node = Soprano::Node.new(value)
      else
        node = Soprano::Node.new
      end
      arg.endStructure
      return node
    end
  end

  class Statement < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " valid?=%s, subject=%s, predicate=%s, object=%s, context=%s>" % 
        [isValid, subject.inspect, predicate.inspect, object.inspect, context.inspect])
    end
    
    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " valid?=%s,\n subject=%s,\n predicate=%s,\n object=%s,\n context=%s>" % 
        [isValid, subject.inspect, predicate.inspect, object.inspect, context.inspect])
    end

    def self.demarshall(arg)
      arg.beginStructure
      subject = Soprano::Node.demarshall(arg)
      predicate = Soprano::Node.demarshall(arg)
      object = Soprano::Node.demarshall(arg)
      context = Soprano::Node.demarshall(arg)
      statement = Soprano::Statement.new(subject, predicate, object, context)
      arg.endStructure
      return statement
    end
  end
  
  class BindingSet < Qt::Base
    def self.demarshall(arg)
      set = {}
      arg.beginStructure
      arg.beginMap
      while !arg.atEnd
        arg.beginMapEntry
        name = ""
        arg >> name
        node = Soprano::Node.demarshall(arg)
        arg.endMapEntry
        set[name.to_sym] = node
      end
      arg.endMap
      arg.endStructure
      return set
    end

    def [](v)
      value(v.to_s)
    end
  end

  class NodeIterator < Qt::Base
    include Enumerable

    def each
      allNodes.each do |node|
        yield node
      end
    end
  end

  class StatementIterator < Qt::Base
    include Enumerable

    def each
      allStatements.each do |statement|
        yield statement
      end
    end
  end

  class QueryResultIterator < Qt::Base
    include Enumerable

    def each
      allBindings.each do |bindingSet|
        yield bindingSet
      end
    end
  end

  class StorageModel < Qt::Base
    # executeQuery() is a pure virtual method with an optional 
    # arg. This isn't supported by the smoke code generation
    # yet, so cater for the optional arg here for now
    def executeQuery(*args)
      if args.length == 2
        super(args[0], args[1], nil)
      else
        super
      end
    end
  end

  class Client < Qt::Base
    class DBusNodeIterator < Qt::Base
      include Enumerable

      def each
        allNodes.each do |node|
          yield node
        end
      end
    end

    class DBusStatementIterator < Qt::Base
      include Enumerable

      def each
        allStatements.each do |statement|
          yield statement
        end
      end
    end

    class DBusQueryResultIterator < Qt::Base
      include Enumerable

      def each
        allBindings.each do |bindingSet|
          yield bindingSet
        end
      end
    end

  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;

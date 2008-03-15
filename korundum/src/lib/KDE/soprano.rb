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
  class Node
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
        if literal.isString && !language.empty?
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
        if literal.isString && !language.empty?
          pp.text str.sub(/>$/, ' "%s"@%s>' % [literal.toString, language])
        else 
          pp.text str.sub(/>$/, ' "%s"^^<%s>>' % [literal.toString, literal.dataTypeUri.toString])
        end
      when Soprano::Node::BlankNode:
        pp.text str.sub(/>$/, " _:%s>" % identifier)
      end
    end

    def self.unmarshall(arg)
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

  class Statement
    def self.unmarshall(arg)
      arg.beginStructure
        subject = Soprano::Node.unmarshall(arg)
        predicate = Soprano::Node.unmarshall(arg)
        object = Soprano::Node.unmarshall(arg)
        context = Soprano::Node.unmarshall(arg)
        statement = Soprano::Statement.new(subject, predicate, object, context)
      arg.endStructure
      return statement
    end
  end
  
  class BindingSet
    def self.unmarshall(arg)
      set = {}
      arg.beginStructure
        arg.beginMap
        while !arg.atEnd
          arg.beginMapEntry
            name = ""
            arg >> name
            val = Soprano::Node.unmarshall(arg)
          arg.endMapEntry
          set[name.to_sym] = val
        end
        arg.endMap
      arg.endStructure
      return set
    end
  end

  module Query
    QueryLanguageNone = 0x0     # No query language */
    QueryLanguageSparql = 0x1   # The SPARQL query language: http://www.w3.org/TR/rdf-sparql-query/ */
    QueryLanguageRdql = 0x2     # The RDQL RDF query language: http://www.w3.org/Submission/2004/SUBM-RDQL-20040109/ */
    QueryLanguageSerql = 0x4    # Sesame RDF %Query Language: http://openrdf.org/doc/sesame2/users/ch05.html */
    QueryLanguageUser = 0x1000  # The user type can be used to introduce unknown query lanaguages by name */

    def Query.queryLanguageToString(lang, userQueryLanguage )
      case lang 
      when QueryLanguageNone:
        return "none"
      when QueryLanguageSparql:
        return "SPARQL"
      when QueryLanguageRdql:
        return "RDQL"
      when QueryLanguageSerql:
        return "SERQL"
      else
        return userQueryLanguage
      end
    end

  end

  module Client
    class DBusNodeIterator
      include Enumerable

      def initialize(serviceName, dbusObject)
        @interface = Qt::DBusInterface.new("org.soprano.Server", dbusObject, "org.soprano.NodeIterator")
      end

      def each
        while @interface.next
          reply = @interface.current
          node = Soprano::Node.unmarshall(reply)
          yield node
        end
      end

      def close
        @interface.close
      end
    end

    class DBusStatementIterator
      include Enumerable

      def initialize(serviceName, dbusObject)
        @interface = Qt::DBusInterface.new("org.soprano.Server", dbusObject, "org.soprano.StatementIterator")
      end

      def each
        while @interface.next
          reply = @interface.current
          statement = Soprano::Statement.unmarshall(reply)
          yield statement
        end
      end

      def close
        @interface.close
      end
    end

    class DBusQueryResultIterator
      include Enumerable

      def initialize(serviceName, dbusObject)
        @interface = Qt::DBusInterface.new("org.soprano.Server", dbusObject, "org.soprano.QueryResultIterator")
      end

      def each
        while @interface.next
          reply = @interface.current
          set = Soprano::BindingSet.unmarshall(reply)
          yield set
        end
      end

      def next
        @interface.next
      end

      def current
        reply = @interface.current
        set = Soprano::BindingSet.unmarshall(reply)
        return set
      end

      def currentStatement
        reply = @interface.currentStatement
        statement = Soprano::Statement.unmarshall(reply)
        return statement
      end

      def bindingByName(name)
        reply = @interface.bindingByName(name.to_s)
        node = Soprano::Node.unmarshall(reply)
        return node
      end

      def bindingByIndex(index)
        reply = @interface.bindingByIndex(index)
        node = Soprano::Node.unmarshall(reply)
        return node
      end

      def bindingCount
        @interface.bindingCount
      end

      def bindingNames
        @interface.bindingNames
      end

      def boolValue
        @interface.boolValue
      end

      def bool?
        @interface.isBool
      end

      def binding?
        @interface.isBinding
      end

      def graph?
        @interface.isGraph
      end

      def close
        @interface.close
      end
    end

    class DBusModel < Qt::Object
      signals :statementsAdded, :statementsRemoved,
              'statementAdded(Soprano::Statement)',
              'statementRemoved(Soprano::Statement)'

      def initialize(serviceName, dbusObject)
        super()
        @interface = Qt::DBusInterface.new( serviceName,
                                            dbusObject,
                                            "org.soprano.Model",
                                            Qt::DBusConnection.sessionBus(),
                                            self )

        connect( @interface, SIGNAL(:statementsAdded),
                 self, SIGNAL(:statementsAdded) )
        connect( @interface, SIGNAL(:statementsRemoved),
                 self, SIGNAL(:statementsRemoved) )
=begin
        connect( @interface, SIGNAL('statementAdded(Soprano::Statement)'),
                 self, SIGNAL('statementAdded(Soprano::Statement)') )
        connect( @interface, SIGNAL('statementRemoved(Soprano::Statement)'),
                 self, SIGNAL('statementRemoved(Soprano::Statement)') )
=end
      end

      def listStatements(partial)
        reply = @interface.call("listStatements", partial)
        if reply.type == Qt::DBusMessage::ReplyMessage
          return DBusStatementIterator.new(@interface.service, reply.arguments[0].value)
        else
          return nil
        end
      end

      def listContexts
        reply = @interface.call("listContexts")
        if reply.type == Qt::DBusMessage::ReplyMessage
          return DBusNodeIterator.new(@interface.service, reply.arguments[0].value)
        else
          return nil
        end
      end

      def executeQuery(query, language, userQueryLanguage = "")
        reply = @interface.call("executeQuery", query, Soprano::Query.queryLanguageToString(language, userQueryLanguage))
        if reply.type == Qt::DBusMessage::ReplyMessage
          return DBusQueryResultIterator.new(@interface.service, reply.arguments[0].value)
        else
          return nil
        end
      end

      def addStatement(statement)
        reply = @interface.call("addStatement", statement)
        if reply.type == Qt::DBusMessage::ReplyMessage
          return reply.arguments[0].value
        else
          return nil
        end
      end

      def removeStatement(statement)
        reply = @interface.call("removeStatement", statement)
        if reply.type == Qt::DBusMessage::ReplyMessage
          return reply.arguments[0].value
        else
          return nil
        end
      end

      def removeAllStatements(statement)
        reply = @interface.call("removeAllStatements", statement)
        if reply.type == Qt::DBusMessage::ReplyMessage
          return reply.arguments[0].value
        else
          return nil
        end
      end

      def statementCount()
        reply = @interface.call("statementCount")
        if reply.type == Qt::DBusMessage::ReplyMessage
          return reply.arguments[0].value
        else
          return nil
        end
      end

      def isEmpty()
        return @interface.isEmpty
      end

      def empty?
        return @interface.isEmpty
      end

      def containsStatement(statement)
        reply = @interface.call("containsStatement", statement)
        if reply.type == Qt::DBusMessage::ReplyMessage
          return reply.arguments[0].value
        else
          return nil
        end
      end

      def containsAnyStatement(statement)
        reply = @interface.call("containsAnyStatement", statement)
        if reply.type == Qt::DBusMessage::ReplyMessage
          return reply.arguments[0].value
        else
          return nil
        end
      end

      def createBlankNode()
        reply = @interface.createBlankNode
        return Soprano::Node.unmarshall(reply)
      end
    end

    class DBusClient < Qt::Object
      def initialize(serviceName = "", parent = nil)
        super(parent)
        @interface = Qt::DBusInterface.new( serviceName.empty? ? "org.soprano.Server" : serviceName,
                                            "/org/soprano/Server",
                                            "org.soprano.Server",
                                            Qt::DBusConnection.sessionBus(),
                                            self )
      end

      def isValid
        return @interface.isValid
      end

      def valid?
        isValid
      end

      def allModels
        return @interface.allModels
      end

      def createModel(name)
        reply = @interface.call("createModel", name)
        return DBusModel.new(@interface.service, reply.value)
      end

      def removeModel(name)
        return @interface.removeModel(name)
      end
    end
  end
end

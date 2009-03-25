require 'active_rdf'
require 'queryengine/query2sparql'
require 'korundum4'

# Soprano DBus server adapter
class SopranoAdapter < ActiveRdfAdapter
  $activerdflog.info "loading Soprano adapter"
  ConnectionPool.register_adapter(:soprano, self)

  attr_reader :engine
  attr_reader :caching

  @@soprano_cache = {}

  def SopranoAdapter.get_cache
    return @@soprano_cache
  end
  
  # Instantiate the connection with the SPARQL Endpoint.
  # available parameters:
  # * :model => name of model to use, defaults to 'main'
  # * :service => DBus service to use, defaults to 'org.soprano.Server'
  # * :backend => the name of a backend, such as 'virtuoso'
  def initialize(params = {})
    super()  
    @reads = true
    @writes = true

    @model_name = params[:model] || 'main'
    @caching = params[:caching] || false
    @backend_name = params[:backend]

    if @backend_name
      @backend = Soprano.discoverBackendByName(@backend_name)
      settings = []
      if @backend_name =~ /^virtuoso/
        @host = params[:host] || 'localhost'
        @port = params[:port] || 1111
        @username = params[:username] || 'dba'
        @password = params[:password] || 'dba'
        settings << Soprano::BackendSetting.new(Soprano::BackendOptionHost, @host)
        settings << Soprano::BackendSetting.new(Soprano::BackendOptionPort, @port)
        settings << Soprano::BackendSetting.new(Soprano::BackendOptionUsername, @username)
        settings << Soprano::BackendSetting.new(Soprano::BackendOptionPassword, @password)
      end
      @model = @backend.createModel(settings)
    else
      # For accessing the Nepomuk store in KDE, use 'org.kde.NepomukServer'
      @service = params[:service] || 'org.soprano.Server'

      @client = Soprano::Client::DBusClient.new(@service)
      @model = @client.createModel(@model_name)
    end
  end

  def size
    @model.statementCount
  end

  def clear
    @model.removeStatement(Soprano::Node.new, Soprano::Node.new, Soprano::Node.new, Soprano::Node.new)
  end

  # load a file from the given location with the given syntax into the model.
  def load(location, syntax="n-triples")
    if @backend_name
      system("sopranocmd --backend #{@backend_name} --host #{@host} --port #{@port} --username #{@username} --password #{@password} --serialization #{syntax} import #{location}")
    else
      system("sopranocmd --dbus #{@service} --serialization #{syntax} --model #{@model_name} import #{location}")
    end
  end

  # query datastore with query string (SPARQL), returns array with query results
  # may be called with a block
  def query(query, &block)
    qs = Query2SPARQL.translate(query)

    if @caching
       result = query_cache(qs)
       if result.nil?
         $activerdflog.debug "cache miss for query #{qs}"
       else
         $activerdflog.debug "cache hit for query #{qs}"
         return result
       end
    end

    result = execute_soprano_query(qs, query.select_clauses, &block)
    add_to_cache(qs, result) if @caching
    result = [] if result == "timeout"
    return result
  end
    
  # do the real work of executing the sparql query
  def execute_soprano_query(qs, select_clauses, &block)
    results = []

    # querying soprano server
    binding_set = @model.executeQuery(qs, Soprano::Query::QueryLanguageSparql)

    binding_set.each do |binding|
      result = []
      select_clauses.each do |var|
        if binding[var]
          result << soprano_node_to_activerdf(binding[var])
        end
      end
      results << result
    end

    if block_given?
      results.each do |*clauses|
        yield(*clauses)
      end
    else
      results
    end
  end

  def flush
    true
  end  

  def save
    true
  end

  def close
    ConnectionPool.remove_data_source(self)
  end
  
  # add triple to datamodel
  def add(s, p, o, c=nil)
    $activerdflog.debug "adding triple #{s} #{p} #{o} #{c}"

    # verify input
    if s.nil? || p.nil? || o.nil?
      $activerdflog.debug "cannot add triple with empty subject, exiting"
      return false
    end 
    
    unless s.respond_to?(:uri) && p.respond_to?(:uri)
      $activerdflog.debug "cannot add triple where s/p are not resources, exiting"
      return false
    end
  
    @model.addStatement(Soprano::Statement.new(wrap(s), wrap(p), wrap(o), wrap(c)))
    save if ConnectionPool.auto_flush?
  end

  # deletes triple(s,p,o) from datastore
  # nil parameters match anything: delete(nil,nil,nil) will delete all triples
  # ActiveRDF will pass the symbol :all as 'o' when all values of
  # object for the subject/predicate should be deleted
  def delete(s, p, o, c=nil)
    $activerdflog.debug "removing triple(s) #{s} #{p} #{o} #{c}"

    o = nil if o == :all
    @model.removeAllStatements(Soprano::Statement.new(wrap(s), wrap(p), wrap(o), wrap(c)))
  end

  private
  def add_to_cache(query_string, result)
    unless result.nil? or result.empty?
      if result == "timeout"
        @@soprano_cache.store(query_string, [])
      else 
        $activerdflog.debug "adding to soprano cache - query: #{query_string}"
        @@soprano_cache.store(query_string, result) 
      end
    end
  end
  
  def query_cache(query_string)
    if @@soprano_cache.include?(query_string)
      return @@soprano_cache.fetch(query_string)
    else
      return nil
    end
  end

  # Converts a Soprano::Node to the ActiveRDF equivalent
  def soprano_node_to_activerdf(node)
      case node.type.to_i
      when Soprano::Node::EmptyNode:
        "(empty)"
      when Soprano::Node::ResourceNode:
        RDFS::Resource.new(node.uri.toString)
      when Soprano::Node::LiteralNode:
        node.literal.variant.value
      when Soprano::Node::BlankNode:
        nil
      end
  end

  def wrap(node)
    case node
    when nil
      Soprano::Node.new
    when RDFS::Resource
      Soprano::Node.new(Qt::Url.new(node.uri))
    when String
      if node =~ /^_:(.*)/
        Soprano::Node.new($1)
      elsif node =~ /(.*)@(.*)/
        Soprano::Node.new(Soprano::LiteralValue.new($1), $2)
      else
        Soprano::Node.new(Soprano::LiteralValue.new(node))
      end
    else
      Soprano::Node.new(Soprano::LiteralValue.new(node))
    end
  end

end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;

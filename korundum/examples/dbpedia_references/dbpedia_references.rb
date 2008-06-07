$KCODE = 'u'

require 'korundum4'
require 'soprano'
require 'qtwebkit'
require 'cgi'
require 'rexml/document'

# Parser for SPARQL XML result set. Derived from the parser in the
# ActiveRDF SPARQL adapter code. Produces an Array of Hashes, each
# hash contains keys for each of the variables in the query, and
# values which are Soprano nodes.
#
class SparqlResultParser
  attr_reader :result

  def initialize
    @result = []
    @vars = []
    @current_type = nil
  end
  
  def tag_start(name, attrs)
    case name
    when 'variable'
      @vars << attrs['name']
    when 'result'
      @current_result = {}
    when 'binding'
      @current_binding = attrs['name']
    when 'bnode', 'uri'
      @current_type = name
    when 'literal', 'typed-literal'
      @current_type = name
      @datatype = attrs['datatype']
      @xmllang = attrs['xml:lang']
    end
  end
  
  def tag_end(name)
    if name == "result"
      @result << @current_result
    elsif name == 'bnode' || name == 'literal' || name == 'typed-literal' || name == 'uri'
      @current_type = nil
    elsif name == "sparql"
    end
  end
  
  def text(text)
    if !@current_type.nil?
      @current_result[@current_binding] = create_node(@current_type, @datatype, @xmllang, text)
    end
  end

  # create ruby objects for each RDF node
  def create_node(type, datatype, xmllang, value)
    case type
    when 'uri'
      Soprano::Node.new(Qt::Url.new(value))
    when 'bnode'
      Soprano::Node.new(value)
    when 'literal', 'typed-literal'
      if xmllang
        Soprano::Node.new(Soprano::LiteralValue.new(value), xmllang)
      elsif datatype
        Soprano::Node.new(Soprano::LiteralValue.fromString(value, Qt::Url.new(datatype)))
      else
        Soprano::Node.new(Soprano::LiteralValue.new(value))
      end
    end
  end
  
  def method_missing (*args)
  end
end

SPARQL_REFERENCES_QUERY = <<-EOS
PREFIX p: <http://dbpedia.org/property/>  
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT DISTINCT ?reference ?label WHERE { 
    <http://dbpedia.org/resource/%s> <http://dbpedia.org/property/reference> ?reference .
    OPTIONAL {?reference <http://www.w3.org/2000/01/rdf-schema#label> ?label}
}
EOS

SPARQL_ABSTRACT_QUERY = <<-EOS
PREFIX p: <http://dbpedia.org/property/>  
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT DISTINCT ?abstract WHERE {
    OPTIONAL { <http://dbpedia.org/resource/%s> <http://dbpedia.org/property/abstract> ?abstract }
    OPTIONAL { <http://dbpedia.org/resource/%s> <http://dbpedia.org/abstract> ?abstract }
    FILTER( lang(?abstract) = "en" )
}
EOS

class DBPediaReferences < KDE::MainWindow
  slots 'abstractQueryCompleted(KJob*)',
        'queryData(KIO::Job*, QByteArray)',
        'referencesQueryCompleted(KJob*)',
        :queryStart

  def initialize(parent = nil)
    super(parent)
    @endpoint = 'http://dbpedia.org/sparql'

    widget = Qt::Widget.new(self)
    self.centralWidget = widget

    title = Qt::Label.new("Search DBpedia for references to a resource") do |t|
      t.alignment = Qt::AlignCenter
    end
    
    @edit = KDE::LineEdit.new
    
    @webview1 = Qt::WebView.new
    @webview1.page.linkDelegationPolicy = Qt::WebPage::DelegateAllLinks
    @webview1.html = ""

    @webview2 = Qt::WebView.new
    @webview2.html = ""

    splitter = Qt::Splitter.new do |s|
      s.orientation = Qt::Vertical
      s.addWidget(@webview1)
      s.addWidget(@webview2)
    end
    
    widget.layout = Qt::VBoxLayout.new do |l|
      l.addWidget(title)
      Qt::HBoxLayout.new do |h|
        l.insertLayout 1, h
        h.addWidget(Qt::Label.new("Resource name"))
        h.addWidget(@edit)
      end
      l.addWidget(splitter)
    end

    connect(@edit, SIGNAL(:returnPressed), self, SLOT(:queryStart))

    @webview1.connect SIGNAL("linkClicked(QUrl)") do |url| 
      @webview2.load(url)
    end
  end

  def queryStart
    # puts "sourceRequested(#{source_name})"
    if @job
      return false
    end

    @webview2.html = ""
    @source_name = @edit.text
    @sparql_results_xml = ""
    @html = ""
    query_url = KDE::Url.new("#{@endpoint}?query=#{CGI.escape(SPARQL_ABSTRACT_QUERY % 
                            [@source_name.gsub(' ', '_'), @source_name.gsub(' ', '_')])}")
    @job = KIO::get(query_url, KIO::Reload, KIO::HideProgressInfo)
    @job.addMetaData("accept", "application/sparql-results+xml" )
    connect(@job, SIGNAL('data(KIO::Job*, QByteArray)'), self,
            SLOT('queryData(KIO::Job*, QByteArray)'))
    connect(@job, SIGNAL('result(KJob*)'), self, SLOT('abstractQueryCompleted(KJob*)'))
    return true
  end

  def queryData(job, data)
    # puts "queryData(#{job})"
    @sparql_results_xml += data.to_s
  end

  def abstractQueryCompleted(job)
    @job.doKill
    @job = nil
    @html += "<meta http-equiv='charset' content='UTF-8'><h1>#{@edit.text}</h1>"
    parser = SparqlResultParser.new
    REXML::Document.parse_stream(@sparql_results_xml, parser)
    index = 0
    parser.result.each do |binding|
      @html += "<p>#{binding['abstract'].literal.variant.toString}</p><br />"
    end

    @sparql_results_xml = ""
    query_url = KDE::Url.new("#{@endpoint}?query=#{CGI.escape(SPARQL_REFERENCES_QUERY % @source_name.gsub(' ', '_'))}")
    @job = KIO::get(query_url, KIO::Reload, KIO::HideProgressInfo)
    @job.addMetaData("accept", "application/sparql-results+xml" )
    connect(@job, SIGNAL('data(KIO::Job*, QByteArray)'), self,
            SLOT('queryData(KIO::Job*, QByteArray)'))
    connect(@job, SIGNAL('result(KJob*)'), self, SLOT('referencesQueryCompleted(KJob*)'))
  end

  def referencesQueryCompleted(job)
    @job.doKill
    @job = nil
    parser = SparqlResultParser.new
    REXML::Document.parse_stream(@sparql_results_xml, parser)
    index = 0
    parser.result.each do |binding|
      if binding['label']
        @html += "<a href='#{binding['reference'].uri.toString}'>[#{index + 1}]</a>  #{binding['label'].literal.variant.value}<br />"
      else
        @html += "<a href='#{binding['reference'].uri.toString}'>[#{index + 1}]  #{binding['reference'].uri.toString}</a><br />"
      end
      index += 1
    end
    @webview1.html = @html
  end
end

about = KDE::AboutData.new("dbpedia_references", "DBpedia demo", KDE.ki18n(""), "")
KDE::CmdLineArgs.init(ARGV, about)
KDE::Application.new

kmainwindow = DBPediaReferences.new

kmainwindow.resize(700, 600)
kmainwindow.show
$kapp.exec

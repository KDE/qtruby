#!/usr/bin/env ruby

require 'Korundum'

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["RubberDoc"], about)
app = KDE::Application.new()

# Qt.debug_level = Qt::DebugLevel::High
# Qt.debug_level = Qt::DebugLevel::Extensive

DEBUG        = false
DEBUG_IDX    = false
DEBUG_FAST   = false
DEBUG_SEARCH = false
DEBUG_GOTO   = false # crashes?

def time_me str
   t1 = Time.now
   yield
   t2 = Time.now
   log "#{str}: #{"%.02f" % (t2 - t1).to_f}s"
end

class String
   def trigrams
      list = []
      0.upto(self.length-3) {
         |p|
         list << self.slice(p, 3)
      }
      list
   end
end

module DOMUtils

   def DOMUtils.each_child node
      indent = 0
      until node.isNull 
         yield node
         if not node.firstChild.isNull
            node = node.firstChild
            indent += 1
         elsif not node.nextSibling.isNull
            node = node.nextSibling
         else
            while indent > 0 and !node.isNull and node.nextSibling.isNull
               node = node.parentNode
               indent -= 1
            end
            if not node.isNull
               node = node.nextSibling
            end
         end
         break if indent == 0
      end
   end

   def DOMUtils.find_node doc, path_a
      n = doc
      path_a.reverse.each {
         |index|
         top = n.childNodes.length
         n = n.childNodes.item (top - index)
      }
      n
   end

   def DOMUtils.list_parent_node_types node
      n = node
      types_a = []
      until n.isNull
         types_a << { :nodeType => n.nodeType, :elementId => n.elementId }
         n = n.parentNode
      end
      types_a 
   end

   def DOMUtils.get_node_path node
      n = node
      path_a = []
      until n.isNull
         top = n.parentNode.childNodes.length
         idx = n.index
         path_a << (top-idx) if (n.elementId != 0)
         n = n.parentNode
      end
      path_a 
   end

end

class GenericTriGramIndex
   attr_accessor :trigrams

   def initialize
      clear
   end

   def clear
      @trigrams = {}
   end

   def insert_with_key string, key
      string.downcase.trigrams.each {
         |trigram| 
         @trigrams[trigram] = [] unless @trigrams.has_key? trigram
         @trigrams[trigram] << key
      }
   end

   # returns a list of matching keys
   def search search_string
      return [] if search_string.length < 3
      trigs = search_string.downcase.trigrams
      key_subset = @trigrams[trigs.delete_at(0)]
      return [] if key_subset.nil?
      trigs.each {
         |trigram|
         trigram_subset = @trigrams[trigram]
         return [] if trigram_subset.nil?
         key_subset &= trigram_subset 
      }
      key_subset
   end
end

module LoggedDebug

   def init_logger parent
      @logger = Qt::TextEdit.new parent
      @logger.setTextFormat Qt::LogText
   end

   def log s
      @logger.append s
      scrolldown_logger
   end

   def scrolldown_logger
      @logger.scrollToBottom
   end

end

module MyGui

   def init_gui
      buttons = Qt::HBox.new self
      @panes = Qt::Splitter.new self
      @panes.setOrientation Qt::Splitter::Horizontal
      setStretchFactor @panes, 10

      @pane_blah = Qt::VBox.new @panes

      @rightpane = Qt::Splitter.new @panes
      @rightpane.setOrientation Qt::Splitter::Vertical
      @viewed = KDE::HTMLPart.new @rightpane
      init_logger @rightpane

      @listview = Qt::ListView.new @pane_blah
      @listview.addColumn "IDXs"
      @listview.hideColumn 1 unless DEBUG_IDX
      @listview.addColumn "URIs"
      @listview.setResizeMode Qt::ListView::LastColumn
      @listview.setRootIsDecorated true

      @label    = Qt::Label.new self

      Qt::Object.connect @listview, SIGNAL("clicked(QListViewItem*)"),
                         self,      SLOT("clicked_result(QListViewItem*)")
      Qt::Object.connect @viewed,   SIGNAL("completed()"),
                         self,      SLOT("khtml_part_init_complete()")

      Qt::Object::connect @viewed,  SIGNAL("setWindowCaption(const QString&)"),
                          @viewed.widget.topLevelWidget, 
                                    SLOT("setCaption(const QString&)")

      Qt::Object::connect @viewed.browserExtension, 
                                    SIGNAL("openURLRequest(const KURL&, const KParts::URLArgs&)"),
                          self,     SLOT("openURL(const KURL&)")

      KDE::Action.new "&Quit",        "quit",    KDE::Shortcut.new(), self, SLOT("quit()"),                     @main.actionCollection, "file_quit"
      KDE::Action.new "&Index-All",              KDE::Shortcut.new(), self, SLOT("index_all()"),                @main.actionCollection, "index_all"
      @back = \
      KDE::Action.new "&Back",        "back",    KDE::Shortcut.new(), self, SLOT("go_back()"),                  @main.actionCollection, "back"
      @forward = \
      KDE::Action.new "&Forward",     "forward", KDE::Shortcut.new(), self, SLOT("go_forward()"),               @main.actionCollection, "forward"
      KDE::Action.new "&Home",        "gohome",  KDE::Shortcut.new(), self, SLOT("go_home()"),                  @main.actionCollection, "home"
      KDE::Action.new "&Next Match",  "next",    KDE::Shortcut.new(), self, SLOT("goto_next_match()"),          @main.actionCollection, "next_match"

      clearLocation = KDE::Action.new "Clear Location Bar", "locationbar_erase", KDE::Shortcut.new(), self, SLOT("clear_location()"), @main.actionCollection, "clear_location"
      clearLocation.setWhatsThis "Clear Location bar<p>Clears the content of the location bar."

      @searchlabel = Qt::Label.new @main
      @searchlabel.setText "Search: "

      @searchcombo = KDE::HistoryCombo.new @main
      Qt::Object.connect @searchcombo,  SIGNAL("returnPressed()"),
                         self,          SLOT("goto_search()")
      Qt::Object.connect @searchcombo,  SIGNAL("textChanged(const QString&)"),
                         self,          SLOT("search(const QString&)")

      KDE::WidgetAction.new @searchlabel, "Search: ", KDE::Shortcut.new(Qt::Key_F6), nil, nil, @main.actionCollection, "location_label"
      @searchlabel.setBuddy @searchcombo

      ca = KDE::WidgetAction.new @searchcombo, "Search", KDE::Shortcut.new, nil, nil, @main.actionCollection, "toolbar_url_combo"
      ca.setAutoSized true
      Qt::WhatsThis::add @searchcombo, "Search<p>Enter a search term."
   end

   def clear_location
      @searchcombo.clearEdit
   end

   def openURL kurl
      kurl.url =~ /(.*?)(#(.*))?$/
      url, anchor = $1, $3
      id = @id2uri.invert[url]
      if id.nil?
         log "link points outside indexed space!" 
         return
      end
      goto_id_and_hl id unless id == @shown_doc_id
      @viewed.gotoAnchor anchor unless anchor.nil?
   end

   def gui_init_proportions
      # todo - save these settings
      desktop = Qt::Application::desktop
      sx = (desktop.width  * (2.0/3.0)).to_i
      sy = (desktop.height * (2.0/3.0)).to_i

      @main.resize sx, sy

      logsize     = 0
      resultssize = (sx /  5.0).to_i

      @rightpane.setSizes [sy-logsize, logsize]
      @panes.setSizes     [resultssize, sx-resultssize]

      @rightpane.setResizeMode @logger, Qt::Splitter::KeepSize

      @panes.setResizeMode @pane_blah,  Qt::Splitter::KeepSize
      @panes.setResizeMode @rightpane,  Qt::Splitter::KeepSize
   end

end

module IndexStorage

   def dotfilepref
      basedir = ENV["HOME"] + "/.rubberdocs"
      prefix = basedir + "/." + ENV["BASEDOCURL"].gsub(/\//,",") + ".idx"
      Dir.mkdir basedir unless File.exists? basedir
      prefix
   end

   def index_fname ;     "#{dotfilepref}.doc";    end
   def nodeindex_fname ; "#{dotfilepref}.nodes";  end
   def titles_fname;     "#{dotfilepref}.titles"; end
   def uris_fname;       "#{dotfilepref}.uris";   end
   def preloaded_fname;  "#{dotfilepref}.pfxed";  end

   def load_indexes
      return false unless File.exists? index_fname and File.exists? nodeindex_fname
      File.open(index_fname,     "r") { |file| @index     = Marshal.load file }
      File.open(nodeindex_fname, "r") { |file| @nodeindex = Marshal.load file }
      File.open(titles_fname,    "r") { |file| @id2title  = Marshal.load file }
      File.open(uris_fname,      "r") { |file| @id2uri    = Marshal.load file }
      File.open(preloaded_fname, "r") { |file| @id2depth  = Marshal.load file }
      true
   end

   def save_indexes
      File.open(index_fname,     "w") { |file| Marshal.dump @index,     file }
      File.open(nodeindex_fname, "w") { |file| Marshal.dump @nodeindex, file }
      File.open(titles_fname,    "w") { |file| Marshal.dump @id2title,  file }
      File.open(uris_fname,      "w") { |file| Marshal.dump @id2uri,    file }
      File.open(preloaded_fname, "w") { |file| Marshal.dump @id2depth,  file }
   end

end

module HTMLIndexer

   def index_documents
      # fix this to use kde's actual dir
      @t1 = Time.now
      @url = first_url
      @indexed_more = false
      already_indexed = load_indexes
      return if already_indexed 
      t1 = Time.now
      @viewed.hide
      @done = []
      @todo_links = []
      progress = KDE::ProgressDialog.new(self, "blah", "Indexing files...", "Abort Indexing", true)
      total_num_files = Dir.glob("#{@pref}/**/*.html").length
      progress.progressBar.setTotalSteps total_num_files
      @todo_links = [ DOM::DOMString.new first_url.url ]
      until @todo_links.empty?
         @todo_next = []
         while more_to_do
            progress.progressBar.setProgress @id2title.keys.length
         end
         puts "now: #{@id2title.keys.length} / #{total_num_files}"
         @todo_links = @todo_next 
         fail "errr, you really didn't want to do that dave" if progress.wasCancelled
      end
      progress.progressBar.setProgress total_num_files
      save_indexes
      t2 = Time.now
      log "all documents indexed in #{(t2 - t1).to_i}s"
   end

   def should_follow_link? href
      return false if DEBUG_FAST
      return case href
      when /source/, /members/
         false
      when /^file:#{@pref}/
         true
      else
         false
      end
   end

   def gather_for_current_page
      todo_links = []
      title_map = {}
      anchors = @viewed.htmlDocument.links
      f = anchors.firstItem
      count = anchors.length
      idx = 0
      caret_node = nil
      while true
         break if (count -= 1) < 0
         text = ""
         DOMUtils.each_child(f) {
            |node|
            text << node.nodeValue.string if node.nodeType == DOM::Node::TEXT_NODE
         }
         link = Qt::Internal::cast_object_to f, "DOM::HTMLLinkElement"
         if should_follow_link? link.href.string
            title_map[link.href.string] = text
            todo_links << link.href
         end
         f = anchors.nextItem
         idx += 1
      end
      todo_links
   end

   DocNodeRef = Struct.new :doc_idx, :node_path

   module IndexDepths
      Nothing, Partial, Node = 0, 1, 2
   end

   def alloc_for_current
      log "making space for url #{@viewed.htmlDocument.URL.string.sub(@pref,"")}"
      @id2uri[@shown_doc_id]   = @viewed.htmlDocument.URL.string
      @id2title[@shown_doc_id] = @viewed.htmlDocument.title.string
      @id2depth[@shown_doc_id] = IndexDepths::Nothing
      @shown_doc_id += 1
   end

   def index_current_document
      @indexed_more = true
      return if @id2depth[@shown_doc_id] >= IndexDepths::Partial
      @label.setText "Scanning : #{@url.prettyURL}"
      log "indexing url #{@viewed.htmlDocument.URL.string.sub(@pref,"")}"
      DOMUtils.each_child(@viewed.document) {
         |node|
         next unless node.nodeType == DOM::Node::TEXT_NODE
         @index.insert_with_key node.nodeValue.string, @shown_doc_id
      }
      @id2depth[@shown_doc_id] = IndexDepths::Partial
      @label.setText "Ready"
   end

   def preload_text
      @indexed_more = true
      return if @id2depth[@shown_doc_id] >= IndexDepths::Node
      log "deep indexing url #{@viewed.htmlDocument.URL.string.sub(@pref,"")}"
      @label.setText "Indexing : #{@url.prettyURL}"
      index_current_document
      doc_text = ""
      t1 = Time.now
      DOMUtils.each_child(@viewed.document) {
         |node|
         next unless node.nodeType == DOM::Node::TEXT_NODE
         ref = DocNodeRef.new @shown_doc_id, DOMUtils.get_node_path(node)
         @nodeindex.insert_with_key node.nodeValue.string, ref
         doc_text << node.nodeValue.string
      }
      @id2depth[@shown_doc_id] = IndexDepths::Node
      @label.setText "Ready"
   end

end

module TermHighlighter

   def update_highlight
      return if @search_text.nil? || @search_text.empty?
      puts "WAAAAAAAAAH!!!!!!!!" if $doingit
      return if $doingit
      $doingit = true
      preload_text
      highlighted_nodes = []
      @nodeindex.search(@search_text).each {
         |ref|
         next unless ref.doc_idx == @shown_doc_id
         highlighted_nodes << ref.node_path
      }
      res = highlight_node_list highlighted_nodes
      puts res
      $doingit = false
   end

   ID_TITLE = 95
   ID_META  = 62
   ID_STYLE = 85

   FORBIDDEN_TAGS = [ID_TITLE, ID_META, ID_STYLE]

   def highlight_node_list highlighted_nodes
      doc = @viewed.document
      no_undo_buffer = @to_undo.nil?
      current_doc_already_highlighted = (@shown_doc_id == @last_highlighted_doc_id)
      undo_highlight @to_undo unless no_undo_buffer or !current_doc_already_highlighted
      @last_highlighted_doc_id = @shown_doc_id
      @to_undo = []
      return if highlighted_nodes.empty?
      @current_matching_node_index = 0 if @current_matching_node_index.nil?
      @current_matching_node_index = @current_matching_node_index.modulo highlighted_nodes.length
      caretnode = DOMUtils.find_node doc, highlighted_nodes[@current_matching_node_index]
      @viewed.setCaretVisible false
      @viewed.setCaretPosition caretnode, 0
      caret_path = DOMUtils.get_node_path(caretnode)
      count = 0
      $blab_recall = []
      highlighted_nodes.each {
         |path|
         node      = DOMUtils.find_node doc, path
         match_idx = node.nodeValue.string.downcase.index @search_text.downcase
         if match_idx.nil?
            $screwups = 0 if $screwups.nil?
            puts match_idx
            puts node
            puts node.nodeValue
            puts node.nodeValue.string
            puts node.nodeValue.string.downcase
            puts @search_text.downcase
            warn "if you see this, then alex screwed up!.... #{$screwups} times!"
            $screwups += 1
            next
         end
         parent_info = DOMUtils.list_parent_node_types node
         has_title_parent = !(parent_info.detect { |a| FORBIDDEN_TAGS.include? a[:elementId] }.nil?)
         next if has_title_parent
         before    = doc.createTextNode    node.nodeValue.split(0)
         matched   = doc.createTextNode  before.nodeValue.split(match_idx)
         after     = doc.createTextNode matched.nodeValue.split(@search_text.length)
         DOM::CharacterData.new(DOM::Node.new after).setData DOM::DOMString.new("") \
                                                     if after.nodeValue.string.nil?
         span      = doc.createElement  DOM::DOMString.new("span")
         spanelt   = DOM::HTMLElement.new span
         classname = (path == caret_path) ? "foundword" : "searchword"
         spanelt.setClassName DOM::DOMString.new(classname)
         span.appendChild matched
         node.parentNode.insertBefore before, node
         # p before.nodeValue.string
         node.parentNode.insertBefore span,   node
         # p matched.nodeValue.string
         node.parentNode.insertBefore after,  node
         # p after.nodeValue.string
         @to_undo       << [node.parentNode, before]
         node.parentNode.removeChild node
         rate = (count > 50) ? 50 : 10
         allow_user_input = ((count+=1) % rate == 0)
         if allow_user_input
            puts "blab!"
            $blab = true
            Qt::Application::eventLoop.processEvents Qt::EventLoop::AllEvents, 10 
            puts "unblab!"
            $blab = false
            if !$blab_recall.empty?
               return false 
            end
            @viewed.view.layout
         end
      }
      puts "waza. finished"
      if !$blab_recall.empty?
         puts "did stuff" 
         @timer.start 0, true
      end
   end

   def undo_highlight to_undo
      puts "undoing!"
      to_undo.reverse.each {
         |pnn| pn, before = *pnn
         mid        = before.nextSibling
         after      = mid.nextSibling
         beforetext = before.nodeValue
         aftertext  = after.nodeValue
         pn.removeChild after
         midtxtnode = mid.childNodes.item(0)
         midtext    = midtxtnode.nodeValue
         # p beforetext.string
         # p midtext.string
         # p aftertext.string
         # puts "#{beforetext.string + midtext.string + aftertext.string}"
         str = DOM::DOMString.new ""
         str.insert aftertext, 0
         str.insert midtext, 0
         str.insert beforetext, 0
         # p str
         # puts "== #{str.string}"
         chardata = DOM::CharacterData.new(DOM::Node.new before)
         chardata.setData str
         pn.removeChild mid
      }
      puts "done!"
   end

end

class RubberDoc < Qt::VBox

   slots "khtml_part_init_complete()", 
         "go_back()", "go_forward()", "go_home()", "goto_url()", 
         "goto_search()", "clicked_result(QListViewItem*)",
         "search(const QString&)", "update_highlight()",
         "quit()", "openURL(const KURL&)", "index_all()",
         "goto_next_match()", "clear_location()", "activated()"

   def quit
      $main.close
   end

   attr_accessor :back, :forward, :url

   include LoggedDebug
   include MyGui
   include IndexStorage
   include HTMLIndexer
   include TermHighlighter

   def initialize parent
      super parent
      @main = parent
      @index     = GenericTriGramIndex.new
      @nodeindex = GenericTriGramIndex.new
      @id2uri, @id2title, @id2depth = {}, {}, {}

      @history, @popped_history = [], []
      @shown_doc_id = 0
      @freq_sorted_idxs = nil
      @last_highlighted_doc_id, @to_undo = nil, nil, nil
      @search_text = nil

      init_gui
      gui_init_proportions

      @timer = Qt::Timer.new self
      Qt::Object.connect @timer,    SIGNAL("timeout()"), 
                         self,      SLOT("update_highlight()")
      @viewed.openURL KDE::URL.new("about:blank")
   end

   def khtml_part_init_complete
      Qt::Object.disconnect @viewed,   SIGNAL("completed()"),
                            self,      SLOT("khtml_part_init_complete()")

      @pref = File.dirname first_url.url.gsub("file:","")

      init_khtml_part_settings @viewed
      index_documents

      # maybe make a better choice as to the start page???
      @shown_doc_id = 0
      goto_url @id2uri[@shown_doc_id], false

      @viewed.show

      search "chomp" if DEBUG_SEARCH || DEBUG_GOTO
      goto_search    if DEBUG_GOTO
   end

   def finish
      save_indexes if @indexed_more
   end

   def init_khtml_part_settings khtmlpart
      khtmlpart.setJScriptEnabled true
      khtmlpart.setJavaEnabled    false
      khtmlpart.setPluginsEnabled false
      khtmlpart.setAutoloadImages false
   end

   def load_page
      @viewed.setCaretMode true
      @viewed.setCaretVisible false
      @viewed.document.setAsync false
      @viewed.document.load DOM::DOMString.new @url.url
      @viewed.setUserStyleSheet "span.searchword { background-color: yellow }
                                 span.foundword  { background-color: green }"
      index_current_document unless @id2depth[@shown_doc_id] >= HTMLIndexer::IndexDepths::Partial
   end

   def first_url
      KDE::URL.new ENV["BASEDOCURL"]
   end

   def search s
      if $blab
         puts "got letter but ignored!!"
         $blab_recall << s
         return
      end
      puts "got letter and reacted!!"
      @search_text = s
      idx_hash = Hash.new { |h,k| h[k] = 0 }
      results = @index.search(s)
      results.each {
         |idx|
         idx_hash[idx] += 1
      }
      @freq_sorted_idxs = idx_hash.to_a.sort_by { |a,b| a[0] <=> b[0] }.reverse
      log "results for this search:"
      update_lv
      hl_timeout = 150 # continuation search should be slower?
      @timer.start hl_timeout, true unless @freq_sorted_idxs.empty?
      puts "finished reacting!"
   end

   def look_for_prefixes
      prefixes = []
      # TODO - fix this crappy hack
      @id2title.values.sort.each {
         |title|
         title.gsub! "\n", ""
         p = title.index ":"
         next if p.nil?
         prefix = title[0..p-1]
         prefixes << prefix
         new_title = title[p+1..title.length]
         new_title.gsub! /(\s\s+|^\s+|\s+$)/, ""
         title.replace new_title 
      }
   end

   def each_result
      look_for_prefixes
      return if @freq_sorted_idxs.nil?
      @freq_sorted_idxs.each {
         |a| 
         idx, count = *a
         yield idx, @id2title[idx]
      }
   end

   def update_lv
      @listview.clear
      lv_root = Qt::ListViewItem.new @listview, "", "results"
      lv_root.setOpen true
      each_result {
         |idx, title|
         Qt::ListViewItem.new lv_root, idx.to_s, title
      }
   end

   def goto_search
      idx, count = *(@freq_sorted_idxs.first)
      goto_id_and_hl idx
   end

   def clicked_result i
      return if i == @listview.firstChild or i.nil?
      idx = i.text(0).to_i
      goto_id_and_hl idx
   end

   def goto_id_and_hl idx
      @current_matching_node_index = 0
      goto_url @id2uri[idx], true
      @shown_doc_id = idx 
      update_highlight
   end

   def goto_next_match
      @current_matching_node_index += 1
      update_highlight
   end

   def more_to_do
      return false if @todo_links.empty?
      lhref = @todo_links.pop
      do_for_link lhref 
      true
   end

   def do_for_link lhref
      idx = (lhref.string =~ /#/)
      unless idx.nil?
         lhref = lhref.copy
         lhref.truncate idx 
      end
      skip = @done.include? lhref.string
      return [] if skip
time_me("loading") {
      @viewed.document.setAsync false
      @viewed.document.load lhref
}
      @done << lhref.string
      alloc_for_current
      @todo_next += gather_for_current_page
   end

   def update_ui_elements
      @forward.setEnabled !@popped_history.empty?
      @back.setEnabled    !@history.empty?
   end

   def ph
      log "ph -> "
      @history.each {
         |kurl| log kurl.url
      }
      log ">"
      log "ph@ -> "
      @popped_history.each {
         |kurl| log kurl.url
      }
      log ">"
   end

   def go_back
      log "go_back"
      ph
      @popped_history << @url
      log "BLAH:"
      log @url.url
      ph
      fail "ummm... already at the start, gui bug" if @history.empty?
      goto_url @history.pop, false, false
      update_loc
      update_ui_elements
      ph
   end

   def go_forward
      log "go_forward"
      fail "history bug" if @popped_history.empty?
      goto_url @popped_history.pop, false, false
      @history << @url
      update_loc
      update_ui_elements
      ph
   end

   def update_loc
      # @location.setText @url.prettyURL
   end

   def go_home
      goto_url first_url
   end

   def index_all
      @viewed.hide
      @id2uri.keys.each {
         |id| goto_id_and_hl id
      }
      @viewed.show
   end

   def goto_url url = nil, history_store = true, blah = true
      log "goto_url"
      @popped_history = [] if blah
      if history_store
         @history << @url
      end
      @url = KDE::URL.new(url.nil? ? @location.text : url)
      @label.setText "Loading : #{@url.prettyURL}"
      load_page
      @label.setText "Ready"
      update_loc unless url.nil?
      update_ui_elements
      ph
   end

end

$doingit = false
$blab = false

m = KDE::MainWindow.new
browser = RubberDoc.new m
browser.update_ui_elements
m.createGUI Dir.pwd + "/xmlgui.rc"
m.setCentralWidget browser
$main = m
app.setMainWidget(m)
m.show
app.exec()
browser.finish

__END__

TESTCASE - 
w = KDE::HTMLPart  # notice the missing .new
w.begin
=> crashes badly

(RECHECK)
./kde.rb:29:in `method_missing': Cannot handle 'const QIconSet&' as argument to QTabWidget::changeTab (ArgumentError)
        from ./kde.rb:29:in `initialize'
        from ./kde.rb:92:in `new'
        from ./kde.rb:92
for param nil given to param const QIconSet &
occurs frequently

dum di dum

can't get tabwidget working. umm... wonder what i'm messing up... (RECHECK)

      tabwidget = KDE::TabWidget.new browser
      tabwidget.setTabPosition Qt::TabWidget::Top
      @viewed = KDE::HTMLPart.new tabwidget
      w2 = KDE::HTMLPart.new tabwidget
      tabwidget.changeTab @viewed, Qt::IconSet.new, "blah blah"
      tabwidget.showPage @viewed
      tabwidget.show
      @viewed.show
 
# possible BUG DOM::Text.new(node).data.string # strange that this one doesn't work... (RECHECK)

wierd khtml bug
      @rightpane.setResizeMode @viewed, Qt::Splitter::KeepSize

in order to use KURL's as constants one must place this KApplication init 
at the top of the file otherwise KInstance isn't init'ed before KURL usage

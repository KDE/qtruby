#!/usr/bin/env ruby

require 'Korundum'

# in order to use KURL's as constants one must place this KApplication init 
# at the top of the file otherwise KInstance isn't init'ed before KURL usage
#
about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["RubberDoc"], about)
a = KDE::Application.new()

# Qt.debug_level = Qt::DebugLevel::High
# Qt.debug_level = Qt::DebugLevel::Extensive

DEBUG        = false
DEBUG_IDX    = false
DEBUG_FAST   = true
DEBUG_SEARCH = false
DEBUG_GOTO   = true

def time_me str
   t1 = Time.now
   yield
   t2 = Time.now
   puts "#{str}: #{"%.02f" % (t2 - t1).to_f}s"
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
         n = n.childNodes.item index
      }
      n
   end

   def DOMUtils.get_node_path node
      n = node
      path_a = []
      until n.isNull
         path_a << n.index if (n.elementId != 0)
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
      trigs = search_string.trigrams
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

      @forward  = KDE::PushButton.new(buttons) { setText "Forward" }
      @back     = KDE::PushButton.new(buttons) { setText "Back" }
      @home     = KDE::PushButton.new(buttons) { setText "Home" }

      @location = Qt::LineEdit.new buttons

      @search   = Qt::LineEdit.new @pane_blah

      @listview = Qt::ListView.new @pane_blah
      @listview.addColumn "IDXs"
      @listview.hideColumn 1 unless DEBUG_IDX
      @listview.addColumn "URIs"
      @listview.setResizeMode Qt::ListView::LastColumn
      @listview.setRootIsDecorated true

      @label    = Qt::Label.new self

      Qt::Object.connect @back,     SIGNAL("clicked()"),
                         self,      SLOT("go_back()")
      Qt::Object.connect @forward,  SIGNAL("clicked()"),
                         self,      SLOT("go_forward()")
      Qt::Object.connect @home,     SIGNAL("clicked()"),
                         self,      SLOT("go_home()")
      Qt::Object.connect @search,   SIGNAL("textChanged(const QString&)"),
                         self,      SLOT("search(const QString&)")
      Qt::Object.connect @search,   SIGNAL("returnPressed()"),
                         self,      SLOT("goto_search()")
      Qt::Object.connect @location, SIGNAL("returnPressed()"),
                         self,      SLOT("goto_url()")
      Qt::Object.connect @listview, SIGNAL("clicked(QListViewItem*)"),
                         self,      SLOT("clicked_result(QListViewItem*)")
      Qt::Object.connect @viewed,   SIGNAL("completed()"),
                         self,      SLOT("khtml_part_init_complete()")

      update_ui_elements
   end

   def gui_init_proportions
      # todo - save these settings
      desktop = Qt::Application::desktop
      sx = (desktop.width  * (2.0/3.0)).to_i
      sy = (desktop.height * (2.0/3.0)).to_i

      resize sx, sy

      logsize     = 0
      resultssize = (sx /  5.0).to_i

      @rightpane.setSizes [sy-logsize, logsize]
      @panes.setSizes     [resultssize, sx-resultssize]

      @rightpane.setResizeMode @logger, Qt::Splitter::KeepSize

      @panes.setResizeMode @pane_blah,   Qt::Splitter::KeepSize
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

   def load_indexes
      return false unless File.exists? index_fname and File.exists? nodeindex_fname
      File.open(index_fname,     "r") { |file| @index     = Marshal.load file }
      File.open(nodeindex_fname, "r") { |file| @nodeindex = Marshal.load file }
      File.open(titles_fname,    "r") { |file| @idx2title = Marshal.load file }
      File.open(uris_fname,      "r") { |file| @idx2uri   = Marshal.load file }
      true
   end

   def save_indexes
      File.open(index_fname,     "w") { |file| Marshal.dump @index,     file }
      File.open(nodeindex_fname, "w") { |file| Marshal.dump @nodeindex, file }
      File.open(titles_fname,    "w") { |file| Marshal.dump @idx2title, file }
      File.open(uris_fname,      "w") { |file| Marshal.dump @idx2uri,   file }
   end

end

module HTMLIndexer

   def index_documents
      # fix this to use kde's actual dir
      if load_indexes
         @url = first_url
         load_page
         self.show
         return
      end
      t1 = Time.now
      @viewed.hide
      @done = []
      @todo_links = []
      @t1 = Time.now
      progress = KDE::ProgressDialog.new(self, "blah", "Indexing files...", "Abort Indexing", true)
      progress.progressBar.setTotalSteps 100
      count = 1
      @url = first_url
      @todo_links = [ DOM::DOMString.new first_url.url ]
      until @todo_links.empty?
         progress.progressBar.setProgress count
         count += 10 # TODO base on rate decrease in page number growth providing total estimate and use done number as current
                     # possibly use some sort of averaging algorithm to stop the progress bar being going crazy
                     # possibly make use of the kde version which iirc has a bouncing version that can be used before estimate is stable
         follow_links
         fail "errr, you really didn't want to do that dave" if progress.wasCancelled
      end
      progress.progressBar.reset
      progress.progressBar.setTotalSteps(@index.trigrams.length * 2)
      progress.setLabel  "Writing indexes..."
      save_indexes
      progress.progressBar.reset
      @viewed.show
      t2 = Time.now
      log "all documents indexed in #{(t2 - t1).to_i}s"
      self.show
   end

   def gather_for_current_page
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
         if link.href.string =~ /^file:/ and !DEBUG_FAST
            title_map[link.href.string] = text
            @todo_links << link.href
         end
         f = anchors.nextItem
         caret_node = f if idx == @current_node_index
         idx += 1
      end
      @current_node_index += 1
      @viewed.setCaretPosition caret_node, 0 unless caret_node.nil?
   end

end

module TermHighlighter

   def preload_text
      log "preloading text for url #{@viewed.htmlDocument.URL.string}"
      doc_text = ""
      @idx2uri[@cur_doc_idx]   = @viewed.htmlDocument.URL.string
      @idx2title[@cur_doc_idx] = @viewed.htmlDocument.title.string
      DOMUtils.each_child(@viewed.document) {
         |node|
         next unless node.nodeType == DOM::Node::TEXT_NODE
         @index.insert_with_key node.nodeValue.string, @cur_doc_idx
         ref = DocNodeRef.new @cur_doc_idx, DOMUtils.get_node_path(node)
         @nodeindex.insert_with_key node.nodeValue.string, ref
         doc_text << node.nodeValue.string
      }
      t2 = Time.now
      log "#{doc_text.length} bytes loaded and indexed in #{"%.02f" % (t2 - @t1).to_f}s"
      @t1 = Time.now
      @shown_doc_id = @cur_doc_idx
      @cur_doc_idx += 1
   end

   def update_highlight
      return if @search_text.empty?
      @viewed.setUserStyleSheet "span.searchword  { background-color: yellow }"
      highlighted_nodes = []
      @nodeindex.search(@search_text).each {
         |ref|
         next unless ref.doc_idx == @shown_doc_id
         highlighted_nodes << ref.node_path
      }
      highlight_node_list highlighted_nodes
   end

   def highlight_node_list highlighted_nodes
      doc = @viewed.document
      current_doc_already_highlighted = @shown_doc_id != @last_highlighted_doc_id
      undo_highlight @to_undo unless @to_undo.nil? or current_doc_already_highlighted 
      @last_highlighted_doc_id = @shown_doc_id
      replaced_nodes = []
      @to_undo = []
      highlighted_nodes.reverse.each {
         |path|
         node      = DOMUtils.find_node doc, path
         nodetext  = node.nodeValue.string
         match_idx = nodetext.downcase.index @search_text
         if match_idx.nil?
            warn "if you see this, then alex screwed up...." 
            next
         end
         before    = doc.createTextNode DOM::DOMString.new(nodetext.slice!(0, match_idx))
         matched   = doc.createTextNode DOM::DOMString.new(nodetext.slice!(0, @search_text.length))
         after     = doc.createTextNode DOM::DOMString.new(nodetext)
         span      = doc.createElement  DOM::DOMString.new("span")
         spanelt   = DOM::HTMLElement.new span
         spanelt.setClassName DOM::DOMString.new("searchword")
         span.appendChild matched
         node.parentNode.insertBefore before, node
         node.parentNode.insertBefore span,   node
         node.parentNode.insertBefore after,  node
         @to_undo       << [node.parentNode, before]
         replaced_nodes << [node.parentNode, node]
      }
      replaced_nodes.each {
         |pnn| pn, n = *pnn
         pn.removeChild n
      }
   end

   def undo_highlight to_undo
      to_undo.each {
         |pnn| pn, before = *pnn
         mid        = before.nextSibling
         after      = mid.nextSibling
         beforetext = before.nodeValue.string
         aftertext  = after.nodeValue.string
         pn.removeChild after
         midtxtnode = mid.childNodes.item(0)
         midtext = midtxtnode.nodeValue.string
         str = DOM::DOMString.new(beforetext + midtext + aftertext)
         chardata = DOM::CharacterData.new(DOM::Node.new before)
         chardata.setData str
         pn.removeChild mid
      }
   end

end

class RubberDoc < Qt::VBox

   DocNodeRef = Struct.new :doc_idx, :node_path

   slots "khtml_part_init_complete()", 
         "go_back()", "go_forward()", "go_home()", "goto_url()", 
         "goto_search()", "clicked_result(QListViewItem*)", 
         "search(const QString&)", "update_highlight()"

   attr_accessor :back, :forward, :url

   include LoggedDebug
   include MyGui
   include IndexStorage
   include HTMLIndexer
   include TermHighlighter

   def initialize( *k )
      super( *k )
      @index     = GenericTriGramIndex.new
      @nodeindex = GenericTriGramIndex.new
      @idx2uri, @idx2title = {}, {}

      @history, @popped_history = [], []
      @current_node_index, @cur_doc_idx = 0, 0
      @freq_sorted_idxs = nil
      @shown_doc_id, @last_highlighted_doc_id, @to_undo = nil, nil, nil
      @search_text = nil

      init_gui

      @timer = Qt::Timer.new self
      Qt::Object.connect @timer,    SIGNAL("timeout()"), 
                         self,      SLOT("update_highlight()")
      @viewed.openURL KDE::URL.new("about:blank")
   end

   def khtml_part_init_complete
      Qt::Object.disconnect @viewed,   SIGNAL("completed()"),
                            self,      SLOT("khtml_part_init_complete()")

      gui_init_proportions
      init_khtml_part_settings @viewed
      index_documents

      search "chomp" if DEBUG_SEARCH || DEBUG_GOTO
      goto_search    if DEBUG_GOTO
   end

   def init_khtml_part_settings khtmlpart
      khtmlpart.setJScriptEnabled true
      khtmlpart.setJavaEnabled    false
      khtmlpart.setPluginsEnabled false
      khtmlpart.setAutoloadImages false
   end

   def load_page
      @viewed.setCaretMode true
      @viewed.document.setAsync false
      @viewed.document.load DOM::DOMString.new @url.url
      @viewed.show
   end

   def first_url
      KDE::URL.new ENV["BASEDOCURL"]
   end

   def search s
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
      @timer.start 150, true unless @freq_sorted_idxs.empty?
   end

   def look_for_prefixes
      prefixes = []
      # TODO - fix this crappy hack
      @idx2title.values.sort.each {
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

   def update_lv
      @listview.clear
      lv_root = Qt::ListViewItem.new @listview, "results"
      lv_root.setOpen true
      look_for_prefixes
      @freq_sorted_idxs.each {
         |a| 
         idx, count = *a
         Qt::ListViewItem.new lv_root, idx.to_s, @idx2title[idx]
      } unless @freq_sorted_idxs.nil?
   end

   def goto_search
      idx, count = *(@freq_sorted_idxs.first)
      @shown_doc_id = idx
      goto_url @idx2uri[idx]
   end

   def clicked_result i
      return if i == @listview.firstChild or i.nil?
      idx = i.text(0).to_i
      @shown_doc_id = idx 
      goto_url @idx2uri[idx]
   end

   def follow_links
      was = @todo_links
      todo_next = []
      was.each {
         |lhref|
         idx = (lhref.string =~ /#/)
         unless idx.nil?
            lhref = lhref.copy
            lhref.truncate idx 
         end
         skip = @done.include? lhref.string
         next if skip
         log "loading: #{lhref.string}"
         @viewed.document.setAsync false
         @viewed.document.load lhref
         @done << lhref.string
         @todo_links = []
         gather_for_current_page
         preload_text
         todo_next += @todo_links
      }
      @todo_links = todo_next 
   end

   def update_ui_elements
      @forward.setDisabled @popped_history.empty?
      @back.setDisabled    @history.empty?
   end

   def go_back
      fail "ummm... already at the start, gui bug" if @history.empty?
      goto_url @history.pop, false
      @popped_history << @url
      update_loc
   end

   def go_forward
      fail "history bug" if @popped_history.empty?
      goto_url @popped_history.pop
      update_loc
   end

   def update_loc
      @location.setText @url.prettyURL
   end

   def go_home
      goto_url first_url
   end

   def goto_url url = nil, history_store = true
      @popped_history = []
      @url = KDE::URL.new(url.nil? ? @location.text : url)
      @label.setText "going somewhere - #{@url.prettyURL}"
      if history_store
         @history << @url
         update_ui_elements
      end
      load_page
      update_loc unless url.nil?
      update_highlight
   end

end

browser = RubberDoc.new
a.setMainWidget(browser)
a.exec()

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

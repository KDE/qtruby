#!/usr/bin/env ruby

require 'Korundum'

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["RubberDoc"], about)
app = KDE::Application.new()

# Qt.debug_level = Qt::DebugLevel::High
# Qt.debug_level = Qt::DebugLevel::Extensive

# TODO
# improve appearence of sidebar massively
#    cut off after certain number of results?
#    seperate title from proof of hit?
# when pressing return adjust the current node
# major speed ups for ctrl-n/p
# ...

DEBUG        = false
DEBUG_IDX    = false
DEBUG_FAST   = true
DEBUG_SEARCH = true
DEBUG_GOTO   = false # crashes?

def time_me str
   t1 = Time.now
   yield
   t2 = Time.now
   log "#{str}: #{"%.02f" % (t2 - t1).to_f}s"
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

   def DOMUtils.each_parent node
      n = node
      until n.isNull
         yield n
         n = n.parentNode
      end
   end

   def DOMUtils.list_parent_node_types node
      types_a = []
      each_parent(node) {
         |n| types_a << { :nodeType => n.nodeType, :elementId => n.elementId }
      }
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

class String
   def trigrams
      list = []
      0.upto(self.length-3) {
         |pos|
         list << self.slice(pos, 3)
      }
      list
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
      warn "searching for a nil???" if search_string.nil?
      return [] if search_string.nil?
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
      puts "LOG: #{s}"
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

      @results_pane = Qt::VBox.new @panes

      @rightpane = Qt::Splitter.new @panes
      @rightpane.setOrientation Qt::Splitter::Vertical
      @viewed = KDE::HTMLPart.new @rightpane
      init_logger @rightpane

      @listbox = Qt::ListBox.new @results_pane

      @label    = Qt::Label.new self

      Qt::Object.connect @listbox,  SIGNAL("clicked(QListBoxItem*)"),
                         self,      SLOT("clicked_result(QListBoxItem*)")
      Qt::Object.connect @viewed,   SIGNAL("completed()"),
                         self,      SLOT("khtml_part_init_complete()")

      Qt::Object::connect @viewed,  SIGNAL("setWindowCaption(const QString&)"),
                          @viewed.widget.topLevelWidget, 
                                    SLOT("setCaption(const QString&)")

      Qt::Object::connect @viewed.browserExtension, 
                                    SIGNAL("openURLRequest(const KURL&, const KParts::URLArgs&)"),
                          self,     SLOT("open_url(const KURL&)")

      KDE::Action.new "&Quit",        "quit",    KDE::Shortcut.new(), 
                      self, SLOT("quit()"),                     @main.actionCollection, "file_quit"
      KDE::Action.new "&Index-All",              KDE::Shortcut.new(), 
                      self, SLOT("index_all()"),                @main.actionCollection, "index_all"
      @back = \
      KDE::Action.new "&Back",        "back",    KDE::Shortcut.new(Qt::ALT + Qt::Key_Left), 
                      self, SLOT("go_back()"),                  @main.actionCollection, "back"
      @forward = \
      KDE::Action.new "&Forward",     "forward", KDE::Shortcut.new(Qt::ALT + Qt::Key_Right), 
                      self, SLOT("go_forward()"),               @main.actionCollection, "forward"
      KDE::Action.new "&Home",        "gohome",  KDE::Shortcut.new(Qt::Key_Home), 
                      self, SLOT("go_home()"),                  @main.actionCollection, "home"
      KDE::Action.new "&Prev Match",  "previous",KDE::Shortcut.new(Qt::CTRL + Qt::Key_P), 
                      self, SLOT("goto_prev_match()"),          @main.actionCollection, "prev_match"
      KDE::Action.new "&Next Match",  "next",    KDE::Shortcut.new(Qt::CTRL + Qt::Key_N), 
                      self, SLOT("goto_next_match()"),          @main.actionCollection, "next_match"
      KDE::Action.new "&Follow Match","down",    KDE::Shortcut.new(Qt::Key_Return), 
                      self, SLOT("goto_current_match_link()"),  @main.actionCollection, "open_match"

      KDE::Action.new "Search",  "find",         KDE::Shortcut.new(Qt::Key_F6), 
                      self, SLOT("focus_search()"),             @main.actionCollection, "focus_search"
      KDE::Action.new "New Search", "find",      KDE::Shortcut.new(Qt::CTRL + Qt::Key_Slash), 
                      self, SLOT("focus_and_clear_search()"),   @main.actionCollection, "focus_and_clear_search"

      KDE::Action.new "&Create",  "new",         KDE::Shortcut.new(), 
                      self, SLOT("project_create()"),           @main.actionCollection, "project_create"
      KDE::Action.new "&Choose...",  "select",   KDE::Shortcut.new(), 
                      self, SLOT("project_goto()"),             @main.actionCollection, "project_goto"

      clearLocation = KDE::Action.new "Clear Location Bar", "locationbar_erase", KDE::Shortcut.new(), 
                      self, SLOT("clear_location()"), @main.actionCollection, "clear_location"
      clearLocation.setWhatsThis "Clear Location bar<p>Clears the content of the location bar."

      @searchlabel = Qt::Label.new @main
      @searchlabel.setText "Search: "

      @searchcombo = KDE::HistoryCombo.new @main
      focus_search
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

   def focus_search
      @searchcombo.setFocus
   end

   def focus_and_clear_search
      clear_location
      focus_search
   end

   def clear_location
      @searchcombo.clearEdit
   end

   def uri_anchor_split url
      url =~ /(.*?)(#(.*))?$/
      return $1, $3
   end

   def open_url kurl
      url, anchor = uri_anchor_split kurl.url
      goto_url url, false unless id == @shown_doc_id
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

      @rightpane.setResizeMode @logger,   Qt::Splitter::KeepSize

      @panes.setResizeMode @results_pane, Qt::Splitter::KeepSize
      @panes.setResizeMode @rightpane,    Qt::Splitter::KeepSize
   end

end

module IndexStorage

   INDEX_VERSION = 3

   IndexStore = Struct.new :index, :nodeindex, :textcache, :id2title, :id2uri, :id2depth, :version

   def index_fname
      basedir = ENV["HOME"] + "/.rubberdocs"
      prefix = basedir + "/." + @pref.gsub(/\//,",") + ".idx"
      Dir.mkdir basedir unless File.exists? basedir
      "#{prefix}.doc"
   end

   def depth_debug
      puts "depth_debug : begin"
      @id2depth.each_key {
         |id|
         puts "indexed to depth #{@id2depth[id]} : #{@id2uri[id]}"
      }
      puts "end :"
   end

   def load_indexes
      return false unless File.exists? index_fname
      Qt::Application::setOverrideCursor(Qt::Cursor.new Qt::WaitCursor)
      File.open(index_fname, "r") {
         |file| 
         w = Marshal.load file rescue nil
         return false if w.nil? || w.version < INDEX_VERSION
         @index     = w.index
         @nodeindex = w.nodeindex
         @textcache = w.textcache
         @id2title  = w.id2title
         @id2uri    = w.id2uri
         @id2depth  = w.id2depth
         @indexed_more = false
         true
      }
      Qt::Application::restoreOverrideCursor
   end

   def save_indexes
      return unless @indexed_more
      File.open(index_fname, "w") {
         |file| 
         w = IndexStore.new
         w.index      = @index
         w.nodeindex  = @nodeindex
         w.textcache  = @textcache
         w.id2title   = @id2title
         w.id2uri     = @id2uri
         w.id2depth   = @id2depth
         w.version    = INDEX_VERSION
         Marshal.dump w, file
      }
   end

end

module HTMLIndexer

   DocNodeRef = Struct.new :doc_idx, :node_path

   module IndexDepths
      # TitleIndexed implies "LinkTitlesIndexed"
      Allocated, TitleIndexed, LinksFollowed, Partial, Node = 0, 1, 2, 3, 4
   end

   def index_documents
      # fix this to use kde's actual dir
      @t1 = Time.now
      @url = first_url
      already_indexed = load_indexes
      @top_doc_id = already_indexed ? @id2uri.keys.max + 1 : 0
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
         @todo_links = @todo_next 
         fail "errr, you really didn't want to do that dave" if progress.wasCancelled
      end
      progress.progressBar.setProgress total_num_files
      save_indexes
      t2 = Time.now
      log "all documents indexed in #{(t2 - t1).to_i}s"
   end

   def should_follow? lhref
      case lhref
      when /source/, /members/
         ret = false
      when /^file:#{@pref}/
         ret = true
      else
         ret = false
      end
      ret
   end

   def gather_for_current_page
      index_current_title
      return [] if @id2depth[@shown_doc_id] >= IndexDepths::LinksFollowed
      todo_links = []
      title_map = {}
      anchors = @viewed.htmlDocument.links
      f = anchors.firstItem
      count = anchors.length
      until (count -= 1) < 0
         text = ""
         DOMUtils.each_child(f) {
            |node|
            text << node.nodeValue.string if node.nodeType == DOM::Node::TEXT_NODE
         }
         link = Qt::Internal::cast_object_to f, DOM::HTMLLinkElement
         if should_follow? link.href.string
            title_map[link.href.string] = text
            urlonly, = uri_anchor_split link.href.string
            add_link_to_index urlonly, text
            todo_links << link.href unless DEBUG_FAST
         end
         f = anchors.nextItem
      end
      @id2depth[@shown_doc_id] = IndexDepths::LinksFollowed
      return todo_links
   end

   def find_allocated_uri uri
      id = @id2uri.invert[uri]
      return id
   end

   # sets @shown_doc_id
   def index_current_title
      id = find_allocated_uri(@viewed.htmlDocument.URL.string)
      return if !id.nil? and @id2depth[id] >= IndexDepths::TitleIndexed
      log "making space for url #{@viewed.htmlDocument.URL.string.sub(@pref,"")}"
      id = alloc_index_space @viewed.htmlDocument.URL.string if id.nil?
      @indexed_more = true
      @id2title[id] = @viewed.htmlDocument.title.string
      @id2depth[id] = IndexDepths::TitleIndexed
      @shown_doc_id = id
   end

   def alloc_index_space uri
      @indexed_more = true
      id = @top_doc_id
        @id2uri[@top_doc_id] = uri
      @id2title[@top_doc_id] = nil
      @id2depth[@top_doc_id] = IndexDepths::Allocated
      @top_doc_id += 1
      id
   end

   def add_link_to_index uri, title
      return unless find_allocated_uri(uri).nil?
      @indexed_more = true
      new_id = alloc_index_space uri
      @id2title[new_id] = title
      @index.insert_with_key title, new_id
   end

   def index_current_document
      return if @id2depth[@shown_doc_id] >= IndexDepths::Partial
      Qt::Application::setOverrideCursor(Qt::Cursor.new Qt::WaitCursor)
      @indexed_more = true
      @label.setText "Scanning : #{@url.prettyURL}"
      log "indexing url #{@viewed.htmlDocument.URL.string.sub(@pref,"")}"
      DOMUtils.each_child(@viewed.document) {
         |node|
         next unless node.nodeType == DOM::Node::TEXT_NODE
         @index.insert_with_key node.nodeValue.string, @shown_doc_id
      }
      @id2depth[@shown_doc_id] = IndexDepths::Partial
      @label.setText "Ready"
      Qt::Application::restoreOverrideCursor
   end

   def preload_text
      return if @id2depth[@shown_doc_id] >= IndexDepths::Node
      Qt::Application::setOverrideCursor(Qt::Cursor.new Qt::WaitCursor)
      @indexed_more = true
      index_current_document
      log "deep indexing url #{@viewed.htmlDocument.URL.string.sub(@pref,"")}"
      @label.setText "Indexing : #{@url.prettyURL}"
      doc_text = ""
      t1 = Time.now
      DOMUtils.each_child(@viewed.document) {
         |node|
         next unless node.nodeType == DOM::Node::TEXT_NODE
         ref = DocNodeRef.new @shown_doc_id, DOMUtils.get_node_path(node)
         @nodeindex.insert_with_key node.nodeValue.string, ref
         @textcache[ref] = node.nodeValue.string
         doc_text << node.nodeValue.string
      }
      @id2depth[@shown_doc_id] = IndexDepths::Node
      @label.setText "Ready"
      Qt::Application::restoreOverrideCursor
   end

end

# TODO - this sucks, use khtml to get the values
module IDS
   A     = 1
   META  = 62
   STYLE = 85
   TITLE = 95
end

module TermHighlighter

   include IDS

   FORBIDDEN_TAGS = [IDS::TITLE, IDS::META, IDS::STYLE]

   def update_highlight
      return if @search_text.nil? || @search_text.empty?
      return if @in_update_highlight
      @in_update_highlight = true
      preload_text
      highlighted_nodes = []
      @nodeindex.search(@search_text).each {
         |ref|
         next unless ref.doc_idx == @shown_doc_id
         highlighted_nodes << ref.node_path
      }
      highlight_node_list highlighted_nodes
      @in_update_highlight = false
   end

   def mark_screwup
      @screwups = 0 if @screwups.nil?
      warn "if you see this, then alex screwed up!.... #{@screwups} times!"
      @screwups += 1
   end

   def highlight_node_list highlighted_nodes
      doc = @viewed.document
      no_undo_buffer = @to_undo.nil?
      current_doc_already_highlighted = (@shown_doc_id == @last_highlighted_doc_id)
      undo_highlight @to_undo unless no_undo_buffer or !current_doc_already_highlighted
      @last_highlighted_doc_id = @shown_doc_id
      @to_undo = []
      return if highlighted_nodes.empty?
      Qt::Application::setOverrideCursor(Qt::Cursor.new Qt::WaitCursor)
      cursor_override = true
      @current_matching_node_index = 0 if @current_matching_node_index.nil?
      @current_matching_node_index = @current_matching_node_index.modulo highlighted_nodes.length
      caretnode = DOMUtils.find_node doc, highlighted_nodes[@current_matching_node_index]
      @viewed.setCaretVisible false
      @viewed.setCaretPosition caretnode, 0
      caret_path = DOMUtils.get_node_path(caretnode)
      count = 0
      @skipped_highlight_requests = false
      @current_matched_href       = nil
      highlighted_nodes.sort.each {
         |path|
         node      = DOMUtils.find_node doc, path
         next mark_screwup if node.nodeValue.string.nil?
         match_idx = node.nodeValue.string.downcase.index @search_text.downcase
         next mark_screwup if match_idx.nil?
         parent_info = DOMUtils.list_parent_node_types node
         has_title_parent = !(parent_info.detect { |a| FORBIDDEN_TAGS.include? a[:elementId] }.nil?)
         next if has_title_parent
         if path == caret_path
            DOMUtils.each_parent(node) {
               |n| 
               next unless n.elementId == IDS::A
               # link = DOM::HTMLLinkElement.new n # WTF? why doesn't this work???
               link = Qt::Internal::cast_object_to n, "DOM::HTMLLinkElement"
               @current_matched_href = link.href.string
            }
         end
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
         node.parentNode.insertBefore span,   node
         node.parentNode.insertBefore after,  node
         @to_undo       << [node.parentNode, before]
         node.parentNode.removeChild node
         rate = (count > 50) ? 50 : 10
         allow_user_input = ((count+=1) % rate == 0)
         if allow_user_input
            Qt::Application::restoreOverrideCursor if cursor_override
            cursor_override = false
            @in_node_highlight = true
            Qt::Application::eventLoop.processEvents Qt::EventLoop::AllEvents, 10 
            @in_node_highlight = false
            if @skipped_highlight_requests
               @timer.start 50, true
               return false 
            end
            @viewed.view.layout
         end
      }
      if @skipped_highlight_requests
         @timer.start 50, true
      end
      Qt::Application::restoreOverrideCursor if cursor_override
   end

   def undo_highlight to_undo
      to_undo.reverse.each {
         |pnn| pn, before = *pnn
         mid        = before.nextSibling
         after      = mid.nextSibling
         beforetext = before.nodeValue
         aftertext  = after.nodeValue
         pn.removeChild after
         midtxtnode = mid.childNodes.item(0)
         midtext    = midtxtnode.nodeValue
         str = DOM::DOMString.new ""
         str.insert aftertext, 0
         str.insert midtext, 0
         str.insert beforetext, 0
         chardata = DOM::CharacterData.new(DOM::Node.new before)
         chardata.setData str
         pn.removeChild mid
      }
   end

end

class SmallIconSet
   def SmallIconSet.[] name
      loader = KDE::Global::instance.iconLoader
      return loader.loadIconSet name, KDE::Icon::Small, 0
   end
end

class ProjectEditDialog < Qt::Object

   slots "select_file()", "slot_ok()"

   def initialize project_name, parent=nil,name=nil,caption=nil

      super(parent, name)
      @parent = parent

      @dialog = KDE::DialogBase.new(parent,name, true, caption,
                                    KDE::DialogBase::Ok|KDE::DialogBase::Cancel, KDE::DialogBase::Ok, false)

      vbox = Qt::VBox.new @dialog

      grid = Qt::Grid.new 2, Qt::Horizontal, vbox

      titlelabel = Qt::Label.new "Name:", grid
      @title      = KDE::LineEdit.new grid
      titlelabel.setBuddy @title

      urllabel   = Qt::Label.new "Location:", grid
      lochbox    = Qt::HBox.new grid
      @url        = KDE::LineEdit.new lochbox
      urllabel.setBuddy @url
      locselc    = Qt::PushButton.new lochbox
      locselc.setIconSet SmallIconSet["up"]

      blub = Qt::HBox.new vbox
      Qt::Label.new "Is main one?:", blub 
      @cb = Qt::CheckBox.new blub 

      enabled = @parent.projects_data.project_list.empty?

      unless project_name.nil? 
         project_url = @parent.projects_data.project_list[project_name]
         @title.setText project_name
         @url.setText   project_url
         enabled = true if (project_name == @parent.projects_data.enabled_name)
      end

      @cb.setChecked true if enabled

      Qt::Object.connect @dialog, SIGNAL("okClicked()"), 
                         self,    SLOT("slot_ok()")
 
      Qt::Object.connect locselc, SIGNAL("clicked()"), 
                         self,    SLOT("select_file()")
 
      @title.setFocus

      @dialog.setMainWidget vbox

      @modified = false
   end

   def select_file
      s = Qt::FileDialog::getOpenFileName ENV["HOME"], "HTML Files (*.html)",
                                          @parent, "open file dialog", "Choose a file"
      @url.setText s unless s.nil?
   end

   def edit
      @dialog.exec
      return @modified
   end

   def new_name
      @title.text
   end

   def new_url
      @url.text
   end

   def new_enabled
      @cb.isChecked
   end

   def slot_ok
      @parent.projects_data.project_list[new_name] = new_url
      @parent.projects_data.enabled_name = new_name if new_enabled
      @modified = true
   end

end

class ProjectSelectDialog < Qt::Object

   slots "edit_selected_project()", "delete_selected_project()", "project_create_button()", "project_selected()"

   def initialize parent=nil,name=nil,caption=nil
      super(parent, name)
      @parent = parent

      @dialog = KDE::DialogBase.new parent,name, true, caption,
                                    KDE::DialogBase::Ok|KDE::DialogBase::Cancel, KDE::DialogBase::Ok, false

      vbox = Qt::VBox.new @dialog

      @listbox = Qt::ListBox.new vbox

      fill_listbox

      hbox = Qt::HBox.new vbox
      button_new  = Qt::PushButton.new "New...", hbox
      button_del  = Qt::PushButton.new "Delete", hbox
      button_edit = Qt::PushButton.new "Edit...", hbox

      Qt::Object.connect button_new,  SIGNAL("clicked()"), 
                         self,        SLOT("project_create_button()")

      Qt::Object.connect button_del,  SIGNAL("clicked()"), 
                         self,        SLOT("delete_selected_project()")

      Qt::Object.connect button_edit, SIGNAL("clicked()"), 
                         self,        SLOT("edit_selected_project()")

      Qt::Object.connect @listbox,    SIGNAL("doubleClicked(QListBoxItem *)"), 
                         self,        SLOT("project_selected()")

      @dialog.setMainWidget vbox
   end

   def project_selected
      return if @listbox.selectedItem.nil?
      @parent.current_project_name = @listbox.selectedItem.text
      @parent.blah_blah
      @dialog.reject
   end

   def fill_listbox
      @listbox.clear
      @parent.projects_data.project_list.keys.each {
         |name| 
         enabled = (name == @parent.projects_data.enabled_name)
         icon = enabled ? "forward" : "down"
         pm = SmallIconSet[icon].pixmap(Qt::IconSet::Automatic, Qt::IconSet::Normal)
         it = Qt::ListBoxPixmap.new pm, name
         @listbox.insertItem it
      }
   end

   def edit_selected_project
      return if @listbox.selectedItem.nil?
      oldname = @listbox.selectedItem.text
      dialog = ProjectEditDialog.new oldname, @parent
      mod = dialog.edit
      if mod and oldname != dialog.new_name
         @parent.projects_data.project_list.delete oldname
      end
      fill_listbox if mod
   end

   def project_create_button
      mod = @parent.project_create
      fill_listbox if mod
   end

   def delete_selected_project
      return if @listbox.selectedItem.nil?
      # TODO - confirmation dialog
      @parent.projects_data.project_list.delete @listbox.selectedItem.text
      fill_listbox
   end

   def select
      @dialog.exec
   end

end

module ProjectManager

   def project_create
      dialog = ProjectEditDialog.new nil, self
      dialog.edit
      while @projects_data.project_list.empty?
         dialog.edit
      end
   end

   def project_goto
      dialog = ProjectSelectDialog.new self
      dialog.select
      if @projects_data.project_list.empty?
         project_create
      end
   end

   require 'yaml'

   def yamlfname 
      ENV["HOME"] + "/.rubberdocs/projects.yaml"
   end

   PROJECT_STORE_VERSION = 0

   Projects = Struct.new :project_list, :enabled_name, :version

   def load_projects
      okay = false
      if File.exists? yamlfname
         @projects_data = YAML::load File.open(yamlfname)
         if (@projects_data.version rescue -1) >= PROJECT_STORE_VERSION
            okay = true
         end
      end
      if not okay or @projects_data.project_list.empty?
         @projects_data = Projects.new({}, nil, PROJECT_STORE_VERSION)
         project_create
      end
      if @projects_data.enabled_name.nil?
         @projects_data.enabled_name = @projects_data.project_list.keys.first
      end
   end

   def save_projects
      File.open(yamlfname, "w+") {
         |file|
         file.puts @projects_data.to_yaml
      }
   end

end

class RubberDoc < Qt::VBox

   slots "khtml_part_init_complete()", 
         "go_back()", "go_forward()", "go_home()", "goto_url()", 
         "goto_search()", "clicked_result(QListBoxItem*)",
         "search(const QString&)", "update_highlight()",
         "quit()", "open_url(const KURL&)", "index_all()",
         "goto_prev_match()", "goto_next_match()", "clear_location()", "activated()",
         "goto_current_match_link()", "focus_search()", "focus_and_clear_search()",
         "project_create()", "project_goto()"

   attr_accessor :back, :forward, :url, :projects_data

   include LoggedDebug
   include MyGui
   include IndexStorage
   include HTMLIndexer
   include TermHighlighter
   include ProjectManager

   def init_blah
      @index     = GenericTriGramIndex.new
      @nodeindex = GenericTriGramIndex.new
      @textcache = {}
      @id2uri, @id2title, @id2depth = {}, {}, {}

      @history, @popped_history = [], []
      @shown_doc_id = 0
      @freq_sorted_idxs = nil
      @last_highlighted_doc_id, @to_undo = nil, nil, nil
      @search_text = nil
      @current_matched_href = nil

      @in_update_highlight = false
      @in_node_highlight   = false

      @lvis = nil
   end

   def initialize parent
      super parent
      @main = parent

      load_projects
      @current_project_name = @projects_data.enabled_name

      init_blah

      init_gui
      gui_init_proportions

      @timer = Qt::Timer.new self
      Qt::Object.connect @timer,    SIGNAL("timeout()"), 
                         self,      SLOT("update_highlight()")

      @viewed.openURL KDE::URL.new("about:blank")

      @init_connected = true
   end

   def blah_blah
      save_indexes
      init_blah
      khtml_part_init_complete
   end

   def quit
      @main.close
   end

   def khtml_part_init_complete
      Qt::Object.disconnect @viewed,   SIGNAL("completed()"),
                            self,      SLOT("khtml_part_init_complete()") if @init_connected

      @pref = File.dirname first_url.url.gsub("file:","")

      init_khtml_part_settings @viewed if @init_connected
      index_documents

      # maybe make a better choice as to the start page???
      @shown_doc_id = 0
      goto_url @id2uri[@shown_doc_id], false

      @viewed.show

      search "qlistview" if DEBUG_SEARCH || DEBUG_GOTO
      goto_search        if DEBUG_GOTO

      @init_connected = false
   end

   def finish
      save_projects
      save_indexes
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
      Qt::Application::eventLoop.processEvents Qt::EventLoop::ExcludeUserInput
   end

   attr_accessor :current_project_name

   def first_url
      return KDE::URL.new @projects_data.project_list[@current_project_name]
   end

   def search s
      if @in_node_highlight
         @skipped_highlight_requests = true
         return
      end
      puts "search request: #{s}"
      @search_text = s

      results = @index.search(s)
      results += @nodeindex.search(s).collect { |docref| docref.doc_idx }

      idx_hash = Hash.new { |h,k| h[k] = 0 }
      results.each {
         |idx| idx_hash[idx] += 1
      }
      @freq_sorted_idxs = idx_hash.to_a.sort_by { |val| val[1] }.reverse

      update_lv

      hl_timeout = 150 # continuation search should be slower?
      @timer.start hl_timeout, true unless @freq_sorted_idxs.empty?
   end

   def look_for_prefixes
      prefixes = []
      # TODO - fix this crappy hack
      @id2title.values.compact.sort.each {
         |title|
         title.gsub! "\n", ""
         pos = title.index ":"
         next if pos.nil?
         prefix = title[0..pos-1]
         prefixes << prefix
         new_title = title[pos+1..title.length]
         new_title.gsub! /(\s\s+|^\s+|\s+$)/, ""
         title.replace new_title 
      }
   end

   class ResultItem < Qt::ListBoxItem
      def initialize header, text
         super()
         @text, @header = text, header
         @font  = Qt::Font.new("Helvetica", 8)
         @flags = Qt::AlignLeft | Qt::WordBreak
      end
      def paint painter
         w, h = width(listBox), height(listBox)
         header_height = (text_height @font, @header) + 5
         painter.setFont @font
         painter.fillRect 5, 5, w - 10, header_height, Qt::Brush.new(Qt::Color.new 150,100,150)
         painter.drawText 5, 5, w - 10, header_height, @flags, @header
         painter.fillRect 5, header_height, w - 10, h - 10, Qt::Brush.new(Qt::Color.new 100,150,150)
         painter.setFont @font
         painter.drawText 5, header_height + 2, w - 10, h - 10, @flags, @text
      end
      def text_height font, text
         fm = Qt::FontMetrics.new font
         br = fm.boundingRect 0, 0, width(listBox) - 20, 8192, @flags, text
         br.height
      end
      def height listbox
         h = 0
         h += text_height @font, @text
         h += text_height @font, @header
         return h + 10
      end
      def width listbox
         listBox.width - 5
      end
   end

   CUTOFF = 100
   def update_lv
      @listbox.clear
      @lvis = {}
      look_for_prefixes
      return if @freq_sorted_idxs.nil?
      @freq_sorted_idxs.each {
         |a| idx, count = *a
         title = @id2title[idx]
         # we must re-search until we have a doc -> nodes list
         matches_text = ""
         @nodeindex.search(@search_text).each {
            |ref|
            break if matches_text.length > CUTOFF
            next unless ref.doc_idx == idx
            matches_text << @textcache[ref] << "\n"
         }
         matches_text = matches_text.slice 0..CUTOFF
         lvi = ResultItem.new "(#{count}) #{title}", matches_text
         @listbox.insertItem lvi
         @lvis[lvi] = idx
      }
   end

   def goto_search
      idx, count = *(@freq_sorted_idxs.first)
      goto_id_and_hl idx
   end

   def clicked_result i
      return if i.nil?
      idx = @lvis[i]
      goto_id_and_hl idx
   end

   def goto_id_and_hl idx
      @current_matching_node_index = 0
      goto_url @id2uri[idx], true
      @shown_doc_id = idx 
      update_highlight
   end

   def goto_current_match_link
      open_url KDE::URL.new(@current_matched_href) unless @current_matched_href.nil?
   end

   def skip_matches n
      @current_matching_node_index += n # autowraps
      if @in_node_highlight
         @skipped_highlight_requests = true
         return
      end
      update_highlight
   end

   def goto_prev_match
      skip_matches -1
   end

   def goto_next_match
      skip_matches +1
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
      newlinks = gather_for_current_page
      @todo_next += newlinks
   end

   def update_ui_elements
      @forward.setEnabled !@popped_history.empty?
      @back.setEnabled    !@history.empty?
   end

   def go_back
      @popped_history << @url
      fail "ummm... already at the start, gui bug" if @history.empty?
      goto_url @history.pop, false, false
      update_loc
      update_ui_elements
   end

   def go_forward
      fail "history bug" if @popped_history.empty?
      goto_url @popped_history.pop, false, false
      @history << @url
      update_loc
      update_ui_elements
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

   def goto_url url = nil, history_store = true, clear_forward = true
      @popped_history = [] if clear_forward
      if history_store
         @history << @url
      end
      @url = KDE::URL.new(url.nil? ? @location.text : url)
      @label.setText "Loading : #{@url.prettyURL}"
      urlonly, = uri_anchor_split @url.url
      id = @id2uri.invert[@url.url]
      if id.nil? and !(should_follow? urlonly)
            warn "link points outside indexed space!" 
            return
      end
      load_page
      if id.nil?
         gather_for_current_page
         id = @shown_doc_id
         index_current_document
      else
         @shown_doc_id = id
      end
      @label.setText "Ready"
      update_loc unless url.nil?
      update_ui_elements
   end

end

m = KDE::MainWindow.new
browser = RubberDoc.new m
browser.update_ui_elements
guixmlfname = Dir.pwd + "/RubberDoc.rc"
guixmlfname = File.dirname(File.readlink $0) + "/RubberDoc.rc" unless File.exists? guixmlfname
m.createGUI guixmlfname
m.setCentralWidget browser
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

class ProjectSelectDialog < KDE::DialogBase
   def initialize parent=nil,name=nil,caption=nil
      super(parent,name, true, caption,
            KDE::DialogBase::Ok|KDE::DialogBase::Cancel, KDE::DialogBase::Ok, false, KDE::GuiItem.new)
      blah blah
   end
end

# painter.fillRect 5, 5, width(listBox) - 10, height(listBox) - 10, Qt::Color.new(255,0,0)

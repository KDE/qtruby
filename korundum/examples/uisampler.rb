=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

require 'korundum4'

require 'uimodules/uiwidgets.rb'
require 'uimodules/uidialogs.rb'
require 'uimodules/uimenus.rb'
require 'uimodules/uimisc.rb'
require 'uimodules/uixml.rb'

$listItems = {"Dialogs" =>
                {"KDE::AboutDialog" => ["KDE::AboutApplication", "KDE::AboutContainer", "KDE::ImageTrackLabel",
                                  "KDE::AboutContainerBase", "KDE::AboutContributor", "KDE::AboutWidget"],
                "KDE::AboutApplicationDialog" => [],
                "KDE::BugReport" => [],
                "KDE::ColorDialog" => [],
                "KDE::Dialog" => [],
                "KDE::DialogBase" => ["KDE::DialogBaseButton", "KDE::DialogBase::SButton", "KDE::DialogBaseTile"],
                "KDE::FontDialog" => [],
                "KDE::ShortcutsDialog" => [],
                "KDE::LineEditDlg" => [],
                "KDE::MessageBox" => [],
                "KDE::PasswordDialog" => [],
                "KDE::Wizard" => []},
            "Widgets" =>
                {"KDE::AnimWidget" => [],
                 "KDE::AuthIcon" => ["KDE::RootPermsIcon", "KDE::WritePermsIcon"],
                 "KDE::ButtonBox" => [],
                 "KDE::CharSelect" => ["KDE::CharSelectTable"],
                 "KDE::ColorButton" => [],
                 "KDE::ColorCells" => [],
                 "KDE::ColorCombo" => [],
                 "KDE::ColorPatch" => [],
                 "KDE::ColorTable" => [],
                 "KDE::ComboBox" => [],
                 "KDE::CompletionBox" => [],
                 "KDE::ContainerLayout" => ["KDE::ContainerLayout::KContainerLayoutItem"],
                 "KDE::Cursor" => [],
                 "KDE::DatePicker" => ["KDE::DateInternalMonthPicker", "KDE::DateInternalYearSelector"],
                 "KDE::DateTable" => [],
                 "KDE::EditListBox" => [],
                 "KDE::FontChooser" => [],
                 "KDE::HueSaturationSelector" => [],
                 "KDE::IconView" => [],
                 "KDE::JanusWidget" => ["KDE::JanusWidget::IconListBox"],
                 "KDE::KeyChooser" => [],
                 "KDE::Led" => [],
                 "KDE::LineEdit" => [],
                 "KDE::ListBox" => [],
                 "KDE::TreeWidget" => [],
                 "KDE::NumInput" => ["KDE::DoubleNumInput", "KDE::IntNumInput"],
                 "KDE::PasswordEdit" => [],
                 "KDE::RootPixmap" => [],
                 "KDE::MainWindow" => [],
                 "KDE::RestrictedLine" => [],
                 "KDE::Ruler" => [],
                 "KDE::Selector" => ["KDE::GradientSelector", "KDE::ValueSelector", "KDE::HSSelector", "KDE::XYSelector"],
                 "KDE::Separator" => [],
                 "KDE::SqueezedTextLabel" => [],
                 "KDE::TextBrowser" => [],
                 "KDE::TextEdit" => ["KDE::EdFind", "KDE::EdGotoLine", "KDE::EdReplace"],
                 "KDE::UrlLabel" => []},
            "XML" =>
                {"KDE::ActionCollection" => [],
                 "KDE::EditToolbar" => [],
                 "KDE::EditToolbarWidget" => [],
                 "KDE::XMLGUIBuilder" => [],
                 "KDE::XMLGUIClient" => ["KDE::XMLGUIClient::DocStruct"],
                 "KDE::XMLGUIFactory" => []},
            "Menus/Toolbars" =>
                {"KDE::AccelMenu" => [],
                 "KDE::Action" => ["KDE::FontAction", "KDE::FontSizeAction", "KDE::ListAction", "KDE::RecentFilesAction", "KDE::RadioAction",
                             "KDE::SelectAction", "KDE::ToggleAction"],
                 "KDE::ActionMenu" => [],
                 "KDE::ActionSeparator" => [],
                 "KDE::ContextMenuManager" => [],
                 "KDE::DCOPActionProxy" => [],
                 "KDE::HelpMenu" => [],
                 "KDE::MenuBar" => [],
                 "KDE::PanelApplet" => [],
                 "KDE::PanelExtension" => [],
                 "KDE::PanelMenu" => [],
                 "KDE::PopupFrame" => [],
                 "KDE::PopupMenu" => [],
                 "KDE::PopupTitle" => [],
                 "KDE::StatusBar" => [],
                 "KDE::StatusBarLabel" => [],
                 "KDE::StdAction" => [],
                 "KDE::ToolBar" => ["KDE::ToolBarButton", "KDE::ToolBarButtonList", "KDE::ToolBarPopupAction",
                              "KDE::ToolBarRadioGroup", "KDE::ToolBarSeparator"],
                 "KDE::WindowListMenu" => []},
            "Other" =>
                {"KDE::AlphaPainter" => [],
                 "KDE::CModule" => [],
                 "KDE::Color" => [],
                 "KDE::ColorDrag" => [],
                 "KDE::Command" => ["KDE::MacroCommand"],
                 "KDE::CommandHistory" => [],
                 "KDE::DateValidator" => [],
                 "KDE::DockWindow" => ["KDE::DockButton_Private - KPanelMenu", "KDE::DockButton_Private",
                                 "KDE::DockSplitter", "KDE::DockTabCtl_PrivateStruct", "KDE::DockWidgetAbstractHeader",
                                 "KDE::DockWidgetAbstractHeaderDrag", "KDE::DockWidgetHeader",
                                 "KDE::DockWidgetHeaderDrag", "KDE::DockWidgetPrivate"],
                 "KDE::DoubleValidator" => [],
                 "KDE::IntValidator" => [],
                 "KDE::PixmapIO" => [],
                 "KDE::SharedPixmap" => [],
                 "KDE::SystemTray" => [],
                 "KDE::ThemeBase" => ["KDE::ThemeCache", "KDE::ThemePixmap", "KDE::ThemeStyle"],
                 "QXEmbed" => []}}

BLANK_MSG = <<END_OF_STRING 
<b>UISampler</b> - provides examples of <b>Korundum</b> widgets<p>
Select a dialog/widget/menu/etc example from the tree at left
END_OF_STRING


class MainWin < KDE::MainWindow
	TREE_WIDTH = 220
		 
	slots 'lvClicked(QTreeWidgetItem*)'
	
	attr_accessor :edit, :currentPageObj
	
    def initialize(*args)
        super

        setCaption("Samples of Korundum widget usage")
		# The following leave about 375 x 390 for the rt hand panel
		mainGeom  = Qt::Rect.new(0, 0, 640, 500)
        setGeometry(mainGeom)

        # create the main view - list view on the left and an
        # area to display frames on the right
        @mainView  = Qt::Splitter.new(self)
        @tree      = Qt::TreeWidget.new(@mainView)
        @page      = Qt::StackedWidget.new(@mainView)
        blankPage = Qt::Widget.new(@page)
        blankPage.setGeometry(0, 0, 375, 390)
#        blankPage.setBackgroundMode(Qt::Widget::PaletteBase)

        blankLbl = Qt::Label.new(BLANK_MSG, blankPage)
        blankLbl.setGeometry(40, 10, 380, 150)
#        blankLbl.setBackgroundMode(Qt::Widget::PaletteBase)

        blankPM = Qt::Pixmap.new("rbtestimage.png")
        pmLbl   = Qt::Label.new("", blankPage)
        pmLbl.setPixmap(blankPM)
        pmLbl.setGeometry(40, 160, 300, 200)
#        pmLbl.setBackgroundMode(Qt::Widget::PaletteBase)

        @page.insertWidget(1, blankPage)
#        @page.raiseWidget(1)

        setCentralWidget(@mainView)

        initTreeWidget()
        connect(@tree, SIGNAL("itemClicked(QTreeWidgetItem*,int)"), self, SLOT('lvClicked(QTreeWidgetItem*)'))

        @edit = nil
        @currentPageObj = nil
		@prefix = {"Dialogs" => "UIDialogs::dlg", "Widgets" => "UIWidgets::wid", "XML" => "UIXML::xml",
					 "Menus/Toolbars" => "UIMenus::menu", "Other" => "UIMisc::misc"}
	end

    def initTreeWidget()
        @tree.columnCount = 1
        @tree.headerLabels = ["Category"]

#        @tree.addColumn("Category", TREE_WIDTH - 21)
#        tree.setMaximumWidth(treeWidth)
        @mainView.setSizes([TREE_WIDTH, 375])
        @tree.setRootIsDecorated(true)
#        @tree.setVScrollBarMode(Qt::ScrollArea::AlwaysOn)
        topLevel = $listItems.keys()
		topLevel.each do |item_1|
            parent = Qt::TreeWidgetItem.new(@tree, [item_1])
            secondLevel = $listItems[item_1].keys()
			secondLevel.each do |item_2|
                child = Qt::TreeWidgetItem.new(parent, [item_2])
				$listItems[item_1][item_2].each do |item_3|
                    Qt::TreeWidgetItem.new(child, [item_3])
				end
			end
		end
	end

    def lvClicked(lvItem)
        if lvItem.nil?
            return
		end
puts "In lvClicked: %s" % lvItem.text(0)
        if $listItems.keys().include?(lvItem.text(0))
            return
		end

        p = lvItem.parent()
        if $listItems.keys().include?(p.text(0))
            pfx = @prefix[p.text(0)]
            funcCall = pfx + lvItem.text(0).sub("KDE::","K") + "(self)"
        else
            pfx = @prefix[p.parent().text(0)]
            funcCall = pfx + lvItem.parent().text(0).sub("KDE::","K") + "(self)"
		end
        eval funcCall
	end

    def addPage()
        @edit = nil
        @currentPageObj = nil
        current = @page.widget(2)
        if current
            @page.removeWidget(current)
            current.dispose
		end

        newPage = Qt::Widget.new(@page)
        newPage.setGeometry(0, 0, 375, 390)
#        newPage.setBackgroundMode(QWidget.PaletteBase)
        @page.insertWidget(2, newPage)
        @page.currentWidget = newPage

        return newPage
	end
end

#-------------------- main ------------------------------------------------
# Qt::debug_level = Qt::DebugLevel::High
appName = "UISampler"
about = KDE::AboutData.new("uisampler", appName, KDE.ki18n("KDE UI Widgets Sampler"), "0.1")
KDE::CmdLineArgs.init(ARGV, about)
app = KDE::Application.new
mainWindow = MainWin.new(nil)
mainWindow.show
app.exec


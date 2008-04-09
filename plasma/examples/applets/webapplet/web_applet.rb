
require 'plasma_applet'

class WebApplet < Plasma::Applet

  slots 'dataUpdated(QString,Plasma::DataEngine::Data)',
        'load(QUrl)',
        'setHtml(QByteArray)',
        'loadHtml(QUrl)',
        'loadDone(bool)'

  def initialize(parent, args)
    super
  end

  def init
    resize(150, 150)

    @page = Plasma::WebContent.new(self)
    @page.page = Qt::WebPage.new(@page)
    @page.page.linkDelegationPolicy = Qt::WebPage::DelegateAllLinks
    @page.page.settings.setAttribute(Qt::WebSettings::LinksIncludedInFocusChain, true)

    connect(@page, SIGNAL('loadDone(bool)'), self, SLOT('loadDone(bool)'))
    connect(@page.page, SIGNAL('linkClicked(QUrl)'), self, SLOT('load(QUrl)'))

    @page.mainFrame.setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff)
    @page.mainFrame.setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff)

    @page.url = Qt::Url.new("http://dot.kde.org/")

    return true
  end

  def paintInterface(p, option, rect)
  end

  def load(url)
    puts "Loading"
    @page.url = url
  end

  def view
    @page
  end

  def loadDone(success)
    puts "page loaded"
  end

  def contentSizeHint
    if @page
        return @page.sizeHint
    end
    super
  end

  def constraintsUpdated(constraints)
    if constraints.to_i & Plasma::SizeConstraint.to_i
      @page.resize(size())
    end
  end

  def setHtml(html, baseUrl = Qt::Url.new)
    puts "loading"
    @page.mainFrame.setHtml(html, baseUrl)
  end

  def loadHtml(url = Qt::Url.new)
    puts "loading"
    @page.mainFrame.load(url)
  end

end

=begin
/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 * Copyright 2007 Aaron Seigo <aseigo@kde.org
 *
 * Translated to Ruby by Richard Dale
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

=end

require 'plasma_applet'

class FullView < Qt::GraphicsView
  slots 'sceneRectChanged(QRectF)',
        'resizeEvent(QResizeEvent)'

  def initialize(ff = "planar", loc = "floating", parent = nil)
    super()
    @formfactor = Plasma::Planar
    @location = Plasma::Floating
    @containment = nil
    @applet = nil
    @corona = Plasma::Corona.new

    setFrameStyle(Qt::Frame::NoFrame)
    formfactor = ff.downcase
    if formfactor.empty? || formfactor == "planar"
        @formfactor = Plasma::Planar
    elsif formfactor == "vertical"
        @formfactor = Plasma::Vertical
    elsif formfactor == "horizontal"
        @formfactor = Plasma::Horizontal
    elsif formfactor == "mediacenter"
        @formfactor = Plasma::MediaCenter
    end

    location = loc.downcase
    if loc.empty? || loc == "floating"
        @location = Plasma::Floating
    elsif loc == "desktop"
        @location = Plasma::Desktop
    elsif loc == "fullscreen"
        @location = Plasma::FullScreen
    elsif loc == "top"
        @location = Plasma::TopEdge
    elsif loc == "bottom"
        @location = Plasma::BottomEdge
    elsif loc == "right"
        @location = Plasma::RightEdge
    elsif loc == "left"
        @location = Plasma::LeftEdge
    end

    setScene(@corona)
    connect(@corona, SIGNAL('sceneRectChanged(QRectF)'),
            self, SLOT('sceneRectChanged(QRectF)'))
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff)
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff)
    setAlignment(Qt::AlignLeft | Qt::AlignTop)
  end

  def addApplet(a, args)
    @containment = @corona.addContainment("null")
    @containment.formFactor = @formfactor
    @containment.location = @location
    @applet = @containment.addApplet(a, args, Qt::RectF.new(0, 0, -1, -1))
    @applet.setFlag(Qt::GraphicsItem::ItemIsMovable, false)

    setSceneRect(@applet.geometry)
    setWindowTitle(@applet.name)
    setWindowIcon(Qt::Icon.new(KDE::SmallIcon(@applet.icon)))
  end

  def resizeEvent(event)
    if @applet.nil?
        puts "no applet"
        return
    end

    # The applet always keeps its aspect ratio, so let's respect it.
    newWidth = 0
    newHeight = 0

    if @applet.aspectRatioMode == Qt::KeepAspectRatio
      ratio = event.oldSize.width / event.oldSize.height
      newPossibleWidth = size.height * ratio
      if newPossibleWidth > size.width
        newHeight = size.width / ratio
        newWidth = newHeight * ratio
      else
        newWidth = newPossibleWidth
        newHeight = newWidth / ratio
      end
    else
      newWidth = size.width
      newHeight = size.height
    end

    @containment.resize(Qt::SizeF.new(size()))
    @applet.resize(Qt::SizeF.new(newWidth, newHeight))
  end

  def sceneRectChanged(rect)
    if @applet
      setSceneRect(@applet.geometry)
    end
  end
end

description = I18N_NOOP( "Run Plasma applets in their own window" )
version = "1.0"
aboutData = KDE::AboutData.new( "plasmoidviewer", nil, KDE::ki18n( "Plasma Applet Viewer" ),
                                version, KDE::ki18n( description ), KDE::AboutData::License_BSD,
                                KDE::ki18n( "(C) 2007, The KDE Team" ) )
aboutData.addAuthor( KDE::ki18n( "Frerich Raabe" ),
                     KDE::ki18n( "Original author" ),
                     "raabe@kde.org" )

KDE::CmdLineArgs.init( ARGV, aboutData )

options = KDE::CmdLineOptions.new
options.add( "f" )
options.add( "formfactor <name>", KDE::ki18n( "The formfactor to use (horizontal, vertical, mediacenter or planar)" ), "planar")
options.add( "l" )
options.add( "location <name>", KDE::ki18n( "The location constraint to start the Containment with (floating, desktop, fullscreen, top, bottom, left, right)" ), "floating")
options.add( "p" )
options.add( "pixmapcache <size>", KDE::ki18n("The size in KB to set the pixmap cache to"))
options.add( "!+applet", KDE::ki18n( "Name of applet to add (required)" ) )
options.add( "+[args]", KDE::ki18n( "Optional arguments of the applet to add" ) )
KDE::CmdLineArgs.addCmdLineOptions( options )

app = KDE::Application.new

args = KDE::CmdLineArgs.parsedArgs

if args.count == 0
  KDE::CmdLineArgs.usageError(KDE::i18n("No applet name specified"))
end

formfactor = "planar"
if args.isSet("formfactor")
  puts "setting FormFactor to #{args.getOption("formfactor")}"
  formfactor = args.getOption("formfactor")
end

location = "floating"
if args.isSet("location")
  puts "setting Location to #{args.getOption("location")}"
  location = args.getOption("location")
end

appletArgs = []
for i in 1...args.count do
  appletArgs << Qt::Variant.new(args.arg(i))
end

view = FullView.new( formfactor, location )
# At this point arg(0) is always set
view.addApplet( args.arg(0), appletArgs )
view.show

action = KDE::StandardAction.quit(app, SLOT(:quit), view)
view.addAction(action)

if args.isSet("pixmapcache")
  puts "setting pixmap cache to #{args.getOption("pixmapcache")}"
  Qt::PixmapCache.cacheLimit = args.getOption("pixmapcache").to_i
end

app.exec



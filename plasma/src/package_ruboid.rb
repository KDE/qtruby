require 'plasma_applet'

module RubyAppletScript
  class PackageRuboid < Plasma::PackageStructure
    def initialize(parent, args)
      super(parent, "Ruboid")
      addDirectoryDefinition("images", "images", KDE::i18n("Images"))
      mimetypes = []
      mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
      setMimetypes("images", mimetypes)

      addDirectoryDefinition("config", "config/", KDE::i18n("Configuration Definitions"))
      mimetypes = []
      mimetypes << "text/xml";
      setMimetypes("config", mimetypes)
      setMimetypes("configui", mimetypes)

      addDirectoryDefinition("ui", "ui", KDE::i18n("Executable Scripts"))
      setMimetypes("ui", mimetypes)

      addDirectoryDefinition("scripts", "code", KDE::i18n("Executable Scripts"))
      mimetypes = []
      mimetypes << "text/*"
      setMimetypes("scripts", mimetypes)

      addFileDefinition("mainconfiggui", "ui/config.ui", KDE::i18n("Main Config UI File"))
      addFileDefinition("mainconfigxml", "config/main.xml", KDE::i18n("Configuration XML file"))
      addFileDefinition("mainscript", "code/main.rb", KDE::i18n("Main Script File"))
      setRequired("mainscript", true)
    end
  end
end
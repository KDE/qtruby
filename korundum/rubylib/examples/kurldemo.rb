=begin
Copyright 2003 Jim Bublitz

Terms and Conditions

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Except as contained in this notice, the name of the copyright holder shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from the
copyright holder.
=end

require 'Korundum'

class MainWin < KDE::MainWindow
    def initialize(*args)
        super

		@urls = ["http://slashdot.org", "http://www.kde.org", "http://www.riverbankcomputing.co.uk", "http://yahoo.com"]
        setGeometry(0, 0, 400, 600)

        edit = KDE::TextEdit.new(self)
        setCentralWidget(edit)

        edit.insert("KURL Demo\n")
        edit.insert("\nAdding these urls:\n\n")
        @urls.each do |url|
            edit.insert("    #{url}\n")
		end

        edit.insert("\nCreating KURLs (iterating to print):\n\n")
        urlList = []
        @urls.each do |url|
            urlList << KDE::URL.new(url)
		end

        urlList.each do |url|
            edit.insert("     #{url.url()}\n")
		end

        edit.insert("\nFirst url -- urlList [0]:\n\n")
        edit.insert("    #{urlList[0].to_s}\n")
        edit.insert("    #{urlList[0].url()}\n")

        edit.insert("\nLast url -- urlList [-1]:\n\n")
        edit.insert("    #{urlList[-1].to_s}\n")
        edit.insert("    #{urlList[-1].url()}\n")

        edit.insert("\nMiddle urls -- urlList [2,4]:\n\n")
        ulist = urlList[2,4]
        ulist.each do |url|
            edit.insert("    #{url.to_s}\n")
            edit.insert("    #{url.url()}\n")
		end

        edit.insert("\nLength of urlList -- len (urlList):\n\n")
        edit.insert("    Length = #{urlList.length}\n")

        edit.insert("\nurl in urlList? -- KURL (\"http://yahoo.com\") in urlList\n\n")
        edit.insert("    KDE::URL.new(\"http://yahoo.com\") in urlList = ")
		tmp = KDE::URL.new("http://yahoo.com")
		urlList.each do |url| 
			if url.url == tmp.url
        		edit.insert("true")
			end
		end
	end
end


#-------------------- main ------------------------------------------------

description = "A basic application template"
version     = "1.0"
aboutData   = KDE::AboutData.new("", "",
    version, description, KDE::AboutData::License_GPL,
    "(C) 2003 whoever the author is")

aboutData.addAuthor("author1", "whatever they did", "email@somedomain")
aboutData.addAuthor("author2", "they did something else", "another@email.address")

KDE::CmdLineArgs.init(ARGV, aboutData)

KDE::CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app = KDE::Application.new()
mainWindow = MainWin.new(nil, "main window")
mainWindow.show()
app.exec()

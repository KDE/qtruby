#!/usr/bin/env ruby
=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

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

module DCOPTest

	def DCOPTest.getAnyApplication(client, appName)
		client.registeredApplications().each do |app|
			if app == appName or app =~ Regexp.new("^#{appName}-")
				puts app
				puts
				objList = client.remoteObjects(app)
				objList.each do |obj|
					puts " #{obj}"
					funcs = client.remoteFunctions(app, obj)
					funcs.each { |f| puts "  #{f}" }
				end
			end
		end
	end

end

#-------------------- main ------------------------------------------------

description = "A basic application template"
version     = "1.0"
aboutData   = KDE::AboutData.new("testdcopext", "testdcopext",
    version, description, KDE::AboutData::License_GPL,
    "(C) 2003 whoever the author is")

aboutData.addAuthor("author1", "whatever they did", "email@somedomain")
aboutData.addAuthor("author2", "they did something else", "another@email.address")

KDE::CmdLineArgs.init(ARGV, aboutData)

KDE::CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app   = KDE::Application.new
dcop  = app.dcopClient
puts "DCOP Application: #{dcop.appId} starting"

# DCOPTest.getAnyApplication(dcop, "konqueror")

puts "--------------------------"
puts "The DCOPObjects for kicker:"

d = KDE::DCOPRef.new('kicker')
objs = d.interfaces()
objs.each { |obj| puts obj }

puts
puts "get kicker panel size via DCOP"
res = d.panelSize()
puts res
puts "--------------------------"

puts
puts "Call a method that doesn't exist"
res = d.junk()
puts (res.nil? ? "Call failed" : "Call succeeded")
puts "--------------------------"

puts
puts
puts "Start a kwrite instance"

error = ""
dcopService = ""
pid = Qt::Integer.new

errcode = KDE::Application.startServiceByDesktopName("kwrite", "", error, dcopService, pid)
dcopService = "kwrite-" + pid.to_s
puts "errcode: #{errcode.to_s}   error: #{error}   dcopService: #{dcopService}  pid: #{pid.to_s}"
puts "--------------------------"
sleep(2)

o1 = KDE::DCOPRef.new(dcopService, "EditInterface#1")
#puts "Check if insertLine is a valid function"
#puts "valid", o1.insertLine.valid
#puts "--------------------------"
#puts "insertLine's arg types and names"
# puts o1.insertLine.argtypes,  o1.insertLine.argnames
puts "--------------------------"
puts "Insert a line into the kwrite instance we launched"

res = o1.insertLine(0, 'Now is the time for all good men to come to the aid of their party')
if res.nil?
	puts "call returns: failed"
else
	puts "call returns: #{res.to_s}"
end

app.exec



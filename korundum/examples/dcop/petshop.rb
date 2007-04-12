#!/usr/bin/env ruby

# This is an example of a DCOP enabled application written in Ruby, using
# Korundum. Taken from the PyKDE example_dcopexport.py example which was
# derived from server.py example in kdebindings written by Torben Weis 
# and Julian Rockey

require 'Korundum'

# An object with DCOP slots needn't be a subclass of DCOPObject, but
# this DeadParrotObject is one 

class DeadParrotObject < KDE::DCOPObject

	k_dcop	'QString getParrotType()', 'void setParrotType(QString)', 
			'QString squawk()', 'QStringList adjectives()',
			'int age()', 'void setAge(int)'
	
	def initialize(id = 'dead parrot')
		super(id)
		@parrot_type = "Norwegian Blue"
		@age = 7
	end
	
	def getParrotType()
		@parrot_type
	end
	
	def setParrotType(parrot_type)
		@parrot_type = parrot_type
	end
	
	def age()
		@age
	end
	
	def setAge(a)
		@age = a
	end
			
	def squawk
		if rand(2) == 0
			"This parrot, a #{@parrot_type}, is pining for the fjords"
		else
			"This parrot, #{@age} months old, is a #{@parrot_type}"
		end
	end
	
	def adjectives
		return	["passed on", "is no more", "ceased to be", "expired", "gone to meet his maker",
                 "a stiff", "bereft of life", "rests in peace", "metabolic processes are now history",
                 "off the twig", "kicked the bucket", "shuffled off his mortal coil",
                 "run down his curtain", "joined the bleedin' choir invisible", "THIS IS AN EX-PARROT"]
	end
end

description = "A basic application template"
version     = "1.0"
aboutData   = KDE::AboutData.new("petshop", "Dead Parrot Test",
    version, description, KDE::AboutData::License_GPL,
    "(C) 2003 whoever the author is")

aboutData.addAuthor("author1", "whatever they did", "email@somedomain")
aboutData.addAuthor("author2", "they did something else", "another@email.address")

KDE::CmdLineArgs.init(ARGV, aboutData)

KDE::CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app   = KDE::UniqueApplication.new
dcop  = app.dcopClient
puts "DCOP Application: #{dcop.appId} starting"

parrot         = DeadParrotObject.new
another_parrot = DeadParrotObject.new('polly')

message = <<EOS
Run kdcop and look for the 'petshop' application instance.

This program exports the DeadParrotObject object.
Double-clicking those object's methods will allow you to get or set data.

To end the application, in kdcop choose the MainApplication-Interface
object and double-click the quit() method.
EOS

print message

app.exec



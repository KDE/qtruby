#!/usr/bin/ruby

# Example of use of the Korundum soprano bindings, trying to mimic sopranocmd.
# Translated from Python to Ruby by Richard Dale

# Copyright (c) 2009 Olivier Berger + Institut Télécom, released under
# the BSD license.
# Copyright (c) 2009 Richard Dale

require 'Qt4'
require 'soprano'
require 'getoptlong'

opts = GetoptLong.new(
  [ "--help", "-H", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--backend", "-b", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--dir", "-d", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--port", "-p", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--host", "-h", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--username", "-U", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--password", "-P", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--settings", "-S", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--serialization", "-s", GetoptLong::REQUIRED_ARGUMENT ]
)

backendOpt = dirOpt = portOpt = hostOpt = usernameOpt = passwordOpt = settingsOpt = serializationOpt = nil

opts.each do |opt, arg|
  case opt
  when '--help'
  when '--backend'
    backendOpt = arg
  when '--dir'
    directoryOpt = arg
  when '--port'
    portOpt = arg.to_i
  when '--host'
    hostOpt = arg
  when '--username'
    usernameOpt = arg
  when '--password'
    passwordOpt = arg
  when '--settings'
    settingsOpt = arg
  when '--serialization'
    serializationOpt = arg
  end
end

command = ARGV[0]

if !['query', 'list'].include? command
  $stderr.puts "Error: unknown command #{command}"
  exit 1
end

if command == 'query'
  query = ARGV[1]
end

backend = Soprano.discoverBackendByName(backendOpt)

settings = []
settings << Soprano::BackendSetting.new(Soprano::BackendOptionHost, hostOpt)
settings << Soprano::BackendSetting.new(Soprano::BackendOptionPort, portOpt)
settings << Soprano::BackendSetting.new(Soprano::BackendOptionUsername, usernameOpt)
settings << Soprano::BackendSetting.new(Soprano::BackendOptionPassword, passwordOpt)
model = backend.createModel(settings)

if command == 'query'
  result = model.executeQuery(query, Soprano::Query::QueryLanguageSparql)

  result.each do |binding_set|
    binding_set.bindingNames.each do |name|
      puts "#{binding_set.value(name).inspect}"
    end
  end
elsif command == 'list'
  statements = model.listStatements
  statements.each do |statement|
    puts "#{statement.subject.toN3} - #{statement.predicate.toN3} - #{statement.object.toN3}"
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;

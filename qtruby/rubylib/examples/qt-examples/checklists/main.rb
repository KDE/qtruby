#!/usr/bin/env ruby

require 'Qt'

require 'checklists'

a = Qt::Application.new(ARGV)

checklists = CheckLists.new
checklists.resize(650, 350)
checklists.setCaption('QtRuby Example - CheckLists')
a.setMainWidget(checklists)
checklists.show

a.exec()

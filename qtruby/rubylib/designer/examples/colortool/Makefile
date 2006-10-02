RBUIC=rbuic

RBUICIMPLS=mainform.rb optionsform.rb colornameform.rb \
	findform.rb qmake_image_collection.rb

all: $(RBUICIMPLS)

mainform.rb:
	$(RBUIC) mainform.ui -o mainform.rb

optionsform.rb: optionsform.ui
	$(RBUIC) optionsform.ui -o optionsform.rb

colornameform.rb: colornameform.ui
	$(RBUIC) colornameform.ui -o colornameform.rb

findform.rb: findform.ui
	$(RBUIC) findform.ui -o findform.rb

qmake_image_collection.rb:
	$(RBUIC) -embed colortool images/filenew.png images/fileopen.png \
	images/filesave.png images/editcut.png images/editcopy.png \
	images/searchfind.png images/tabwidget.png images/table.png \
	images/iconview.png images/richtextedit.png images/widgetstack.png \
	images/editraise.png -o qmake_image_collection.rb

clean:
	rm -f $(RBUICIMPLS)

	

require 'Qt'

class CheckLists < Qt::Widget
	slots 'copy1to2()', 'copy2to3()'

	# Constructor
	#
	# Create all child widgets of the CheckList Widget
	def initialize
		super()

		lay = Qt::HBoxLayout.new(self)
		lay.setMargin(5)

		# create a widget which layouts its childs in a column
		vbox1 = Qt::VBoxLayout.new(lay)
		vbox1.setMargin(5)

		# First child: a Label
		vbox1.addWidget(Qt::Label.new('Check some items!', self))

		# Second child: the ListView
		@lv1 = Qt::ListView.new(self)
		vbox1.addWidget(@lv1)
		@lv1.addColumn('Items')
		@lv1.setRootIsDecorated(true)

		# create a list with 4 ListViewItems which will be parent items of other ListViewItems
		parentList = Array.new


		parentList.push(Qt::ListViewItem.new(@lv1, 'Parent Item 1'))
		parentList.push(Qt::ListViewItem.new(@lv1, 'Parent Item 2'))
		parentList.push(Qt::ListViewItem.new(@lv1, 'Parent Item 3'))
		parentList.push(Qt::ListViewItem.new(@lv1, 'Parent Item 4'))

		item = 0
		num = 1
		# go through the list of parent items...
		parentList.each {|item|
			item.setOpen(true)
			# ...and create 5 checkable child ListViewItems for each parent item
			for i in 1..5
				str = sprintf('%s. Child of Parent %s', i, num)
				Qt::CheckListItem.new(item, str, Qt::CheckListItem.CheckBox)
			end
			num = num + 1
		}

		# Create another widget for layouting
		tmp = Qt::VBoxLayout.new(lay)
		tmp.setMargin(5)

		# create a pushbutton
		copy1 = Qt::PushButton.new('  ->  ', self)
		tmp.addWidget(copy1)
		copy1.setMaximumWidth(copy1.sizeHint.width)
		# connect the SIGNAL clicked() of the pushbutton with the SLOT copy1to2()
		connect(copy1, SIGNAL('clicked()'), self, SLOT('copy1to2()'))

		# another widget for layouting
		vbox2 = Qt::VBoxLayout.new(lay)
		vbox2.setMargin(5)

		# and another label
		vbox2.addWidget(Qt::Label.new('Check one item!', self))

		# create the second listview
		@lv2 = Qt::ListView.new(self)
		vbox2.addWidget(@lv2)
		@lv2.addColumn('Items')
		@lv2.setRootIsDecorated(true)

		# another widget needed for layouting only
		tmp = Qt::VBoxLayout.new(lay)
		tmp.setMargin(5)

		# create another pushbutton...
		copy2 = Qt::PushButton.new('  ->  ', self)
		lay.addWidget( copy2 )
		copy2.setMaximumWidth(copy2.sizeHint.width)
		# ...and connect its clicked() SIGNAL to the copy2to3() SLOT
		connect(copy2, SIGNAL('clicked()'), self, SLOT('copy2to3()'))

		tmp = Qt::VBoxLayout.new(lay)
		tmp.setMargin(5)

		# and create a label which will be at the right of the window
		@label = Qt::Label.new('No Item yet...', self)
		tmp.addWidget(@label)
	end

	# SLOT copy1to2()
	#
	# Copies all checked ListViewItems from the first ListView to
	# the second one, and inserts them as Radio-ListViewItem.
	def copy1to2
		@lv2.clear
		it = Qt::ListViewItemIterator.new(@lv1)
		# Insert first a controller Item into the second ListView. Always if Radio-ListViewItems
		# are inserted into a Listview, the parent item of these MUST be a controller Item!
		item = Qt::CheckListItem.new(@lv2, 'Controller', Qt::CheckListItem::Controller );
		item.setOpen(true);

		# iterate through the first ListView...
		while (it.current)
			# ...check state of childs, and...
			if ( it.current.parent )
				#  ...if the item is checked...
				if (it.current.isOn)
					# ...insert a Radio-ListViewItem with the same text into the second ListView
					Qt::CheckListItem.new(item, it.current.text(0), Qt::CheckListItem::RadioButton)
				end
			end
			it += 1
		end

		if (item.firstChild)
			item.firstChild.setOn(true)
		end
	end


	# SLOT copy2to3()
	#
	# Copies the checked item of the second ListView into the
	# Label at the right.
	def copy2to3
		# create an iterator which operates on the second ListView
		it = Qt::ListViewItemIterator.new(@lv2)

		@label.setText('No Item checked')

		# iterate through the second ListView...
		while (it.current)
			# ...check state of childs, and...
			if ( it.current.parent)
				# ...if the item is checked...
				if (it.current.isOn)
					# ...set the text of the item to the label
					@label.setText(it.current.text(0))
				end
			end
			it += 1
		end
	end
end

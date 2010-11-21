=begin
    Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with self library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
=end

class MainWidget < Qt::Widget

  slots 'collectionClicked(Akonadi::Collection)',
        'itemActivated(QModelIndex)',
        'itemFetchDone(job)',
        'threadCollection()'

  def initialize(parent = nil)
    super(parent)
    @mainWindow = parent
    connect( @mainWindow, SIGNAL(:threadCollection),
             self, SLOT(:threadCollection) )

    layout = Qt::HBoxLayout.new(self)

    splitter = Qt::Splitter.new(Qt::Horizontal, self)
    layout.addWidget(splitter)

    # Left part, collection view
    @collectionList = Akonadi::CollectionView.new
    connect( @collectionList, SIGNAL('clicked(Akonadi::Collection)'),
             SLOT('collectionClicked(Akonadi::Collection)') )
    collectionDelegate = Akonadi::CollectionStatisticsDelegate.new(@collectionList)
    collectionDelegate.unreadCountShown = true  #For testing, should be toggleable columns eventually
    @collectionList.itemDelegate = collectionDelegate
    splitter.addWidget(@collectionList)
    # Filter the collection to only show the emails
    @collectionModel = Akonadi::CollectionStatisticsModel.new(self)
    @collectionProxyModel = Akonadi::CollectionFilterProxyModel.new(self) do |m|
      m.sourceModel = @collectionModel
      m.addMimeTypeFilter("message/rfc822")
    end

    # display collections sorted
    sortModel = Qt::SortFilterProxyModel.new(self) do |s|
      s.dynamicSortFilter = true
      s.sortCaseSensitivity = Qt::CaseInsensitive
      s.sourceModel = @collectionProxyModel
    end

    # Right part, message list + message viewer
    rightSplitter = Qt::Splitter.new(Qt::Vertical, self)
    splitter.addWidget( rightSplitter )
    @messageList = Qt::TreeView.new(self) do |l|
      l.dragEnabled = true
      l.selectionMode = Qt::AbstractItemView::ExtendedSelection
    end
    connect(@messageList, SIGNAL('clicked(QModelIndex)'), SLOT('itemActivated(QModelIndex)'))
    rightSplitter.addWidget(@messageList)

    @collectionList.model = sortModel
    @messageModel = Akonadi::MessageModel.new(self)
    @messageProxyModel = Akonadi::MessageThreaderProxyModel.new(self)
    @messageProxyModel.sourceModel = @messageModel
    @messageList.model = @messageProxyModel

    @messageView = Qt::TextEdit.new(self)
    rightSplitter.addWidget(@messageView)

    splitter.sizes = [200, 500]
    rightSplitter.sizes = [300, 200]
  end

  def collectionClicked(collection)
    @currentCollection = collection
    @messageModel.collection = Akonadi::Collection.new(@currentCollection)
  end

  def itemActivated(index)
    item = @messageModel.itemForIndex(@messageProxyModel.mapToSource(index))

    if !item.valid?
      return
    end

    job = Akonadi::ItemFetchJob.new(item, self)
    job.fetchScope.fetchFullPayload
    connect(job, SIGNAL('result(KJob*)'), SLOT('itemFetchDone(KJob*)'))
    job.start()
  end

  def itemFetchDone(job)
    fetch = job
    if job.error
      puts "Mail fetch failed: #{job.errorString}"
    elsif fetch.items.empty?
      puts "No mail found!"
    else
      item = fetch.items.first
      @messageView.plainText = item.payloadData
    end
  end

  def threadCollection
    return if @currentCollection.nil?
    a = @currentCollection.attribute(Akonadi::Collection::AddIfMissing)
    a.deserialize(Qt::ByteArray.new("sort"))
    job = Akonadi::CollectionModifyJob.new(@currentCollection)
    if !job.exec
      puts "Unable to modify collection"
    end
  end
end




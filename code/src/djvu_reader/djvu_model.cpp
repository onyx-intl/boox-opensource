#include "djvu_model.h"

namespace djvu_reader
{

DjvuModel::DjvuModel(const QString & program_name)
    : BaseModel()
    , need_save_bookmarks_(false)
    , is_ready_(false)
    , doc_(program_name, true)
{
    connect(&doc_, SIGNAL(error(QString, QString, int)), SIGNAL(docError(QString, QString, int)));
    connect(&doc_, SIGNAL(info(QString)), SIGNAL(docInfo(QString)));
    connect(&doc_, SIGNAL(docInfo()), SIGNAL(docReady()));
    connect(&doc_, SIGNAL(pageInfo()), SIGNAL(docPageReady()));
    connect(&doc_, SIGNAL(thumbnail(int)), SIGNAL(docThumbnailReady(int)));
    connect(&doc_, SIGNAL(idle()), SIGNAL(docIdle()));
}

DjvuModel::~DjvuModel()
{
    close();
}

bool DjvuModel::open(const QString & path)
{
    if (!path.isEmpty())
    {
        doc_.setFileName(path);
        if (doc_.isValid())
        {
            path_     = path;
            is_ready_ = true;
            loadOptions();
            return true;
        }
    }
    return false;
}

bool DjvuModel::close()
{
    if (!is_ready_)
    {
        qDebug("The model has been closed!");
        return true;
    }

    conf_.options.clear();

    // reset flag
    is_ready_ = false;
    return true;
}

bool DjvuModel::save()
{
    if (!isReady())
    {
        return false;
    }

    emit requestSaveAllOptions();
    saveOptions();
    return true;
}

bool DjvuModel::openCMS()
{
    if (!content_manager_.isOpen())
    {
        vbf::openDatabase(path_, content_manager_);
    }
    return content_manager_.isOpen();
}

bool DjvuModel::loadOptions()
{
    bool ret = openCMS();
    if (ret)
    {
        ret = vbf::loadDocumentOptions(content_manager_, path_, conf_);
    }

    ret = loadBookmarks();
    return ret;
}

bool DjvuModel::saveOptions()
{
    bool ret = openCMS();
    if (ret)
    {
        ret = vbf::saveDocumentOptions(content_manager_, path_, conf_);
    }

    ret = saveBookmarks();
    return ret;
}

bool DjvuModel::isTheDocument(const QString &path)
{
    return ( path_ == path );
}

bool DjvuModel::metadata(const MetadataTag tag, QVariant &value)
{
    return false;
}

QImage DjvuModel::getThumbnail(const int width, const int height)
{
    return QImage();
}

int DjvuModel::firstPageNumber()
{
    return 0;
}

int DjvuModel::getPagesTotalNumber()
{
    return doc_.getPageCount();
}

shared_ptr<ddjvu_pageinfo_t> DjvuModel::getPageInfo(int page_no)
{
    shared_ptr<ddjvu_pageinfo_t> page_info(new ddjvu_pageinfo_t);
    if (doc_ != 0 && isReady())
    {
        ddjvu_status_t status = ddjvu_document_get_pageinfo(doc_, page_no, page_info.get());
        if (status != DDJVU_JOB_OK)
        {
            return shared_ptr<ddjvu_pageinfo_t>();
        }
    }
    return page_info;
}

bool DjvuModel::saveBookmarks()
{
    bool ret = true;
    if (need_save_bookmarks_)
    {
        ret = vbf::saveBookmarks(content_manager_, path(), bookmarks_);
    }
    if (ret)
    {
        need_save_bookmarks_ = false;
    }
    return ret;
}

bool DjvuModel::loadBookmarks()
{
    need_save_bookmarks_ = false;
    bookmarks_.clear();
    return vbf::loadBookmarks(content_manager_, path(), bookmarks_);
}

QString DjvuModel::getPageText(int page_no)
{
    bool entity_existed = false;
    PageTextEntities & entities = QDjVuPage::pageTextEntities(page_no, entity_existed);
    if (!entity_existed)
    {
        miniexp_t page_text = doc_.getPageText(page_no);
        if (page_text != miniexp_nil)
        {
            shared_ptr<ddjvu_pageinfo_t> page_info = getPageInfo(page_no);
            int height = page_info->height;

            QString granularity("line");
            QQueue<miniexp_t> queue;
            queue.enqueue(page_text);
            while (!queue.isEmpty())
            {
                miniexp_t cur = queue.dequeue();
                if ( miniexp_listp( cur )
                     && ( miniexp_length( cur ) > 0 )
                     && miniexp_symbolp( miniexp_nth( 0, cur ) ) )
                {
                    int size = miniexp_length( cur );
                    QString sym = QString::fromUtf8( miniexp_to_name( miniexp_nth( 0, cur ) ) );
                    qDebug("Symbol:%s", qPrintable(sym));
                    if ( sym == granularity )
                    {
                        if ( size >= 6 )
                        {
                            TextEntityPtr text_entity(new TextEntity());
                            int xmin = miniexp_to_int( miniexp_nth( 1, cur ) );
                            int ymin = miniexp_to_int( miniexp_nth( 2, cur ) );
                            int xmax = miniexp_to_int( miniexp_nth( 3, cur ) );
                            int ymax = miniexp_to_int( miniexp_nth( 4, cur ) );
                            QRect rect( xmin, height - ymax, xmax - xmin, ymax - ymin );
                            text_entity->area = rect;
                            text_entity->text = QString::fromUtf8( miniexp_to_str( miniexp_nth( 5, cur ) ) );
                            entities.push_back(text_entity);
                        }
                    }
                    else
                    {
                        for ( int i = 5; i < size; ++i )
                        {
                            queue.enqueue( miniexp_nth( i, cur ) );
                        }
                    }
                }
            }
        }
    }

    QString ret;
    if (!entities.isEmpty())
    {
        PageTextEntities::iterator idx = entities.begin();
        TextEntityPtr entity = *idx;
        do
        {
            ret = entity->text;
            idx++;
        } while (ret.isEmpty() && idx != entities.end());
    }
    return ret;
}

struct LessByPosition
{
    bool operator()( const Bookmark& a, const Bookmark& b ) const
    {
        return a.data().toInt() < b.data().toInt();
    }
};

bool DjvuModel::addBookmark(const int page_start, const int page_end)
{
    if (hasBookmark(page_start, page_end))
    {
        return false;
    }

    QString title = getPageText(page_start);
    if (title.isEmpty())
    {
        // When the title is empty.
        QString format(tr("Bookmark %1"));
        title = format.arg(bookmarks_.size() + 1);
    }
    else
    {
        title = title.trimmed();
        if (title.size() > 100)
        {
            title = title.left(97);
            title.append("...");
        }
    }

    // Insert into bookmarks list.
    need_save_bookmarks_ = insertBookmark( bookmarks_,
                                           Bookmark( title, page_start ),
                                           LessByPosition());
    return need_save_bookmarks_;
}

bool DjvuModel::deleteBookmark(const int page_start, const int page_end)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            iter = bookmarks_.erase(iter);
            need_save_bookmarks_ = true;
            break;
        }
    }
    return need_save_bookmarks_;
}

bool DjvuModel::hasBookmark(const int page_start, const int page_end)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            return true;
        }
    }
    return false;
}

bool DjvuModel::updateBookmark(const int page_start, const int page_end, const QString & name)
{
    if (name.isEmpty())
    {
        return false;
    }

    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            (*iter).mutable_title() = name;
            need_save_bookmarks_ = true;
            break;
        }
    }
    return need_save_bookmarks_;
}

QString DjvuModel::getFirstBookmarkTitle(const int page_start, const int page_end)
{
    QString title;
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            title = iter->title();
            break;
        }
    }
    return title;
}

void DjvuModel::getBookmarksModel(QStandardItemModel & bookmarks_model)
{
    bookmarks_model.setColumnCount(2);
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    int row = 0;
    for(BookmarksIter iter  = begin; iter != end; ++iter, ++row)
    {
        // page number
        int loc = iter->data().toInt();
        if (loc >= 0)
        {
            // title
            QStandardItem *title = new QStandardItem( iter->title() );
            title->setData( iter->data() );
            title->setEditable( false );
            bookmarks_model.setItem( row, 0, title );

            // page number
            int page_number = loc + 1;
            QString str( tr("%1") );
            str = str.arg(page_number);
            QStandardItem *page = new QStandardItem(str);
            page->setEditable(false);
            page->setTextAlignment(Qt::AlignCenter);
            bookmarks_model.setItem(row, 1, page);
        }
    }

    bookmarks_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
    bookmarks_model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
}

bool DjvuModel::hasOutlines()
{
    miniexp_t outline = doc_.getDocumentOutline();
    if (miniexp_listp(outline) &&
        (miniexp_length(outline) > 0) &&
        miniexp_symbolp(miniexp_nth(0, outline)) &&
        (QString::fromUtf8(miniexp_to_name(miniexp_nth(0, outline))) == QLatin1String("bookmarks")))
    {
        return true;
    }
    return false;
}

QStandardItemModel* DjvuModel::getOutlineModel()
{
    if (outline_model_ != 0)
    {
        return outline_model_.get();
    }

    if (hasOutlines())
    {
        miniexp_t outline = doc_.getDocumentOutline();
        outline_model_.reset( new QStandardItemModel() );
        QStandardItem *root = outline_model_->invisibleRootItem();
        loadOutlineItem(root, outline, 1);

        // set header data
        outline_model_->setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
        outline_model_->setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
        return outline_model_.get();
    }
    return 0;
}

QString DjvuModel::getDestByTOCIndex(const QModelIndex & index)
{
    if (outline_model_ == 0)
    {
        return QString();
    }

    QStandardItem *item = outline_model_->itemFromIndex( index );
    QString dest = item->data(OUTLINE_ITEM).toString();
    return dest;
}

void DjvuModel::loadOutlineItem(QStandardItem * parent, miniexp_t exp, int offset)
{
    if (!miniexp_listp(exp))
    {
        return;
    }

    int len = miniexp_length(exp);
    for ( int i = qMax(offset, 0); i < len; ++i )
    {
        miniexp_t cur = miniexp_nth(i, exp);

        if (miniexp_consp(cur) &&
            (miniexp_length(cur) > 0) &&
            miniexp_stringp(miniexp_nth(0, cur)) &&
            miniexp_stringp(miniexp_nth(1, cur)))
        {
            QString title = QString::fromUtf8( miniexp_to_str( miniexp_nth( 0, cur ) ) );
            QString dest = QString::fromUtf8( miniexp_to_str( miniexp_nth( 1, cur ) ) );

            if (dest.isEmpty() ||
                ((dest.at(0) == QLatin1Char('#')) && (dest.remove(0, 1) != title)))
            {
                QStandardItem *model_item = new QStandardItem(title);
                model_item->setData(dest, OUTLINE_ITEM);

                QStandardItem *page_item = new QStandardItem(dest);
                page_item->setTextAlignment( Qt::AlignCenter );
                page_item->setData(dest, OUTLINE_ITEM);

                int row_count = parent->rowCount();
                parent->appendRow( model_item );
                if (page_item != 0)
                {
                    parent->setChild( row_count, 1, page_item );
                }

                if (miniexp_length( cur ) > 2)
                {
                    loadOutlineItem(model_item, cur, 2);
                }
            }
        }
    }
}

}

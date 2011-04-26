#include <time.h>

#include <core/decoder.h>
#include <core/decoderfactory.h>
#include <core/fileinfo.h>

#include "fileloader.h"
#include "playlistmodel.h"
#include "playlistitem.h"
#include "playstate.h"
#include "playlistsettings.h"

namespace player
{

#define INVALID_ROW -1

TagUpdater::TagUpdater(QObject* o, PlayListItem* item)
    : observable_(o)
    , item_(item)
{
    item_->setFlag(PlayListItem::EDITING);
    connect(observable_, SIGNAL(destroyed(QObject *)),SLOT(updateTag()));
    connect(observable_, SIGNAL(destroyed(QObject *)),SLOT(deleteLater()));
}

void TagUpdater::updateTag()
{
    if (item_->flag() == PlayListItem::SCHEDULED_FOR_DELETION)
    {
        delete item_;
        item_ = NULL;
    }
    else
    {
        item_->updateTags();
        item_->setFlag(PlayListItem::FREE);
    }
}


PlayListModel::PlayListModel(QObject *parent)
    : QObject(parent)
    , selection_()
    , play_state_(new NormalPlayState(this))
    , play_list_model_(0)
{
    qsrand(time(0));
    shuffle_ = false;
    total_length_ = 0;
    current_ = 0;
    block_update_signals_ = false;
    is_repeatable_list_ = true;
    readSettings();
}

PlayListModel::~PlayListModel()
{
    writeSettings();
    clear();
    //qDeleteAll(m_registered_pl_formats);

    foreach(GuardedFileLoader l, running_loaders_)
    {
        if (!l.isNull())
        {
            l->finish();
            l->wait();
        }
    }
    delete PlaylistSettings::instance();
}

void PlayListModel::load(PlayListItem *item)
{
    total_length_ += item->length();
    items_ << item;

    if (item->fileInfo()->path() == init_file_path_)
    {
        current_item_ = item;
        current_ = items_.indexOf(current_item_);
        emit firstAdded();
    }

    if (!block_update_signals_)
    {
        emit listChanged();
    }
}

int PlayListModel::count()
{
    return items_.size();
}

PlayListItem* PlayListModel::currentItem()
{
    if (items_.isEmpty())
    {
        return 0;
    }
    return items_.at(qMin(items_.size() - 1, current_));
}

PlayListItem* PlayListModel::item(int row) const
{
    return (row <  items_.size() && row >= 0) ? items_.at(row) : 0;
}

int PlayListModel::currentRow()
{
    return current_;
}

bool PlayListModel::setCurrent(int c)
{
    if (c > count()-1 || c < 0)
    {
        return FALSE;
    }
    current_ = c;
    current_item_ = items_.at(c);
    emit currentChanged();
    emit listChanged();
    return TRUE;
}


bool PlayListModel::next()
{
    if (isFileLoaderRunning())
    {
        play_state_->prepare();
    }

    return play_state_->next();
}

bool PlayListModel::previous()
{
    if (isFileLoaderRunning())
    {
        play_state_->prepare();
    }

    return play_state_->previous();
}

void PlayListModel::clear()
{
    foreach(GuardedFileLoader l, running_loaders_)
    {
        if (!l.isNull())
        {
            l->finish();
            l->wait();
        }
    }

    running_loaders_.clear();

    current_ = 0;
    while (!items_.isEmpty())
    {
        PlayListItem* mf = items_.takeFirst();

        if (mf->flag() == PlayListItem::FREE)
        {
            delete mf;
        }
        else if (mf->flag() == PlayListItem::EDITING)
        {
            mf->setFlag(PlayListItem::SCHEDULED_FOR_DELETION);
        }
    }
    queued_songs_.clear();
    play_list_model_.reset(0);
    total_length_ = 0;
    current_ = 0;

    play_state_->resetState();
    emit listChanged();
}

void PlayListModel::clearSelection()
{
    for (int i = 0; i<items_.size(); ++i)
    {
        items_.at(i)->setSelected(FALSE);
    }
    emit listChanged();
}

QList <QString> PlayListModel::getTitles(int b,int l)
{
    QList <QString> titles;
    for (int i = b;(i < b + l) &&(i < items_.size()); ++i)
    {
        titles << items_.at(i)->text();
    }
    return titles;
}

QList <QString> PlayListModel::getTimes(int b,int l)
{
    QList <QString> times;
    for (int i = b;(i < b + l) &&(i < items_.size()); ++i)
    {
        times << QString("%1").arg(items_.at(i)->length() /60) +":"
        +QString("%1").arg(items_.at(i)->length() %60/10) +
        QString("%1").arg(items_.at(i)->length() %60%10);
    }
    return times;
}

bool PlayListModel::isSelected(int row)
{
    if (items_.count() > row && row >= 0)
        return items_.at(row)->isSelected();

    return false;
}

void PlayListModel::setSelected(int row, bool yes)
{
    if (items_.count() > row && row >= 0)
        items_.at(row)->setSelected(yes);
}

void PlayListModel::removeSelected()
{
    removeSelection(false);
}

void PlayListModel::removeUnselected()
{
    removeSelection(true);
}

void PlayListModel::removeAt (int i)
{
    if ((i < count()) && (i >= 0))
    {
        PlayListItem* f = items_.takeAt(i);
        total_length_ -= f->length();
        if (total_length_ < 0)
            total_length_ = qMin(0, total_length_);

        if (f->flag() == PlayListItem::FREE)
        {
            delete f;
            f = NULL;
        }
        else if (f->flag() == PlayListItem::EDITING)
            f->setFlag(PlayListItem::SCHEDULED_FOR_DELETION);

        if (current_ >= i && current_ != 0)
            current_--;

        if (!items_.isEmpty())
            current_item_ = items_.at(current_);

        play_state_->prepare();
        emit listChanged();
    }
}

void PlayListModel::removeSelection(bool inverted)
{
    int i = 0;

    int select_after_delete = -1;

    while (!items_.isEmpty() && i<items_.size())
    {
        if (items_.at(i)->isSelected() ^ inverted)
        {
            PlayListItem* f = items_.takeAt(i);
            total_length_ -= f->length();
            if (total_length_ < 0)
                total_length_ = 0;

            if (f->flag() == PlayListItem::FREE)
            {
                delete f;
                f = NULL;
            }
            else if (f->flag() == PlayListItem::EDITING)
                f->setFlag(PlayListItem::SCHEDULED_FOR_DELETION);

            select_after_delete = i;

            if (current_ >= i && current_!=0)
                current_--;
        }
        else
            i++;
    }

    if (!items_.isEmpty())
        current_item_ = items_.at(current_);

    if (select_after_delete >= items_.count())
        select_after_delete = items_.count() - 1;

    setSelected(select_after_delete,true);

    play_state_->prepare();

    emit listChanged();
}

void PlayListModel::invertSelection()
{
    for (int i = 0; i<items_.size(); ++i)
        items_.at(i)->setSelected(!items_.at(i)->isSelected());
    emit listChanged();
}

void PlayListModel::selectAll()
{
    for (int i = 0; i<items_.size(); ++i)
        items_.at(i)->setSelected(TRUE);
    emit listChanged();
}

void PlayListModel::showDetails()
{
    for (int i = 0; i<items_.size(); ++i)
    {
        if (items_.at(i)->isSelected())
        {
            if (!QFile::exists(items_.at(i)->url()))
            {
                PlayListItem *item = items_.at(i);
                QString str;
                str.append(tr("Url:") + " %1\n");
                str.append(tr("Title:") + " %2\n");
                str.append(tr("Artist:") + " %3\n");
                str.append(tr("Album:") + " %4\n");
                str.append(tr("Genre:") + " %5\n");
                str.append(tr("Comment:") + " %6");
                str = str.arg(item->url())
                      .arg(item->title().isEmpty() ? item->text() : item->title())
                      .arg(item->artist())
                      .arg(item->album())
                      .arg(item->genre())
                      .arg(item->comment());
                QMessageBox::information(0, items_.at(i)->url(), str);
                return;
            }

            DecoderFactory *fact = Decoder::findByPath(items_.at(i)->url());
            if (fact)
            {
                QObject* o = fact->showDetails(0, items_.at(i)->url());
                if (o)
                {
                    TagUpdater *updater = new TagUpdater(o,items_.at(i));
                    editing_items_.append(items_.at(i));
                    connect(updater, SIGNAL(destroyed(QObject *)),SIGNAL(listChanged()));
                }
            }
            return;
        }
    }
}

void PlayListModel::readSettings()
{
    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    current_ = settings.value("Playlist/current",0).toInt();

    QString line, param, value;
    int s;
    QList <FileInfo *> infoList;
    QFile file(QDir::homePath() +"/.onyx/playlist.txt");
    file.open(QIODevice::ReadOnly);
    QByteArray array = file.readAll();
    file.close();
    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);
    while (!buffer.atEnd())
    {
        line = QString::fromLocal8Bit(buffer.readLine()).trimmed();
        if ((s = line.indexOf("=")) < 0)
            continue;

        param = line.left(s);
        value = line.right(line.size() - s - 1);

        if (param == "file")
            infoList << new FileInfo(value);
        else if (infoList.isEmpty())
            continue;
        else if (param == "title")
            infoList.last()->setMetaData(PlayerUtils::TITLE, value);
        else if (param == "artist")
            infoList.last()->setMetaData(PlayerUtils::ARTIST, value);
        else if (param == "album")
            infoList.last()->setMetaData(PlayerUtils::ALBUM, value);
        else if (param == "comment")
            infoList.last()->setMetaData(PlayerUtils::COMMENT, value);
        else if (param == "genre")
            infoList.last()->setMetaData(PlayerUtils::GENRE, value);
        else if (param == "year")
            infoList.last()->setMetaData(PlayerUtils::YEAR, value);
        else if (param == "track")
            infoList.last()->setMetaData(PlayerUtils::TRACK, value);
        else if (param == "length")
            infoList.last()->setLength(value.toInt());
    }
    buffer.close();
    if (current_ > infoList.count() - 1)
        current_ = 0;
    block_update_signals_ = TRUE;
    foreach(FileInfo *info, infoList)
    {
        load(new PlayListItem(info));
    }
    block_update_signals_ = FALSE;
    doCurrentVisibleRequest();
}

void PlayListModel::writeSettings()
{
    QFile file(QDir::homePath() +"/.onyx/playlist.txt");
    file.open(QIODevice::WriteOnly);
    foreach(PlayListItem* m, items_)
    {
        file.write(QString("file=%1").arg(m->url()).toLocal8Bit() +"\n");
        file.write(QString("title=%1").arg(m->title()).toLocal8Bit() +"\n");
        file.write(QString("artist=%1").arg(m->artist()).toLocal8Bit() +"\n");
        file.write(QString("album=%1").arg(m->album()).toLocal8Bit() +"\n");
        file.write(QString("comment=%1").arg(m->comment()).toLocal8Bit() +"\n");
        file.write(QString("genre=%1").arg(m->genre()).toLocal8Bit() +"\n");
        file.write(QString("year=%1").arg(m->year()).toLocal8Bit() +"\n");
        file.write(QString("track=%1").arg(m->track()).toLocal8Bit() +"\n");
        file.write(QString("length=%1").arg(m->length()).toLocal8Bit() +"\n");
    }
    file.close();
    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    settings.setValue("Playlist/current", current_);
}

void PlayListModel::addFile(const QString& path)
{
    if (path.isEmpty())
    {
        return;
    }

    init_file_path_ = path;
    QList <FileInfo *> playList;
    Decoder::createPlayList(path, playList, PlaylistSettings::instance()->useMetadata());
    foreach(FileInfo *info, playList)
    {
        load(new PlayListItem(info));
    }

    preparePlayState();
}

FileLoader * PlayListModel::createFileLoader()
{
    FileLoader* f_loader = new FileLoader(this);
    // f_loader->setStackSize(20 * 1024 * 1024);
    running_loaders_ << f_loader;
    connect(f_loader, SIGNAL(newPlayListItem(PlayListItem*)), this, SLOT(load(PlayListItem*)), Qt::QueuedConnection);
    connect(f_loader, SIGNAL(finished()), this, SLOT(preparePlayState()));
    connect(f_loader, SIGNAL(finished()), f_loader, SLOT(deleteLater()));
    return f_loader;
}

void PlayListModel::addFiles(const QStringList &files)
{
    FileLoader* f_loader = createFileLoader();
    f_loader->setFilesToLoad(files);
    f_loader->start(QThread::IdlePriority);
}

void PlayListModel::addDirectory(const QString& file_path)
{
    if ( !file_path.isEmpty() )
    {
        QFileInfo info(file_path);
        QString dir_path = info.dir().absolutePath();
        init_file_path_ = info.absoluteFilePath();

        FileLoader* f_loader = createFileLoader();
        f_loader->setDirectoryToLoad(dir_path);
        f_loader->start(QThread::IdlePriority);
    }
}

void PlayListModel::addFileList(const QStringList &l)
{
//    qWarning("void// PlayListModel::addFileList(const QStringList &l)");
    foreach(QString str,l)
    {
        QFileInfo f_info(str);
        if (f_info.exists() || str.contains("://"))
        {
            if (f_info.isDir())
                addDirectory(str);
            else
            {
                addFile(str);
            }
        }
        // Do processing the rest of events to avoid GUI freezing
        QApplication::processEvents(QEventLoop::AllEvents,10);
    }
}

bool PlayListModel::setFileList(const QStringList & l)
{
    bool model_cleared = FALSE;
    foreach(QString str,l)
    {
        QFileInfo f_info(str);
        if (f_info.exists() || str.contains("://"))
        {
            if (!model_cleared)
            {
                clear();
                model_cleared = TRUE;
            }
            if (f_info.isDir())
                addDirectory(str);
            else
            {
                addFile(str);
            }
        }
        // Do processing the rest of events to avoid GUI freezing
        QApplication::processEvents(QEventLoop::AllEvents,10);
    }

    return model_cleared;
}

int PlayListModel::firstSelectedUpper(int row)
{
    for (int i = row - 1;i >= 0;i--)
    {
        if (isSelected(i))
            return i;
    }
    return -1;
}

int PlayListModel::firstSelectedLower(int row)
{
    for (int i = row + 1;i < count() ;i++)
    {
        if (isSelected(i))
            return i;
    }
    return -1;
}

void PlayListModel::moveItems(int from, int to)
{
    // Get rid of useless work
    if (from == to)
        return;

    QList<int> selected_rows = getSelectedRows();

    if (!(bottommostInSelection(from) == INVALID_ROW ||
            from == INVALID_ROW ||
            topmostInSelection(from) == INVALID_ROW)
       )
    {
        if (from > to)
            foreach(int i, selected_rows)
            if (i + to - from < 0)
                break;
            else
                items_.move(i,i + to - from);
        else
            for (int i = selected_rows.count() - 1; i >= 0; i--)
                if (selected_rows[i] + to -from >= items_.count())
                    break;
                else
                    items_.move(selected_rows[i],selected_rows[i] + to - from);

        current_ = items_.indexOf(current_item_);

        emit listChanged();
    }
}

int PlayListModel::topmostInSelection(int row)
{
    if (row == 0)
        return 0;

    for (int i = row - 1;i >= 0;i--)
    {
        if (isSelected(i))
            continue;
        else
            return i + 1;
    }
    return 0;
}

int PlayListModel::bottommostInSelection(int row)
{
    if (row >= items_.count() - 1)
        return row;

    for (int i = row + 1;i < count() ;i++)
    {
        if (isSelected(i))
            continue;
        else
            return i - 1;
    }
    return count() - 1;
}

const SimpleSelection& PlayListModel::getSelection(int row)
{
    selection_.top_ = topmostInSelection(row);
    selection_.anchor_ = row;
    selection_.bottom_ = bottommostInSelection(row);
    selection_.selected_rows_ = getSelectedRows();
    return selection_;
}

QList<int> PlayListModel::getSelectedRows() const
{
    QList<int>selected_rows;
    for (int i = 0;i<items_.count();i++)
    {
        if (items_[i]->isSelected())
        {
            selected_rows.append(i);
        }
    }
    return selected_rows;
}

QList< PlayListItem * > PlayListModel::getSelectedItems() const
{
    QList<PlayListItem*>selected_items;
    for (int i = 0;i<items_.count();i++)
    {
        if (items_[i]->isSelected())
        {
            selected_items.append(items_[i]);
        }
    }
    return selected_items;
}

void PlayListModel::addToQueue()
{
    QList<PlayListItem*> selected_items = getSelectedItems();
    foreach(PlayListItem* file,selected_items)
    {/*
                if(isQueued(file))
                    queued_songs_.removeAt(queued_songs_.indexOf(file));
                else
                    queued_songs_.append(file);
                 */
        setQueued(file);
    }
    emit listChanged();
}

void PlayListModel::setQueued(PlayListItem* file)
{
    if (isQueued(file))
        queued_songs_.removeAt(queued_songs_.indexOf(file));
    else
        queued_songs_.append(file);

    emit listChanged();
}

bool PlayListModel::isQueued(PlayListItem* f) const
{
    return queued_songs_.contains(f);
}

void PlayListModel::setCurrentToQueued()
{
    setCurrent(row(queued_songs_.at(0)));
    queued_songs_.pop_front();
}

bool PlayListModel::isEmptyQueue() const
{
    return queued_songs_.isEmpty();
}

void PlayListModel::randomizeList()
{
    for (int i = 0;i < items_.size();i++)
        items_.swap(qrand()%items_.size(),qrand()%items_.size());

    current_ = items_.indexOf(current_item_);
    emit listChanged();
}

void PlayListModel::reverseList()
{
    for (int i = 0;i < items_.size()/2;i++)
        items_.swap(i,items_.size() - i - 1);

    current_ = items_.indexOf(current_item_);
    emit listChanged();
}

////===============THE BEGINNING OF SORT IMPLEMENTATION =======================////

// First we'll implement bundle of static compare procedures
// to sort items in different ways
static bool _titleLessComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->title() < s2->title();
}

static bool _titleGreaterComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->title() > s2->title();
}

static bool _pathAndFilenameLessComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->url() < s2->url();
}

static bool _pathAndFilenameGreaterComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->url() > s2->url();
}

static bool _filenameLessComparator(PlayListItem* s1,PlayListItem* s2)
{
    QFileInfo i_s1(s1->url());
    QFileInfo i_s2(s2->url());
    return i_s1.baseName() < i_s2.baseName();
}

static bool _filenameGreaterComparator(PlayListItem* s1,PlayListItem* s2)
{
    QFileInfo i_s1(s1->url());
    QFileInfo i_s2(s2->url());
    return i_s1.baseName() > i_s2.baseName();
}

static bool _dateLessComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->year().toInt() < s2->year().toInt();
}

static bool _dateGreaterComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->year().toInt() > s2->year().toInt();
}

static bool _trackLessComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->track().toInt() < s2->track().toInt();
}

static bool _trackGreaterComparator(PlayListItem* s1,PlayListItem* s2)
{
    return s1->track().toInt() > s2->track().toInt();
}

// This is main sort method
void PlayListModel::doSort(int sort_mode,QList<PlayListItem*>& list_to_sort)
{
    QList<PlayListItem*>::iterator begin;
    QList<PlayListItem*>::iterator end;

    begin = list_to_sort.begin();
    end = list_to_sort.end();

    bool(*compareLessFunc)(PlayListItem*,PlayListItem*) = 0;
    bool(*compareGreaterFunc)(PlayListItem*,PlayListItem*) = 0;

    switch (sort_mode)
    {
    case TITLE:
        compareLessFunc = _titleLessComparator;
        compareGreaterFunc = _titleGreaterComparator;
        break;
    case FILENAME:
        compareLessFunc = _filenameLessComparator;
        compareGreaterFunc = _filenameGreaterComparator;
        break;
    case PATH_AND_FILENAME:
        compareLessFunc = _pathAndFilenameLessComparator;
        compareGreaterFunc = _pathAndFilenameGreaterComparator;
        break;
    case DATE:
        compareLessFunc = _dateLessComparator;
        compareGreaterFunc = _dateGreaterComparator;
        break;
        //qWarning("TODO Sort by Date: %s\t%d",__FILE__,__LINE__);
    case TRACK:
        compareLessFunc = _trackLessComparator;
        compareGreaterFunc = _trackGreaterComparator;
        break;
    default:
        compareLessFunc = _titleLessComparator;
        compareGreaterFunc = _titleGreaterComparator;
    }

    static bool sorted_asc = false;
    if (!sorted_asc)
    {
        qSort(begin,end,compareLessFunc);
        sorted_asc = true;
    }
    else
    {
        qSort(begin,end,compareGreaterFunc);
        sorted_asc = false;
    }

    current_ = items_.indexOf(current_item_);
}

void PlayListModel::sortSelection(int mode)
{
    QList<PlayListItem*>selected_items = getSelectedItems();
    QList<int>selected_rows = getSelectedRows();

    doSort(mode,selected_items);

    for (int i = 0;i < selected_rows.count();i++)
        items_.replace(selected_rows[i],selected_items[i]);

    current_ = items_.indexOf(current_item_);
    emit listChanged();
}

void PlayListModel::sort(int mode)
{
    doSort(mode,items_);
    emit listChanged();
}

////=============== THE END OF SORT IMPLEMENTATION =======================////

void PlayListModel::prepareForShufflePlaying(bool val)
{
    if (val)
    {
        play_state_.reset(new ShufflePlayState(this));
    }
    else
    {
        play_state_.reset(new NormalPlayState(this));
    }

    shuffle_ = val;
    emit shuffleChanged(val);
}

void PlayListModel::prepareForRepeatablePlaying(bool val)
{
    is_repeatable_list_ = val;
    emit repeatableListChanged(val);
}

void PlayListModel::doCurrentVisibleRequest()
{
    emit currentChanged();
    emit listChanged();
}

void PlayListModel::setUpdatesEnabled(bool yes)
{
    block_update_signals_ = !yes;
    if (yes)
    {
        emit listChanged();
    }
}

bool PlayListModel::isRepeatableList() const
{
    return is_repeatable_list_;
}

bool PlayListModel::isShuffle() const
{
    return shuffle_;
}

QStandardItemModel* PlayListModel::standardItemModel()
{
    if (play_list_model_ == 0 || play_list_model_->rowCount() < items_.size())
    {
        play_list_model_.reset(new QStandardItemModel);
        int row = 0;
        play_list_model_->setColumnCount(4);
        foreach(PlayListItem* item, items_)
        {
            //QString name_str = item->url().contains('/') ?
            //                   item->url().split('/',QString::SkipEmptyParts).takeLast() : item->url();
            //QStandardItem *name = new QStandardItem(name_str);
            //name->setEditable(false);
            //name->setData(QVariant::fromValue(row));
            //play_list_model_->setItem(row, 0, name);

            QString name_str = item->url();
            if (name_str.contains('/'))
            {
                name_str = name_str.split('/',QString::SkipEmptyParts).takeLast();
            }
            QStandardItem *name = new QStandardItem(name_str);
            name->setEditable(false);
            name->setData(QVariant::fromValue(row));
            play_list_model_->setItem(row, 0, name);


            QString title_str = item->title();
            QStandardItem *title = new QStandardItem(title_str);
            title->setEditable(false);
            title->setData(QVariant::fromValue(row));
            play_list_model_->setItem(row, 1, title);

            QString artist_str = item->artist();
            QStandardItem *artist = new QStandardItem(artist_str);
            artist->setEditable(false);
            artist->setTextAlignment(Qt::AlignCenter);
            play_list_model_->setItem(row, 2, artist);

            QString album_str = item->album();
            QStandardItem *album = new QStandardItem(album_str);
            album->setEditable(false);
            album->setTextAlignment(Qt::AlignCenter);
            play_list_model_->setItem(row, 3, album);
            row++;
        }
    }

    play_list_model_->setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Song")), Qt::DisplayRole);
    play_list_model_->setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
    play_list_model_->setHeaderData(2, Qt::Horizontal, QVariant::fromValue(tr("Artist")), Qt::DisplayRole);
    play_list_model_->setHeaderData(3, Qt::Horizontal, QVariant::fromValue(tr("Album")), Qt::DisplayRole);
    return play_list_model_.get();
}

bool PlayListModel::isFileLoaderRunning() const
{
    foreach(FileLoader* l,running_loaders_)
    if (l && l->isRunning())
        return TRUE;

    return FALSE;
}

void PlayListModel::preparePlayState()
{
    play_state_->prepare();
    emit loadingFinished();
}

bool PlayListModel::convertUnderscore()
{
    return PlaylistSettings::instance()->convertUnderscore();
}

bool PlayListModel::convertTwenty()
{
    return PlaylistSettings::instance()->convertTwenty();
}

bool PlayListModel::useMetadata()
{
    return PlaylistSettings::instance()->useMetadata();
}

const QString PlayListModel::format() const
{
    return PlaylistSettings::instance()->format();
}

void PlayListModel::setConvertUnderscore(bool yes)
{
    PlaylistSettings::instance()->setConvertUnderscore(yes);
    emit settingsChanged();
}

void PlayListModel::setConvertTwenty(bool yes)
{
    PlaylistSettings::instance()->setConvertTwenty(yes);
    emit settingsChanged();
}

void PlayListModel::setUseMetadata(bool yes)
{
    PlaylistSettings::instance()->setUseMetadata(yes);
    emit settingsChanged();
}

void PlayListModel::setFormat(const QString &format)
{
    PlaylistSettings::instance()->setFormat(format);
    emit settingsChanged();
}

}

#include <model/playlistitem.h>
#include <core/fileinfo.h>

#include "onyx_player_view.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"

namespace player
{

enum MusicPlayerMenuType
{
    MENU_MODE = 11,
    MENU_PREVIOUS = 12,
    MENU_PLAY = 13,
    MENU_NEXT = 14,
    MENU_MINIMIZE = 15,
    MENU_EXIT = 16,
};

const static QSize MENU_ITEM_SIZE = QSize(60, 60);
const static QString TAG_ROW = "row";

OnyxPlayerView::OnyxPlayerView(QWidget *parent)
    : OnyxDialog(parent)
    , model_(0)
    , core_(new SoundCore(this))
    , big_layout_(&content_widget_)
    , window_title_layout_(0)
    , artist_layout_(0)
    , album_layout_(0)
    , song_list_view_(0, this)
    , menu_view_(0, this)
    , status_bar_(this, MENU | PROGRESS | BATTERY)
    , repeat_(false)
    , seeking_(false)
    , paused_(false)
    , progress_bar_enabled_(true)
    , skips_(0)
    , previous_page_(1)
{
#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

    connect(core_.get(), SIGNAL(finished()),
            this, SLOT(next()));
    connect(core_.get(), SIGNAL(stateChanged(PlayerUtils::State)),
            this, SLOT(showState(PlayerUtils::State)));
    connect(core_.get(), SIGNAL(elapsedChanged(qint64)),
            this, SLOT(setTime(qint64)));
    connect(core_.get(), SIGNAL(metaDataChanged()),
            this, SLOT(showMetaData()));
    connect(core_.get(), SIGNAL(bufferingProgress(int)),
            this, SLOT(setProgress(int)));

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(volumeChanged(int, bool)), this, SLOT(onSystemVolumeChanged(int, bool)));

    createLayout();
    connectWithChildren();
}

OnyxPlayerView::~OnyxPlayerView()
{

}


void OnyxPlayerView::loadSettings()
{
    int volume = sys::SysStatus::instance().volume();
    qDebug("Read volume in sys conf:%d", volume);
    core_->setVolume(volume, volume);

    // Repeat/Shuffle
    model_->prepareForRepeatablePlaying(PlayerUtils::isRepeatableList());
    model_->prepareForShufflePlaying(PlayerUtils::isShuffled());
}

void OnyxPlayerView::saveSettings()
{
    // save the current playing song
    if (model_->currentItem() != 0)
    {
        QString current_item_path = model_->currentItem()->fileInfo()->path();
        PlayerUtils::setLastSong(current_item_path);
    }

    // save the volume
    PlayerUtils::setLeftVolume(core_->leftVolume());
    PlayerUtils::setRightVolume(core_->rightVolume());

    // save status of shuffle and repeatable playing
//    PlayerUtils::setShuffled(shuffle_button_.isChecked());
//    PlayerUtils::setRepeatableList(cycle_button_.isChecked());
}

void OnyxPlayerView::attachModel(PlayListModel *m)
{
    model_ = m;
    connect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
    connect(model_, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
    connect(model_, SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));
    connect(model_, SIGNAL(shuffleChanged(bool)), this, SLOT(onShuffleStatusChanged(bool)));
    connect(model_, SIGNAL(repeatableListChanged(bool)), this, SLOT(onRepeatListChanged(bool)));

    model_->doCurrentVisibleRequest();
    loadSettings();
}

void OnyxPlayerView::deattachModel()
{
    if (model_ != 0)
    {
        disconnect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
        disconnect(model_, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
        disconnect(model_, SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));
        disconnect(model_, SIGNAL(shuffleChanged(bool)), this, SLOT(onShuffleStatusChanged(bool)));
        disconnect(model_, SIGNAL(repeatableListChanged(bool)), this, SLOT(onRepeatListChanged(bool)));
        model_ = 0;
    }
}

void OnyxPlayerView::createLayout()
{
    vbox_.removeWidget(&title_widget_);
    content_widget_.setBackgroundRole(QPalette::Button);

    window_icon_label_.setPixmap(QPixmap(":/player_icons/music_player_small.png"));
    window_icon_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    window_icon_label_.setFixedHeight(defaultItemHeight());

    window_title_label_.setText(QCoreApplication::tr("Music Player"));
    window_title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    window_title_label_.setFixedHeight(defaultItemHeight());

    normal_mode_pixmap_ = QPixmap(":/player_icons/normal.png");
    repeat_mode_pixmap_ = QPixmap(":/player_icons/repeat.png");
    shuffle_mode_pixmap_ = QPixmap(":/player_icons/shuffle.png");
    play_pixmap_ = QPixmap(":/player_icons/play.png");
    pause_pixmap_ = QPixmap(":/player_icons/pause.png");

    createMenuView();

    title_label_.setText("");
    title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    title_label_.setFixedHeight(defaultItemHeight());

    artist_title_label_.setText(QCoreApplication::tr("Artist: "));
    artist_title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    artist_title_label_.setFixedHeight(defaultItemHeight());
    artist_title_label_.setFixedWidth(100);

    album_title_label_.setText(QCoreApplication::tr("Album: "));
    album_title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    album_title_label_.setFixedHeight(defaultItemHeight());
    album_title_label_.setFixedWidth(100);

    window_title_layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    window_title_layout_.addWidget(&window_icon_label_);
    window_title_layout_.addSpacing(SPACING*4);
    window_title_layout_.addWidget(&window_title_label_, 1, Qt::AlignLeft);

    big_layout_.setContentsMargins(2, 2, 2, 2);
    big_layout_.setSpacing(0);
    big_layout_.addLayout(&window_title_layout_);
    big_layout_.addWidget(&song_list_view_, 1, Qt::AlignTop);

    // add audio title, artist, album
    big_layout_.addWidget(&title_label_);
    artist_layout_.addWidget(&artist_title_label_);
    artist_layout_.addWidget(&artist_label_);
    album_layout_.addWidget(&album_title_label_);
    album_layout_.addWidget(&album_label_);
    big_layout_.addLayout(&artist_layout_);
    big_layout_.addLayout(&album_layout_);

    big_layout_.addWidget(&menu_view_, 0, Qt::AlignBottom);
    big_layout_.addWidget(&status_bar_, 0, Qt::AlignBottom);
}

void OnyxPlayerView::createSongListView()
{
    const int height = defaultItemHeight();
    song_list_view_.setPreferItemSize(QSize(height, height));
    ODatas ds;

    QStandardItemModel * item_model = model_->standardItemModel();
    int rows = item_model->rowCount();
    for (int i=0; i<rows; i++)
    {
        QStandardItem *item = item_model->item(i);
        OData *dd = new OData;
        dd->insert(TAG_TITLE, item->text());
        dd->insert(TAG_ROW, i);
        ds.push_back(dd);
    }

    song_list_view_.setSpacing(2);
    song_list_view_.setFixedGrid(rows, 1);
    int single_height = defaultItemHeight()+2*SPACING;
    song_list_view_.setFixedHeight(single_height*rows);
    song_list_view_.setData(ds);
    song_list_view_.setNeighbor(&menu_view_, CatalogView::DOWN);
    song_list_view_.setNeighbor(&menu_view_, CatalogView::RECYCLE_DOWN);
}

void OnyxPlayerView::createMenuView()
{
    menu_view_.setPreferItemSize(MENU_ITEM_SIZE);
    ODatas ds;

    OData *dd = new OData;
    dd->insert(TAG_COVER, normal_mode_pixmap_);
    dd->insert(TAG_MENU_TYPE, MENU_MODE);
    ds.push_back(dd);

    dd = new OData;
    QPixmap prev_pixmap(":/player_icons/previous.png");
    dd->insert(TAG_COVER, prev_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_PREVIOUS);
    ds.push_back(dd);

    dd = new OData;
    dd->insert(TAG_COVER, play_pixmap_);
    dd->insert(TAG_MENU_TYPE, MENU_PLAY);
    ds.push_back(dd);

    dd = new OData;
    QPixmap next_pixmap(":/player_icons/next.png");
    dd->insert(TAG_COVER, next_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_NEXT);
    ds.push_back(dd);

    dd = new OData;
    QPixmap min_pixmap(":/player_icons/minimize.png");
    dd->insert(TAG_COVER, min_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_MINIMIZE);
    ds.push_back(dd);

    dd = new OData;
    QPixmap exit_pixmap(":/player_icons/exit.png");
    dd->insert(TAG_COVER, exit_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_EXIT);
    ds.push_back(dd);

    menu_view_.setSpacing(2);
    menu_view_.setFixedGrid(1, 6);
    menu_view_.setFixedHeight(MENU_ITEM_SIZE.height() + 2 * SPACING);
    menu_view_.setData(ds);
    menu_view_.setSearchPolicy(CatalogView::NeighborFirst
            | CatalogView::AutoHorRecycle);
    menu_view_.setNeighbor(&song_list_view_, CatalogView::RECYCLE_UP);
}

void OnyxPlayerView::connectWithChildren()
{
    connect(&song_list_view_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&menu_view_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}


void OnyxPlayerView::showState(PlayerUtils::State state)
{
    int current_state = MUSIC_PLAYING;
    switch (state)
    {
        case PlayerUtils::Playing:
        {
            qDebug("Playing");
//            play_pause_button_.setIcon(pause_icon_);
            break;
        }
        case PlayerUtils::Paused:
        {
            qDebug("Paused");
            current_state = MUSIC_PAUSED;
//            play_pause_button_.setIcon(play_icon_);
            break;
        }
        case PlayerUtils::Stopped:
        {
            qDebug("Stopped");
            current_state = MUSIC_STOPPED;
//            play_pause_button_.setIcon(play_icon_);
            break;
        }
    }
    emit stateChanged(current_state);
}

void OnyxPlayerView::showMetaData()
{
    qDebug("===== metadata ======");
    qDebug("ARTIST = %s", qPrintable(core_->metaData(PlayerUtils::ARTIST)));
    qDebug("TITLE = %s", qPrintable(core_->metaData(PlayerUtils::TITLE)));
    qDebug("ALBUM = %s", qPrintable(core_->metaData(PlayerUtils::ALBUM)));
    qDebug("COMMENT = %s", qPrintable(core_->metaData(PlayerUtils::COMMENT)));
    qDebug("GENRE = %s", qPrintable(core_->metaData(PlayerUtils::GENRE)));
    qDebug("YEAR = %s", qPrintable(core_->metaData(PlayerUtils::YEAR)));
    qDebug("TRACK = %s", qPrintable(core_->metaData(PlayerUtils::TRACK)));
    qDebug("== end of metadata ==");
}

void OnyxPlayerView::setProgress(int p)
{
    qDebug("Progress:%d", p);
    core_->seek(p);
}

void OnyxPlayerView::setTime(qint64 t)
{
    QTime corrent_t(0, 0, 0);
    QTime total_t(0, 0, 0);
    corrent_t=corrent_t.addMSecs( (int)t );
    total_t=total_t.addMSecs( (int)core_->totalTime() );
    QString msg=corrent_t.toString()+"/"+total_t.toString();

    if( total_t.hour()<1 )
        msg=corrent_t.toString(QString("mm:ss"))+" / "+total_t.toString(QString("mm:ss"));

    if (progress_bar_enabled_ && isVisible() && core_->totalTime() >= t)
    {
        static int count = 0;
        if (count == 0)
        {
            onyx::screen::instance().enableUpdate(false);
            qDebug() << "Set Time: " << msg;
//            status_bar_.setProgress(t, core_->totalTime(), true, msg);
            onyx::screen::instance().enableUpdate(true);
            onyx::screen::instance().updateWidget(&status_bar_,
                    onyx::screen::ScreenProxy::DW, false,
                    onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH);
        }
        if (count >= 8)
        {
            count = 0;
        }
        else
        {
            count++;
        }
    }
}

void OnyxPlayerView::play()
{
    model_->doCurrentVisibleRequest();
    if (core_->state() == PlayerUtils::Paused)
    {
        core_->pause();
        return;
    }

    if (model_->count() == 0)
        return;

    QString s = model_->currentItem()->url();
    if (s.isEmpty())
    {
        return;
    }

    if (!core_->play(s))
    {
        //find out the reason why playback failed
        switch ((int) core_->state())
        {
        case PlayerUtils::FatalError:
        {
            qWarning("Fatal Error");
            stop();
            return; //unrecovable error in output, so abort playing
        }
        case PlayerUtils::NormalError:
        {
            //error in decoder, so we should try to play next song
            qWarning("Normal error in decoder, try to play next song");
            skips_++;
            if (skips_ > 5)
            {
                stop();
                qWarning("MediaPlayer: skip limit exceeded");
                break;
            }
            qApp->processEvents();
            if (!model_->isEmptyQueue())
            {
                model_->setCurrentToQueued();
            }
            else if (!model_->next())
            {
                stop();
                return;
            }
            play();
            break;
        }
        }
    }
    else
    {
        skips_ = 0;
    }

    if ( sys::SysStatus::instance().isSystemBusy() )
    {
        // if it is the first time rendering, set busy to be false
        sys::SysStatus::instance().setSystemBusy( false );
    }
    // The firstAdded might be posted when music player is running after the first song is
    // launched, so disconnect the signal after the play() is called.
    disconnect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
}

void OnyxPlayerView::stop()
{
    core_->stop();
}

void OnyxPlayerView::next()
{
    if (!model_->isEmptyQueue())
    {
        model_->setCurrentToQueued();
    }
    else if (!model_->next())
    {
        stop();
        return;
    }

    if (core_->state() != PlayerUtils::Stopped)
    {
        if (core_->state() == PlayerUtils::Paused)
        {
            stop();
        }
        play();
    }
    onyx::screen::watcher().enqueue(&song_list_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::previous()
{
    if (!model_->previous())
    {
        stop();
        return;
    }

    if (core_->state() != PlayerUtils::Stopped)
    {
        if (core_->state() == PlayerUtils::Paused)
        {
            stop();
        }
        play();
    }
    onyx::screen::watcher().enqueue(&song_list_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::pause()
{
    core_->pause();
}

void OnyxPlayerView::close(bool)
{
    hide();
    stop();
    saveSettings();

    emit stateChanged(STOP_PLAYER);
    qApp->exit();
}

void OnyxPlayerView::minimize(bool)
{
    if (!isHidden())
    {
        enableProgressBar(false);
        onyx::screen::instance().enableUpdate(false);
        hide();
        QApplication::processEvents();
        onyx::screen::instance().enableUpdate(true);
        emit stateChanged(HIDE_PLAYER);
    }
}

void OnyxPlayerView::onPlayPauseClicked(bool)
{
    if (core_->state() == PlayerUtils::Playing)
    {
        paused_ = true;
        pause();
    }
    else if (core_->state() == PlayerUtils::Paused ||
             core_->state() == PlayerUtils::Stopped)
    {
        paused_ = false;
        play();
    }
    foreach (ContentView * item, menu_view_.visibleSubItems())
    {
        if (item->data()->contains(TAG_MENU_TYPE))
        {
            int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
            if (MENU_PLAY == menu_type)
            {
                item->data()->insert(TAG_COVER,
                        paused_? pause_pixmap_: play_pixmap_);
                break;
            }
        }
    }
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onNextClicked(bool)
{
    next();
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onPrevClicked(bool)
{
    previous();
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onSystemVolumeChanged(int value, bool muted)
{
    qDebug("Volume Change:%d", value);
    core_->setVolume(value, value);
}

void OnyxPlayerView::onCurrentChanged()
{
    int current_row = model_->currentRow();
    QModelIndex idx = model_->standardItemModel()->index(current_row, 0);
    if (idx.isValid())
    {
        QStandardItem *info_item = model_->standardItemModel()->item(current_row, 1);
        title_label_.setText(info_item->text());
        info_item = model_->standardItemModel()->item(current_row, 2);
        artist_label_.setText(info_item->text());
        info_item = model_->standardItemModel()->item(current_row, 3);
        album_label_.setText(info_item->text());

        foreach (ContentView * item, song_list_view_.visibleSubItems())
        {
            if (item->data()->contains(TAG_ROW))
            {
                int row = item->data()->value(TAG_ROW).toInt();
                if (current_row == row)
                {
                    item->setFocus();
                    break;
                }
            }
        }
    }
}

void OnyxPlayerView::onProgressClicked(const int percent, const int value)
{
    core_->seek(value);
}

void OnyxPlayerView::onShuffleStatusChanged(bool yes)
{
//    shuffle_button_.setIcon(yes ? normal_icon_ : shuffle_icon_);
//    shuffle_button_.setChecked(yes);
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onRepeatListChanged(bool yes)
{
//    cycle_button_.setIcon(yes ? cycle_selected_icon_ : cycle_icon_);
//    cycle_button_.setChecked(yes);
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onLoadingFinished()
{
    createSongListView();

    onCurrentChanged();
    if ( sys::SysStatus::instance().isSystemBusy() )
    {
        // if it is the first time rendering, set busy to be false
        sys::SysStatus::instance().setSystemBusy( false );
    }
    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void OnyxPlayerView::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        int menu_type = item_data->value(TAG_MENU_TYPE).toInt();
        if (MENU_MODE == menu_type)
        {
            // TODO change mode
            qDebug() << "mode clicked.";
        }
        else if (MENU_PREVIOUS == menu_type)
        {
            onPrevClicked(true);
        }
        else if (MENU_PLAY == menu_type)
        {
            onPlayPauseClicked(true);
        }
        else if (MENU_NEXT == menu_type)
        {
            onNextClicked(true);
        }
        else if (MENU_MINIMIZE == menu_type)
        {
            minimize(true);
        }
        else if (MENU_EXIT == menu_type)
        {
            close(true);
        }
    }
    else if (item->data()->contains(TAG_ROW))
    {
        int row = item_data->value(TAG_ROW).toInt();
        QStandardItem *audio_item = model_->standardItemModel()->item(row, 0);
        if (0 != item)
        {
            model_->setCurrent(row);
            play();
        }
    }
}

}

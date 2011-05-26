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
const static int AUDIO_INFO_SPACING = 40;

const static QString PROGRESS_BAR_STYLE = " \
QProgressBar:horizontal                     \
{                                           \
    border: 2px solid gray;                 \
    border-radius: 3px;                     \
    background: white;                      \
    padding: 1px;                           \
}                                           \
QProgressBar::chunk:horizontal              \
{                                           \
    background: black;                      \
}";


OnyxPlayerView::OnyxPlayerView(QWidget *parent)
    : OnyxDialog(parent)
    , model_(0)
    , core_(new SoundCore(this))
    , player_title_bar_(this)
    , big_layout_(&content_widget_)
    , title_layout_(0)
    , artist_layout_(0)
    , album_layout_(0)
    , song_list_view_(0, this)
    , menu_view_(0, this)
    , status_bar_(this, MENU | PROGRESS | BATTERY)
    , single_repeat_mode_(false)
    , shuffle_mode_(false)
    , seeking_(false)
    , paused_(false)
    , progress_bar_enabled_(true)
    , skips_(0)
    , previous_page_(1)
    , fixed_grid_rows_(0)
    , need_refresh_immediately_(true)
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

    // show page info in status bar
    connect(&song_list_view_, SIGNAL(positionChanged(const int, const int)),
            this, SLOT(onPositionChanged(const int, const int)));
    connect(&status_bar_,  SIGNAL(progressClicked(const int, const int)),
            this, SLOT(onPagebarClicked(const int, const int)));

    createLayout();
    connectWithChildren();
}

OnyxPlayerView::~OnyxPlayerView()
{
    clearDatas(song_list_data_);
    clearDatas(menu_view_datas_);
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
    PlayerUtils::setShuffled(shuffle_mode_);
    PlayerUtils::setRepeatableList(true);
}

void OnyxPlayerView::attachModel(PlayListModel *m)
{
    model_ = m;
    connect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
    connect(model_, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
    connect(model_, SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));

    model_->doCurrentVisibleRequest();
    loadSettings();
}

void OnyxPlayerView::deattachModel()
{
    if (model_ != 0)
    {
        // clear data
        ODatas empty_data;
        song_list_view_.setData(empty_data);

        disconnect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
        disconnect(model_, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
        disconnect(model_, SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));

        model_ = 0;
    }
}

void OnyxPlayerView::createLayout()
{
    vbox_.removeWidget(&title_widget_);
    content_widget_.setBackgroundRole(QPalette::Button);

    normal_mode_pixmap_ = QPixmap(":/player_icons/normal.png");
    single_repeat_mode_pixmap_ = QPixmap(":/player_icons/repeat.png");
    shuffle_mode_pixmap_ = QPixmap(":/player_icons/shuffle.png");
    play_pixmap_ = QPixmap(":/player_icons/play.png");
    pause_pixmap_ = QPixmap(":/player_icons/pause.png");

    title_title_label_.setPixmap(QPixmap(":/player_icons/title.png"));
    title_title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    title_title_label_.setFixedHeight(defaultItemHeight());
    title_title_label_.setFixedWidth(AUDIO_INFO_SPACING);

    artist_title_label_.setPixmap(QPixmap(":/player_icons/artist.png"));
    artist_title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    artist_title_label_.setFixedHeight(defaultItemHeight());
    artist_title_label_.setFixedWidth(AUDIO_INFO_SPACING);

    album_title_label_.setPixmap(QPixmap(":/player_icons/album.png"));
    album_title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    album_title_label_.setFixedHeight(defaultItemHeight());
    album_title_label_.setFixedWidth(AUDIO_INFO_SPACING);

    current_time_label_.setText("00:00");
    current_time_label_.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    current_time_label_.setFixedHeight(defaultItemHeight());

    total_time_label_.setText("00:00");
    total_time_label_.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    total_time_label_.setFixedHeight(defaultItemHeight());

    progress_bar_.setStyleSheet(PROGRESS_BAR_STYLE);
    progress_bar_.setTextVisible(false);

    createMenuView();

    big_layout_.setContentsMargins(2, 2, 2, 2);
    big_layout_.setSpacing(0);
    big_layout_.addWidget(&player_title_bar_, 0, Qt::AlignTop);
    big_layout_.addWidget(&song_list_view_, 1, Qt::AlignTop);

    // add title info
    title_layout_.addWidget(&title_title_label_);
    title_layout_.addWidget(&title_label_);
    big_layout_.addLayout(&title_layout_);

    // add artist info
    artist_layout_.addWidget(&artist_title_label_);
    artist_layout_.addWidget(&artist_label_);
    big_layout_.addLayout(&artist_layout_);

    // add album info
    album_layout_.addWidget(&album_title_label_);
    album_layout_.addWidget(&album_label_);
    big_layout_.addLayout(&album_layout_);

    time_layout_.addSpacing(SPACING*2);
    time_layout_.addWidget(&current_time_label_, 0, Qt::AlignLeft);
    time_layout_.addWidget(&total_time_label_, 0, Qt::AlignRight);
    time_layout_.addSpacing(SPACING*2);
    big_layout_.addLayout(&time_layout_);

    big_layout_.addWidget(&progress_bar_, 0, Qt::AlignBottom);

    big_layout_.addWidget(&menu_view_, 0, Qt::AlignBottom);
    big_layout_.addWidget(&status_bar_, 0, Qt::AlignBottom);
}

void OnyxPlayerView::createSongListView()
{
    const int height = defaultItemHeight()+4*SPACING;
    song_list_view_.setPreferItemSize(QSize(-1, height));

    QStandardItemModel * item_model = model_->standardItemModel();
    int rows = item_model->rowCount();
    song_list_data_.clear();
    for (int i=0; i<rows; i++)
    {
        QStandardItem *item = item_model->item(i);
        OData *dd = new OData;
        dd->insert(TAG_TITLE, item->text());
        dd->insert(TAG_FONT_SIZE, 22);
        int alignment = Qt::AlignLeft | Qt::AlignVCenter;
        dd->insert(TAG_ALIGN, alignment);
        dd->insert(TAG_ROW, i);
        song_list_data_.push_back(dd);
    }

    song_list_view_.setSpacing(2);

    int total_height = safeParentWidget(parentWidget())->height();
    setSongListViewFixedGrid(total_height);

    song_list_view_.setData(song_list_data_);
    song_list_view_.setNeighbor(&menu_view_, CatalogView::DOWN);
    song_list_view_.setNeighbor(&menu_view_, CatalogView::RECYCLE_DOWN);
}

void OnyxPlayerView::createMenuView()
{
    menu_view_.setPreferItemSize(MENU_ITEM_SIZE);

    OData *dd = new OData;
    dd->insert(TAG_COVER, normal_mode_pixmap_);
    dd->insert(TAG_MENU_TYPE, MENU_MODE);
    menu_view_datas_.push_back(dd);

    dd = new OData;
    QPixmap prev_pixmap(":/player_icons/previous.png");
    dd->insert(TAG_COVER, prev_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_PREVIOUS);
    menu_view_datas_.push_back(dd);

    dd = new OData;
    dd->insert(TAG_COVER, play_pixmap_);
    dd->insert(TAG_MENU_TYPE, MENU_PLAY);
    menu_view_datas_.push_back(dd);

    dd = new OData;
    QPixmap next_pixmap(":/player_icons/next.png");
    dd->insert(TAG_COVER, next_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_NEXT);
    menu_view_datas_.push_back(dd);

    dd = new OData;
    QPixmap min_pixmap(":/player_icons/minimize.png");
    dd->insert(TAG_COVER, min_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_MINIMIZE);
    menu_view_datas_.push_back(dd);

    dd = new OData;
    QPixmap exit_pixmap(":/player_icons/exit.png");
    dd->insert(TAG_COVER, exit_pixmap);
    dd->insert(TAG_MENU_TYPE, MENU_EXIT);
    menu_view_datas_.push_back(dd);

    menu_view_.setSpacing(2);
    menu_view_.setFixedGrid(1, 6);
    menu_view_.setFixedHeight(MENU_ITEM_SIZE.height() + 2 * SPACING);
    menu_view_.setData(menu_view_datas_);
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

void OnyxPlayerView::keyReleaseEvent(QKeyEvent * ke)
{
    if (ke->key() == Qt::Key_Escape)
    {
        minimize(true);
        ke->accept();
        return;
    }
    OnyxDialog::keyReleaseEvent(ke);
}

void OnyxPlayerView::mouseReleaseEvent(QMouseEvent *me)
{
    QPoint pt = progress_bar_.mapFrom(this, me->pos());
    if (progress_bar_.rect().contains(pt))
    {
        double x = (double)pt.x();
        double width = (double)progress_bar_.width();
        double percentage = x / width;
        int value = (int)(core_->totalTime()*percentage);
        core_->seek(value);
        progress_bar_.setValue(value/1000);
    }
}

void OnyxPlayerView::resizeEvent(QResizeEvent * event)
{
    QSize s = qApp->desktop()->screenGeometry().size();
    if (song_list_view_.visibleSubItems().size() > 0)
    {
        setSongListViewFixedGrid(s.height());
    }
    setFixedSize(s);
}

void OnyxPlayerView::showState(PlayerUtils::State state)
{
    int current_state = MUSIC_PLAYING;
    switch (state)
    {
        case PlayerUtils::Playing:
        {
            qDebug("Playing");
            break;
        }
        case PlayerUtils::Paused:
        {
            qDebug("Paused");
            current_state = MUSIC_PAUSED;
            break;
        }
        case PlayerUtils::Stopped:
        {
            qDebug("Stopped");
            current_state = MUSIC_STOPPED;
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

QString OnyxPlayerView::timeMessage(qint64 time)
{
    QTime qtime(0, 0, 0);
    qtime = qtime.addMSecs((int)time);
    QString msg;
    if (qtime.hour() < 1)
    {
        msg = qtime.toString("mm:ss");
    }
    else
    {
        msg = qtime.toString();
    }
    return msg;
}

int OnyxPlayerView::getStep(qint64 total, qint64 current)
{
    int step = 18;
    if ((total - current) / 1000 < 3)
    {
        step = 4;
    }
    else if (current/1000 < 3)
    {
        step = 2;
    }
    return step;
}

void OnyxPlayerView::setTime(qint64 t)
{
    if (progress_bar_enabled_ && isVisible() && core_->totalTime() >= t)
    {
        static int count = 0;
        if (count == 0 || need_refresh_immediately_)
        {
            need_refresh_immediately_ = false;

            onyx::screen::instance().enableUpdate(false);
            progress_bar_.setValue(t/1000);
            current_time_label_.setText(timeMessage(t));

            // update total time
            bool update_total = false;
            QString total_time_msg = timeMessage(core_->totalTime());
            if (total_time_label_.text() != total_time_msg)
            {
                progress_bar_.setMaximum(core_->totalTime()/1000);
                total_time_label_.setText(timeMessage(core_->totalTime()));
                update_total = true;
            }

            onyx::screen::instance().enableUpdate(true);

            if (update_total)
            {
                total_time_label_.update();
                onyx::screen::watcher().enqueue(&total_time_label_,
                        onyx::screen::ScreenProxy::GU);
            }
            current_time_label_.update();
            onyx::screen::watcher().enqueue(&current_time_label_,
                    onyx::screen::ScreenProxy::GU);
            onyx::screen::watcher().enqueue(&progress_bar_,
                    onyx::screen::ScreenProxy::GU);
        }

        // Wait some seconds to refresh time to save power
        int step = getStep(core_->totalTime(), t);
        if (count >= step)
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
    need_refresh_immediately_ = true;

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
        qint64 total = core_->totalTime();
        onyx::screen::instance().enableUpdate(false);
        progress_bar_.setValue(total/1000);
        current_time_label_.setText(timeMessage(total));
        onyx::screen::instance().enableUpdate(true);

        stop();
        return;
    }

    if (core_->state() != PlayerUtils::Stopped)
    {
        if (core_->state() == PlayerUtils::Paused)
        {
            stop();
        }
        paused_ = false;
        play();
        setPlayPauseIcon();
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
        paused_ = false;
        play();
        setPlayPauseIcon();
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

void OnyxPlayerView::playModeClicked()
{
    ContentView *mode_button_ = menu_view_.visibleSubItems().front();
    bool origin_single_repeat = single_repeat_mode_;
    if (single_repeat_mode_)
    {
        if (mode_button_->data() && mode_button_->data()->contains(TAG_COVER))
        {
            mode_button_->data()->insert(TAG_COVER, shuffle_mode_pixmap_);
        }

        single_repeat_mode_ = false;
        shuffle_mode_ = true;
    }
    else if (shuffle_mode_)
    {
        if (mode_button_->data() && mode_button_->data()->contains(TAG_COVER))
        {
            mode_button_->data()->insert(TAG_COVER, normal_mode_pixmap_);
        }

        single_repeat_mode_ = false;
        shuffle_mode_ = false;
    }
    else
    {
        // normal mode
        if (mode_button_->data() && mode_button_->data()->contains(TAG_COVER))
        {
            mode_button_->data()->insert(TAG_COVER, single_repeat_mode_pixmap_);
        }

        single_repeat_mode_ = true;
        shuffle_mode_ = false;
    }

    model_->prepareForRepeatablePlaying(true);
    model_->prepareForShufflePlaying(shuffle_mode_);

    // for single song repeat
    if (origin_single_repeat != single_repeat_mode_)
    {
        if (!single_repeat_mode_)
        {
            disconnect(core_.get(), SIGNAL(finished()), this, SLOT(play()));
            connect(core_.get(), SIGNAL(finished()), this, SLOT(next()));
        }
        else
        {
            disconnect(core_.get(), SIGNAL(finished()), this, SLOT(next()));
            connect(core_.get(), SIGNAL(finished()), this, SLOT(play()));
        }
    }
    menu_view_.update();
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::setPlayPauseIcon()
{
    ContentView *play_pause = menu_view_.visibleSubItems().at(2);
    OData * data = play_pause->data();
    if (data && data->contains(TAG_COVER))
    {
        data->insert(TAG_COVER, paused_? pause_pixmap_: play_pixmap_);
    }
    menu_view_.update();
    onyx::screen::watcher().enqueue(&menu_view_, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::setCheckedPlayingSong(const int current_page)
{
    int current_row = model_->currentRow();
    QModelIndex idx = model_->standardItemModel()->index(current_row, 0);
    if (idx.isValid())
    {
        if (0 != fixed_grid_rows_)
        {
            // page -> one base index
            int page = current_row/fixed_grid_rows_ + 1;
            if (page == current_page)
            {
                int actual_rows = current_row;
                if (actual_rows >= fixed_grid_rows_)
                {
                    actual_rows = current_row%fixed_grid_rows_;
                }
                song_list_view_.setCheckedTo(actual_rows, 0);
            }
        }
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

    setPlayPauseIcon();
}

void OnyxPlayerView::onNextClicked(bool)
{
    next();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onPrevClicked(bool)
{
    previous();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
}

void OnyxPlayerView::onPositionChanged(const int current, const int total)
{
    setCheckedPlayingSong(current);
    status_bar_.setProgress(current, total);
}

void OnyxPlayerView::onPagebarClicked(const int percentage, const int value)
{
    setCheckedPlayingSong(value);
    song_list_view_.gotoPage(value);
}

void OnyxPlayerView::onSystemVolumeChanged(int value, bool muted)
{
    qDebug("Volume Change:%d", value);
    core_->setVolume(value, value);
}

OData * OnyxPlayerView::getCurrentData(int row)
{
    int size = song_list_data_.size();
    OData *target;
    for (int i=0; i<size; i++)
    {
        OData *dd = song_list_data_.at(i);
        if (dd->contains(TAG_ROW))
        {
            bool ok = false;
            int item_row = dd->value(TAG_ROW).toInt(&ok);
            if (ok && item_row == row)
            {
                target = dd;
                break;
            }
        }
    }
    return target;
}

void OnyxPlayerView::onCurrentChanged()
{
    int current_row = model_->currentRow();
    QModelIndex idx = model_->standardItemModel()->index(current_row, 0);
    if (idx.isValid())
    {
        QStandardItem *info_item = model_->standardItemModel()->item(current_row, 1);
        QString title_info = info_item->text();
        if (title_info.isEmpty())
        {
            info_item = model_->standardItemModel()->item(current_row, 0);
            title_info = info_item->text();
        }
        title_label_.setText(title_info);

        // set artist and album info
        info_item = model_->standardItemModel()->item(current_row, 2);
        artist_label_.setText(info_item->text());
        info_item = model_->standardItemModel()->item(current_row, 3);
        album_label_.setText(info_item->text());

        OData *target_data = getCurrentData(current_row);
        song_list_view_.select(target_data);
        int actual_rows = current_row;
        if (0 != fixed_grid_rows_ && actual_rows >= fixed_grid_rows_)
        {
            actual_rows = current_row%fixed_grid_rows_;
        }
        song_list_view_.setCheckedTo(actual_rows, 0);

        // init progress bar
        progress_bar_.setMinimum(0);
        progress_bar_.setMaximum(core_->totalTime()/1000);
        progress_bar_.setValue(0);

        // set total time
        total_time_label_.setText(timeMessage(core_->totalTime()));

        if (isVisible())
        {
            progress_bar_.update();
            onyx::screen::watcher().enqueue(&progress_bar_,
                    onyx::screen::ScreenProxy::GU);
            total_time_label_.update();
            onyx::screen::watcher().enqueue(&total_time_label_,
                    onyx::screen::ScreenProxy::GU);
        }
    }
}

void OnyxPlayerView::onProgressClicked(const int percent, const int value)
{
    core_->seek(value);
}

void OnyxPlayerView::setSongListViewFixedGrid(int total_height)
{
    int height_left = total_height - 9 * defaultItemHeight();
    int single_height = defaultItemHeight() + 5 * SPACING;
    fixed_grid_rows_ = height_left / single_height;

    song_list_view_.setFixedGrid(fixed_grid_rows_, 1);
    song_list_view_.setFixedHeight(single_height * fixed_grid_rows_);
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
            playModeClicked();
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
            update();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
        }
    }
}

void OnyxPlayerView::playFile(const QString & file_path)
{
    QFileInfo file_info(file_path);
    QString file_name = file_info.fileName();

    int size = song_list_data_.size();
    int target_row = -1;
    for (int i=0; i<size; i++)
    {
        OData *dd = song_list_data_.at(i);
        if (dd->contains(TAG_TITLE))
        {
            if (file_name == dd->value(TAG_TITLE).toString())
            {
                target_row = dd->value(TAG_ROW).toInt();
                break;
            }
        }
    }

    if (target_row >= 0 && target_row < size)
    {

        model_->setCurrent(target_row);
        play();
    }
}

}

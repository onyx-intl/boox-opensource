#include <model/playlistitem.h>
#include <core/fileinfo.h>

#include "player_view.h"

using namespace ui;

namespace player
{

static const QString TOOL_BUTTON_STYLE =   "\
QPushButton                             \
{                                       \
    background: transparent;            \
    font-size: 14px;                    \
    border-width: 1px;                  \
    border-color: transparent;          \
    border-style: solid;                \
    color: black;                       \
    padding: 0px;                       \
}                                       \
QPushButton:pressed                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:checked                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:focus {                     \
    border-width: 2px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 3;                   \
}                                       \
QPushButton:disabled                    \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: dark;                 \
    color: dark;                        \
    background-color: white;            \
}";

static const QString PAGE_BUTTON_STYLE =   "\
QPushButton                             \
{                                       \
    background: transparent;            \
    font-size: 14px;                    \
    border-width: 1px;                  \
    border-color: transparent;          \
    border-style: solid;                \
    color: black;                       \
    padding: 0px;                       \
}                                       \
QPushButton:pressed                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:focus {                     \
    border-width: 2px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 3;                   \
}                                       \
QPushButton:checked                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: white;                       \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:disabled                    \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: dark;                 \
    color: dark;                        \
    background-color: white;            \
}";

static QIcon loadIcon(const QString & path, QSize & size)
{
    QPixmap pixmap(path);
    size = pixmap.size();
    return QIcon(pixmap);
}

PlayerView::PlayerView(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , model_(0)
    , core_(new SoundCore(this))
    , vbox_(this)
    , title_widget_(this)
    , title_layout_(&title_widget_)
    , title_icon_(0)
    , title_label_(0)
    , minimize_button_("", 0)
    , close_button_("", 0)
    , play_list_view_(0, 0)
    , toolbar_layout_(0)
    , prev_page_button_("", 0)
    , next_page_button_("", 0)
    , toolbar_widget_(0)
    , cycle_button_("", 0)
    , shuffle_button_("", 0)
    , prev_button_("", 0)
    , play_pause_button_("", 0)
    , stop_button_("", 0)
    , next_button_("", 0)
    , repeat_button_("", 0)
    , status_bar_(this, MENU | PROGRESS | BATTERY)
    , repeat_(false)
    , seeking_(false)
    , paused_(false)
    , progress_bar_enabled_(true)
    , skips_(0)
    , previous_page_(1)
{
#ifndef Q_WS_QWS
    resize(500, 600);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif
    setWindowTitle(QCoreApplication::tr("Boox Player"));

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
    updateEQ();
    previous_page_ = play_list_view_.currentPage();
}

PlayerView::~PlayerView()
{
}

void PlayerView::loadSettings()
{
    int volume = sys::SysStatus::instance().volume();
    qDebug("Read volume in sys conf:%d", volume);
    core_->setVolume(volume, volume);

    // Repeat/Shuffle
    model_->prepareForRepeatablePlaying(PlayerUtils::isRepeatableList());
    model_->prepareForShufflePlaying(PlayerUtils::isShuffled());
}

void PlayerView::saveSettings()
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
    PlayerUtils::setShuffled(shuffle_button_.isChecked());
    PlayerUtils::setRepeatableList(cycle_button_.isChecked());
}

void PlayerView::attachModel(PlayListModel *m)
{
    model_ = m;
    connect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
    connect(model_, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
    connect(model_, SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));
    connect(model_, SIGNAL(shuffleChanged(bool)), this, SLOT(onShuffleStatusChanged(bool)));
    connect(model_, SIGNAL(repeatableListChanged(bool)), this, SLOT(onRepeatListChanged(bool)));
    connect(&shuffle_button_, SIGNAL(clicked(bool)),
            model_, SLOT(prepareForShufflePlaying(bool)), Qt::QueuedConnection);
    connect(&cycle_button_, SIGNAL(clicked(bool)),
            model_, SLOT(prepareForRepeatablePlaying(bool)), Qt::QueuedConnection);

    model_->doCurrentVisibleRequest();
    loadSettings();
}

void PlayerView::deattachModel()
{
    if (model_ != 0)
    {
        play_list_view_.clear();
        disconnect(model_, SIGNAL(firstAdded()), this, SLOT(play()));
        disconnect(model_, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
        disconnect(model_, SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));
        disconnect(model_, SIGNAL(shuffleChanged(bool)), this, SLOT(onShuffleStatusChanged(bool)));
        disconnect(model_, SIGNAL(repeatableListChanged(bool)), this, SLOT(onRepeatListChanged(bool)));
        disconnect(&shuffle_button_, SIGNAL(clicked(bool)), model_, SLOT(prepareForShufflePlaying(bool)));
        disconnect(&cycle_button_, SIGNAL(clicked(bool)), model_, SLOT(prepareForRepeatablePlaying(bool)));
        model_ = 0;
    }
}

void PlayerView::createLayout()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    vbox_.setSpacing(0);
    vbox_.setContentsMargins(0, 0, 0, 0);

    title_layout_.setSpacing(2);
    title_layout_.setContentsMargins(5, 0, 5, 0);
    title_icon_.setPixmap(QPixmap(":/player_icons/music_player.png"));
    QFont font(title_label_.font());
    font.setPointSize(20);
    title_label_.setFont(font);
    title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    title_label_.setText(QCoreApplication::tr("Music Player"));
    title_layout_.addWidget(&title_icon_, 10);
    title_layout_.addWidget(&title_label_, 500);

    QSize icon_size;
    minimize_button_.setStyleSheet(PAGE_BUTTON_STYLE);
    QIcon minimize_icon = loadIcon(":/player_icons/minimize.png", icon_size);
    minimize_button_.setIconSize(icon_size);
    minimize_button_.setIcon(minimize_icon);
    minimize_button_.setFocusPolicy(Qt::TabFocus);
    title_layout_.addWidget(&minimize_button_);

    close_button_.setStyleSheet(PAGE_BUTTON_STYLE);
    QIcon close_icon = loadIcon(":/player_icons/close.png", icon_size);
    close_button_.setIconSize(icon_size);
    close_button_.setIcon(close_icon);
    close_button_.setFocusPolicy(Qt::TabFocus);
    title_layout_.addWidget(&close_button_);

    // title widget.
    title_widget_.setAutoFillBackground(true);
    title_widget_.setBackgroundRole(QPalette::Dark);
    title_widget_.setContentsMargins(0, 0, 0, 0);
    vbox_.addWidget(&title_widget_);

    // tree widget.
    play_list_view_.showHeader(true);
    vbox_.addWidget(&play_list_view_);

    toolbar_layout_.setSpacing(5);
    toolbar_layout_.setContentsMargins(0, 0, 0, 0);

    // previous page
    prev_page_button_.setStyleSheet(PAGE_BUTTON_STYLE);
    QPixmap prev_page_pixmap(":/player_icons/prev_page.png");
    prev_page_button_.setIconSize(prev_page_pixmap.size());
    prev_page_button_.setIcon(QIcon(prev_page_pixmap));
    prev_page_button_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    prev_page_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_layout_.addWidget(&prev_page_button_);

    // toolbar buttons
    // cycle button
    cycle_button_.setStyleSheet(TOOL_BUTTON_STYLE);
    cycle_button_.setCheckable(true);

    cycle_icon_ = loadIcon(":/player_icons/cycle.png", icon_size);
    cycle_selected_icon_ = loadIcon(":/player_icons/cycle_selected.png", icon_size);
    cycle_button_.setIconSize(icon_size);
    cycle_button_.setIcon(cycle_icon_);
    cycle_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&cycle_button_);

    // shuffle button
    shuffle_button_.setStyleSheet(TOOL_BUTTON_STYLE);
    shuffle_button_.setCheckable(true);

    shuffle_icon_ = loadIcon(":/player_icons/shuffle.png", icon_size);
    normal_icon_ = loadIcon(":/player_icons/normal.png", icon_size);
    shuffle_button_.setIconSize(icon_size);
    shuffle_button_.setIcon(normal_icon_);
    shuffle_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&shuffle_button_);

    // "prev" button
    prev_button_.setStyleSheet(TOOL_BUTTON_STYLE);

    prev_icon_ = loadIcon(":/player_icons/previous.png", icon_size);
    prev_selected_icon_ = loadIcon(":/player_icons/previous_selected.png", icon_size);
    prev_button_.setIconSize(icon_size);
    prev_button_.setIcon(prev_icon_);
    prev_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&prev_button_);

    // play-pause button
    play_pause_button_.setStyleSheet(TOOL_BUTTON_STYLE);

    pause_icon_ = loadIcon(":/player_icons/pause.png", icon_size);
    play_icon_ = loadIcon(":/player_icons/play.png", icon_size);
    play_pause_button_.setIconSize(icon_size);
    play_pause_button_.setIcon(pause_icon_);
    play_pause_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&play_pause_button_);

    // stop button
    stop_button_.setStyleSheet(TOOL_BUTTON_STYLE);

    stop_icon_ = loadIcon(":/player_icons/stop.png", icon_size);
    stop_selected_icon_ = loadIcon(":/player_icons/stop_selected.png", icon_size);
    stop_button_.setIconSize(icon_size);
    stop_button_.setIcon(stop_icon_);
    stop_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&stop_button_);

    // "next" button
    next_button_.setStyleSheet(TOOL_BUTTON_STYLE);

    next_icon_ = loadIcon(":/player_icons/next.png", icon_size);
    next_selected_icon_ = loadIcon(":/player_icons/next_selected.png", icon_size);
    next_button_.setIconSize(icon_size);
    next_button_.setIcon(next_icon_);
    next_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&next_button_);

    // repeat button
    repeat_button_.setStyleSheet(TOOL_BUTTON_STYLE);
    repeat_button_.setCheckable(true);

    repeat_icon_ = loadIcon(":/player_icons/repeat.png", icon_size);
    repeat_selected_icon_ = loadIcon(":/player_icons/repeat_selected.png", icon_size);
    repeat_button_.setIconSize(icon_size);
    repeat_button_.setIcon(repeat_icon_);
    repeat_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_widget_.pushButton(&repeat_button_);

    toolbar_layout_.addWidget(&toolbar_widget_);

    // next page
    next_page_button_.setStyleSheet(PAGE_BUTTON_STYLE);
    QPixmap next_page_pixmap(":/player_icons/next_page.png");
    next_page_button_.setIconSize(next_page_pixmap.size());
    next_page_button_.setIcon(QIcon(next_page_pixmap));
    next_page_button_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    next_page_button_.setFocusPolicy(Qt::TabFocus);
    toolbar_layout_.addWidget(&next_page_button_);

    vbox_.addLayout(&toolbar_layout_);
    vbox_.addWidget(&status_bar_);

    // Connections.
    connect(&close_button_, SIGNAL(clicked(bool)),
            this, SLOT(close(bool)), Qt::QueuedConnection);
    connect(&minimize_button_, SIGNAL(clicked(bool)),
            this, SLOT(minimize(bool)), Qt::QueuedConnection);
    connect(&play_list_view_, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onItemActivated(const QModelIndex &)));
    connect(&play_list_view_, SIGNAL(positionChanged(int, int)),
                this, SLOT(refreshPlayListView(int, int)));
    connect(&status_bar_, SIGNAL(progressClicked(const int, const int)),
            this, SLOT(onProgressClicked(const int, const int)), Qt::QueuedConnection);
    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));

    connect(&play_pause_button_, SIGNAL(clicked(bool)),
            this, SLOT(onPlayPauseClicked(bool)), Qt::QueuedConnection);
    connect(&stop_button_, SIGNAL(clicked(bool)),
            this, SLOT(onStopClicked(bool)), Qt::QueuedConnection);
    connect(&next_button_, SIGNAL(clicked(bool)),
            this, SLOT(onNextClicked(bool)), Qt::QueuedConnection);
    connect(&prev_button_, SIGNAL(clicked(bool)),
            this, SLOT(onPrevClicked(bool)), Qt::QueuedConnection);
    connect(&repeat_button_, SIGNAL(clicked(bool)),
            this, SLOT(onRepeatClicked(bool)), Qt::QueuedConnection);

    connect(&prev_page_button_, SIGNAL(clicked(bool)),
            this, SLOT(prevPage()), Qt::QueuedConnection);
    connect(&next_page_button_, SIGNAL(clicked(bool)),
            this, SLOT(nextPage()), Qt::QueuedConnection);
}

void PlayerView::showState(PlayerUtils::State state)
{
    int current_state = MUSIC_PLAYING;
    switch (state)
    {
        case PlayerUtils::Playing:
        {
            qDebug("Playing");
            play_pause_button_.setIcon(pause_icon_);
            break;
        }
        case PlayerUtils::Paused:
        {
            qDebug("Paused");
            current_state = MUSIC_PAUSED;
            play_pause_button_.setIcon(play_icon_);
            break;
        }
        case PlayerUtils::Stopped:
        {
            qDebug("Stopped");
            current_state = MUSIC_STOPPED;
            play_pause_button_.setIcon(play_icon_);
            break;
        }
    }
    emit stateChanged(current_state);
}

void PlayerView::showMetaData()
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

void PlayerView::onLoadingFinished()
{
    play_list_view_.setModel(model_->standardItemModel());
    QVector<int> percentages;
    percentages.push_back(50);
    percentages.push_back(25);
    percentages.push_back(25);
    play_list_view_.setColumnWidth(percentages);

    onCurrentChanged();
    if ( sys::SysStatus::instance().isSystemBusy() )
    {
        // if it is the first time rendering, set busy to be false
        sys::SysStatus::instance().setSystemBusy( false );
    }
    play_list_view_.update();
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
}

void PlayerView::setProgress(int p)
{
    qDebug("Progress:%d", p);
    core_->seek(p);
}

void PlayerView::refreshPlayListView(int currentPage, int totalPage)
{

    if (previous_page_ != currentPage)
    {
        onyx::screen::instance().flush(&play_list_view_,
                onyx::screen::ScreenProxy::GC, false);
    }
    previous_page_ = currentPage;
}

void PlayerView::onProgressClicked(const int percent, const int value)
{
    core_->seek(value);
}

void PlayerView::nextPage()
{
    play_list_view_.pageDown();
}

void PlayerView::prevPage()
{
    play_list_view_.pageUp();
}

void PlayerView::updateEQ()
{
    // TODO. Implement Me
}

void PlayerView::setTime(qint64 t)
{
    if (progress_bar_enabled_ && isVisible() && core_->totalTime() >= t)
    {
        static int count = 0;
        if (count == 0)
        {
            onyx::screen::instance().enableUpdate(false);
            //qDebug("Set Time:%d, %d", (int)t, int(core_->totalTime()));
            status_bar_.setProgress(t, core_->totalTime());
            onyx::screen::instance().enableUpdate(true);
            onyx::screen::instance().updateWidget(&status_bar_, onyx::screen::ScreenProxy::DW, false, onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH);
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

void PlayerView::close(bool)
{
    hide();
    stop();
    saveSettings();

    emit stateChanged(STOP_PLAYER);
    qApp->exit();
}

void PlayerView::minimize(bool)
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

void PlayerView::onPlayPauseClicked(bool)
{
    if (core_->state() == PlayerUtils::Playing)
    {
        pause();
    }
    else if (core_->state() == PlayerUtils::Paused ||
             core_->state() == PlayerUtils::Stopped)
    {
        play();
    }
    onyx::screen::instance().flush(&play_pause_button_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::onStopClicked(bool)
{
    stop();
    onyx::screen::instance().flush(&stop_button_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::onNextClicked(bool)
{
    next();
    onyx::screen::instance().flush(&toolbar_widget_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::onPrevClicked(bool)
{
    previous();
    onyx::screen::instance().flush(&toolbar_widget_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::onRepeatClicked(bool r)
{
    if (r != repeat_ && !r)
    {
        disconnect(core_.get(), SIGNAL(finished()), this, SLOT(play()));
        connect(core_.get(), SIGNAL(finished()), this, SLOT(next()));
        repeat_button_.setIcon(repeat_icon_);
    }
    else if (r != repeat_ && r)
    {
        disconnect(core_.get(), SIGNAL(finished()), this, SLOT(next()));
        connect(core_.get(), SIGNAL(finished()), this, SLOT(play()));
        repeat_button_.setIcon(repeat_selected_icon_);
    }
    repeat_ = r;
    emit repeatableChanged(r);
    onyx::screen::instance().flush(&repeat_button_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::onItemActivated(const QModelIndex & index)
{
    QStandardItem *item = model_->standardItemModel()->itemFromIndex(index);
    if (item != 0)
    {
        int current_row = item->data().toInt();
        model_->setCurrent(current_row);
        play();
    }
}

void PlayerView::onSystemVolumeChanged(int value, bool muted)
{
    qDebug("Volume Change:%d", value);
    core_->setVolume(value, value);
}

void PlayerView::onCurrentChanged()
{
    int current_row = model_->currentRow();
    QModelIndex idx = model_->standardItemModel()->index(current_row, 0);
    if (idx.isValid())
    {
        play_list_view_.select(idx);
    }
}

void PlayerView::onShuffleStatusChanged(bool yes)
{
    shuffle_button_.setIcon(yes ? normal_icon_ : shuffle_icon_);
    shuffle_button_.setChecked(yes);
    onyx::screen::instance().flush(&shuffle_button_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::onRepeatListChanged(bool yes)
{
    cycle_button_.setIcon(yes ? cycle_selected_icon_ : cycle_icon_);
    cycle_button_.setChecked(yes);
    onyx::screen::instance().flush(&cycle_button_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::play()
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

void PlayerView::stop()
{
    core_->stop();
}

void PlayerView::next()
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
    onyx::screen::instance().flush(&play_list_view_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::previous()
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
    onyx::screen::instance().flush(&play_list_view_, onyx::screen::ScreenProxy::GC, false);
}

void PlayerView::pause()
{
    core_->pause();
}

void PlayerView::popupMenu()
{
    if ( onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW )
    {
        // Stop fastest update mode to get better image quality.
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }

    ui::PopupMenu menu(this);

    // system actions
    std::vector<int> sys_actions;
    sys_actions.push_back(ROTATE_SCREEN);
    sys_actions.push_back(SCREEN_UPDATE_TYPE);
    sys_actions.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(sys_actions);
    menu.setSystemAction(&system_actions_);

    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
        case RETURN_TO_LIBRARY:
            close(true);
            break;
        case SCREEN_UPDATE_TYPE:
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
            onyx::screen::instance().toggleWaveform();
            update();
            break;
        case ROTATE_SCREEN:
            SysStatus::instance().rotateScreen();
            break;
        default:
            break;
        }
    }
}

void PlayerView::changeEvent(QEvent *ce)
{
    QWidget::changeEvent(ce);
}

void PlayerView::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void PlayerView::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget * wnd = 0;
    switch(ke->key())
    {
    case Qt::Key_PageDown:
        {
            nextPage();
        }
        break;
    case Qt::Key_PageUp:
        {
            prevPage();
        }
        break;
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
        {
            wnd = ui::moveFocus(this, ke->key());
            if (wnd)
            {
                wnd->setFocus();
            }
        }
        break;
    case Qt::Key_R:
        {
            emit testReload();
        }
        break;
    case Qt::Key_Escape:
        {
            close(true);
        }
        break;
    case ui::Device_Menu_Key:
        {
            popupMenu();
        }
        break;
    default:
        break;
    }
}

bool PlayerView::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    //qDebug("main window event type %d", event->type());
    if (event->type() == QEvent::UpdateRequest &&
        onyx::screen::instance().isUpdateEnabled() &&
        isActiveWindow())
    {
        static int count = 0;
        qDebug("Update request %d", ++count);
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        event->accept();
        return true;
    }
    return ret;
}

}

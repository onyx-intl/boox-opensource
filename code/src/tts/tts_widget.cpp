
#include "tts.h"
#include "tts_widget.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys.h"

using namespace ui;

namespace tts
{

static const QString TTS_BUTTON_STYLE =   "\
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

TTSWidget::TTSWidget(QWidget *parent, TTS & ref)
    : QDialog(parent, Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint)
    , tts_(ref)
    , layout_(this)
    , menu_button_(tr(""), 0)
    , play_button_(tr(""), 0)
    , close_button_(tr(""), 0)
{
    createLayout();
    setModal(false);
    setBackgroundRole(QPalette::Dark);
    setFocusPolicy(Qt::NoFocus);

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(volumeChanged(int, bool)), this, SLOT(onSystemVolumeChanged(int, bool)));
}

TTSWidget::~TTSWidget()
{
}

void TTSWidget::ensureVisible()
{
    if (!isVisible())
    {
        show();
    }

    QRect parent_rect = parentWidget()->rect();
    int border = (frameGeometry().width() - geometry().width());

    // Check position.
    QPoint new_pos(border, border);
    new_pos.ry() = parent_rect.height() - height() - border * 2;
    update_parent_ = true;
    if (pos() != new_pos)
    {
        move(new_pos);
    }

    // Make sure the widget is visible.
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GU);
}

void TTSWidget::onSystemVolumeChanged(int value, bool muted)
{
    qDebug("Volume Change:%d", value);
    tts_.sound().setVolume(value);
}

bool TTSWidget::speak(const QString & text)
{
    // Make sure the widget is visible.
    onyx::screen::instance().ensureUpdateFinished();

    // Remember the message.
    text_ = text;
    startPlaying();
    return true;
}

void TTSWidget::startPlaying()
{
    tts::TTS_State prev_state = tts_.state();
    tts_.setState(tts::TTS_PLAYING);

    //if (prev_state == tts::TTS_STOPPED)
    {
        tts_.speak(text_);
    }
    /*else if (prev_state == tts::TTS_PAUSED)
    {
        resume();
    }*/

    play_button_.setIcon(stop_icon_);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
}

bool TTSWidget::pause()
{
    play_button_.setIcon(play_icon_);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
    return tts_.pause();
}

bool TTSWidget::resume()
{
    play_button_.setIcon(stop_icon_);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
    return tts_.resume();
}

bool TTSWidget::toggle()
{
    return tts_.toggle();
}

bool TTSWidget::stop()
{
    play_button_.setIcon(play_icon_);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
    return tts_.stop();
}

void TTSWidget::changeVolume(int v, bool m)
{
    tts_.changeVolume(v, m);
}

bool TTSWidget::event(QEvent *e)
{
    int ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        static int count = 0;
        qDebug("tts widget update %d", ++count);
        if (update_parent_)
        {
            onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GC);
            update_parent_ = false;
        }
        else
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        }
        e->accept();
    }
    return ret;
}

void TTSWidget::moveEvent(QMoveEvent *e)
{
    update_parent_ = true;
}

void TTSWidget::resizeEvent(QResizeEvent *e)
{
    update_parent_ = true;
}

static QIcon loadIcon(const QString & path, QSize & size)
{
    QPixmap pixmap(path);
    size = pixmap.size();
    return QIcon(pixmap);
}

void TTSWidget::createLayout()
{
    // top box
    layout_.setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout_.setContentsMargins(2, 2, 2, 2);
    layout_.setSpacing(10);

    menu_button_.setStyleSheet(TTS_BUTTON_STYLE);
    QPixmap menu_map(":/images/tts_menu.png");
    menu_button_.setIcon(QIcon(menu_map));
    menu_button_.setIconSize(menu_map.size());
    menu_button_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    play_button_.setStyleSheet(TTS_BUTTON_STYLE);
    QSize icon_size;
    play_icon_ = loadIcon(":/images/tts_play.png", icon_size);
    stop_icon_ = loadIcon(":/images/tts_stop.png", icon_size);

    play_button_.setIcon(stop_icon_);
    play_button_.setIconSize(icon_size);
    play_button_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    close_button_.setStyleSheet(TTS_BUTTON_STYLE);
    QPixmap close_map(":/images/tts_close.png");
    close_button_.setIcon(QIcon(close_map));
    close_button_.setIconSize(close_map.size());
    close_button_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    layout_.addWidget(&menu_button_);
    layout_.addWidget(&play_button_);
    layout_.addWidget(&close_button_);

    // Setup connection.
    connect(&menu_button_, SIGNAL(clicked(bool)), this, SLOT(onPopupMenu(bool)), Qt::QueuedConnection);
    connect(&play_button_, SIGNAL(clicked(bool)), this, SLOT(onPlayClicked(bool)), Qt::QueuedConnection);
    connect(&close_button_, SIGNAL(clicked(bool)), this, SLOT(onCloseClicked(bool)), Qt::QueuedConnection);
    connect(&tts_, SIGNAL(speakDone()), this, SLOT(onTextPlayed()));

}

void TTSWidget::onPlayClicked(bool)
{
    tts_.toggle();
    if (state() == TTS_PLAYING)
    {
        startPlaying();
    }
    else if (state() == TTS_PAUSED)
    {
        pause();
    }
}

void TTSWidget::updateActions()
{
    QStringList speakers;
    if (tts_.speakers(speakers))
    {
        QString current_speaker;
        tts_.currentSpeaker(current_speaker);
        speaker_actions_.generateActions(speakers, current_speaker);
    }

    QVector<int> speeds;
    if (tts_.speeds(speeds))
    {
        int current_speed = 2;
        tts_.currentSpeed(current_speed);
        speed_actions_.generateActions(speeds, current_speed);
    }

    QVector<int> styles;
    if (tts_.styles(styles))
    {
        int current_style = SPEAK_STYLE_NORMAL;
        tts_.currentStyle(current_style);
        style_actions_.generateActions(styles, current_style);
    }
}

void TTSWidget::onPopupMenu(bool)
{
    // Make sure the display update is finished, otherwise
    // user can not see the menu on the screen.
    onyx::screen::instance().ensureUpdateFinished();
    ui::PopupMenu menu(0);
    updateActions();
    menu.addGroup(&speaker_actions_);
    menu.addGroup(&speed_actions_);
    menu.addGroup(&style_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    update();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);

    QAction * group = menu.selectedCategory();
    if (group == speaker_actions_.category())
    {
        tts_.setSpeaker(speaker_actions_.selectedSpeaker());
    }
    else if (group == speed_actions_.category())
    {
        tts_.setSpeed(speed_actions_.selectedSpeed());
    }
    else if (group == style_actions_.category())
    {
        tts_.setStyle(style_actions_.selectedStyle());
    }
}

void TTSWidget::keyPressEvent(QKeyEvent *ke)
{
    int key = ke->key();
    if (key == Qt::Key_Escape)
    {
        ke->ignore();
        return;
    }
    QDialog::keyPressEvent(ke);
}

void TTSWidget::keyReleaseEvent(QKeyEvent *ke)
{
    int key = ke->key();
    if (key == Qt::Key_Escape)
    {
        onCloseClicked(true);
        ke->ignore();
        return;
    }
    else if (key == ui::Device_Menu_Key)
    {
        onPopupMenu(true);
    }
    ke->accept();
}

void TTSWidget::onCloseClicked(bool)
{
    qDebug("Close TTS Widget");
    if (!isVisible())
    {
        return;
    }

    stop();
    emit closed();
    done(QDialog::Rejected);
}

/// On sentence played, we can play the next sentence now.
/// If there is no more sentece, playFinished signal is emitted.
void TTSWidget::onTextPlayed()
{
    // Check state.
    if (!tts_.isPlaying())
    {
        return;
    }

    emit speakDone();
}

}

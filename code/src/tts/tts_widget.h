#ifndef NABOO_LIB_TTS_WIDGET_H_
#define NABOO_LIB_TTS_WIDGET_H_

#include "onyx/base/base.h"
#include "sound/sound.h"
#include "tts_interface.h"
#include "tts.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/tts_actions.h"

namespace tts
{

/// TTS widget.
class TTSWidget : public QDialog
{
    Q_OBJECT

public:
    TTSWidget(QWidget *parent, TTS & ref);
    ~TTSWidget();

public:
    TTS_State state() { return tts_.state(); }

    void setData(const QVariant & data) { data_ = data; }
    QVariant & data() { return data_; }

    void ensureVisible();

public Q_SLOTS:
    bool speak(const QString & text);
    bool pause();
    bool resume();
    bool toggle();
    bool stop();
    void changeVolume(int, bool);
    void onCloseClicked(bool);

Q_SIGNALS:
    void speakDone();
    void closed();

private Q_SLOTS:
    void onPlayClicked(bool);
    void startPlaying();
    void onTextPlayed();
    void onPopupMenu(bool);
    void onSystemVolumeChanged(int value, bool muted);

private:
    bool event(QEvent *e);
    void moveEvent(QMoveEvent *e);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    void resizeEvent(QResizeEvent *e);
    void createLayout();

    void updateActions();

private:
    TTS & tts_;
    QHBoxLayout   layout_;

    ui::OnyxPushButton menu_button_;
    ui::OnyxPushButton play_button_;
    ui::OnyxPushButton close_button_;
    QButtonGroup       button_group_;

    QIcon              play_icon_;
    QIcon              stop_icon_;

    QVariant   data_;
    QString    text_;
    bool       update_parent_;

    ui::TTSSpeakerActions speaker_actions_;
    ui::TTSSpeedActions   speed_actions_;
    ui::TTSStyleActions   style_actions_;
};

}   // namespace tts

#endif  // NABOO_LIB_TTS_WIDGET_H_

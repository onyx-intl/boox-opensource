#ifndef PLAYER_VIEW_H_
#define PLAYER_VIEW_H_

#include <utils/player_utils.h>
#include <utils/toolbar_widget.h>
#include <core/soundcore.h>
#include <model/playlistmodel.h>

using namespace ui;


namespace player
{

class PlayerView : public QWidget
{
    Q_OBJECT
public:
    PlayerView(QWidget *parent = 0);
    ~PlayerView();

    void attachModel(PlayListModel *m);
    void deattachModel();

    PlayerUtils::State state() { return core_ != 0 ? core_->state() : PlayerUtils::FatalError; }
    qint64 elapsed() { return core_->elapsed(); }
    void enableProgressBar(bool enable) { progress_bar_enabled_ = enable; }

public Q_SLOTS:
    void play();
    void stop();
    void next();
    void previous();
    void pause();
    void close(bool);
    void minimize(bool);

    void onPlayPauseClicked(bool);
    void onStopClicked(bool);
    void onNextClicked(bool);
    void onPrevClicked(bool);
    void onRepeatClicked(bool);

    void onItemActivated(const QModelIndex &);
    void setProgress(int);

    void refreshPlayListView(int currentPage, int totalPage);

Q_SIGNALS:
    void repeatableChanged(bool);
    void stateChanged(int);
    void testReload();

private Q_SLOTS:
    void showState(PlayerUtils::State state);
    void showMetaData();
    void setTime(qint64);
    void onLoadingFinished();
    void onSystemVolumeChanged(int value, bool muted);
    void onCurrentChanged();
    void onProgressClicked(const int percent, const int value);
    void onShuffleStatusChanged(bool yes);
    void onRepeatListChanged(bool yes);

    void nextPage();
    void prevPage();
    void popupMenu();

private:
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    void changeEvent(QEvent *ce);
    bool event(QEvent * event);

    void loadSettings();
    void saveSettings();
    void createLayout();
    void updateEQ();

private:
    PlayListModel           *model_;
    scoped_ptr<SoundCore>   core_;

    QVBoxLayout             vbox_;
    QWidget                 title_widget_;
    QHBoxLayout             title_layout_;
    QLabel                  title_icon_;
    QLabel                  title_label_;
    OnyxPushButton          minimize_button_;
    OnyxPushButton          close_button_;

    OnyxTreeView            play_list_view_;
    QHBoxLayout             toolbar_layout_;

    // page flipping
    OnyxPushButton         prev_page_button_;
    OnyxPushButton         next_page_button_;

    // in toolbar widget
    ToolbarWidget          toolbar_widget_;
    OnyxPushButton         cycle_button_;
    OnyxPushButton         shuffle_button_;
    OnyxPushButton         prev_button_;
    OnyxPushButton         play_pause_button_;
    OnyxPushButton         stop_button_;
    OnyxPushButton         next_button_;
    OnyxPushButton         repeat_button_;

    StatusBar               status_bar_;

    // icons
    QIcon                   repeat_icon_;
    QIcon                   repeat_selected_icon_;
    QIcon                   next_icon_;
    QIcon                   next_selected_icon_;
    QIcon                   normal_icon_;
    QIcon                   shuffle_icon_;
    QIcon                   play_icon_;
    QIcon                   pause_icon_;
    QIcon                   prev_icon_;
    QIcon                   prev_selected_icon_;
    QIcon                   stop_icon_;
    QIcon                   stop_selected_icon_;
    QIcon                   cycle_icon_;
    QIcon                   cycle_selected_icon_;

    // menu actions
    SystemActions           system_actions_;

    // flags
    bool                    repeat_;
    bool                    seeking_;
    bool                    paused_;
    bool                    progress_bar_enabled_;
    int                     skips_;
    int                     previous_page_; ///< store the previous page in page view
};

};
#endif

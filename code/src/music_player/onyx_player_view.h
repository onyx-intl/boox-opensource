#ifndef ONYX_PLAYER_VIEW_H_
#define ONYX_PLAYER_VIEW_H_

#include <utils/player_utils.h>
#include <utils/toolbar_widget.h>
#include <core/soundcore.h>
#include <model/playlistmodel.h>

#include "onyx/base/base.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/content_view.h"
#include "onyx/ui/catalog_view.h"
#include "player_title_bar.h"

using namespace ui;

namespace player
{

class OnyxPlayerView: public OnyxDialog
{
    Q_OBJECT

public:
    OnyxPlayerView(QWidget *parent);
    ~OnyxPlayerView();

    void attachModel(PlayListModel *m);
    void deattachModel();

    PlayerUtils::State state() { return core_ != 0 ? core_->state() : PlayerUtils::FatalError; }
    qint64 elapsed() { return core_->elapsed(); }
    void enableProgressBar(bool enable) { progress_bar_enabled_ = enable; }

public Q_SLOTS:
    void setProgress(int);
    void play();
    void stop();
    void next();
    void previous();
    void pause();
    void close(bool);
    void minimize(bool);

    void playModeClicked();
    void onPlayPauseClicked(bool);
    void onNextClicked(bool);
    void onPrevClicked(bool);

    void onPositionChanged(const int current, const int total);
    void onPagebarClicked(const int percentage, const int value);

Q_SIGNALS:
    void stateChanged(int);
    void testReload();

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item,
            int user_data);

private Q_SLOTS:
    void showState(PlayerUtils::State state);
    void showMetaData();
    void setTime(qint64);

    void onLoadingFinished();
    void onSystemVolumeChanged(int value, bool muted);
    void onCurrentChanged();
    void onProgressClicked(const int percent, const int value);

private:
    void loadSettings();
    void saveSettings();

    void createLayout();
    void createSongListView();
    void createMenuView();
    void connectWithChildren();

    void keyReleaseEvent(QKeyEvent * ke);
    void mouseReleaseEvent(QMouseEvent *me);
    void resizeEvent(QResizeEvent * event);

    QString timeMessage(qint64 time);
    void setSongListViewFixedGrid(int total_height);
    OData * getCurrentData(int row);

    void setPlayPauseIcon();

    void setCheckedPlayingSong(const int current_page);

private:
    PlayListModel           *model_;
    scoped_ptr<SoundCore>   core_;

    PlayerTitleBar player_title_bar_;

    QVBoxLayout big_layout_;
    QHBoxLayout artist_layout_;
    QHBoxLayout title_layout_;
    QHBoxLayout album_layout_;
    QHBoxLayout time_layout_;

    OnyxLabel title_title_label_;
    OnyxLabel title_label_;
    OnyxLabel artist_title_label_;
    OnyxLabel artist_label_;
    OnyxLabel album_title_label_;
    OnyxLabel album_label_;

    OnyxLabel current_time_label_;
    OnyxLabel total_time_label_;

    CatalogView song_list_view_;
    QProgressBar progress_bar_;
    CatalogView menu_view_;

    StatusBar status_bar_;

    QPixmap normal_mode_pixmap_;
    QPixmap single_repeat_mode_pixmap_;
    QPixmap shuffle_mode_pixmap_;
    QPixmap play_pixmap_;
    QPixmap pause_pixmap_;

    // menu actions
    SystemActions system_actions_;

    ODatas song_list_data_;

    // flags
    bool single_repeat_mode_;
    bool shuffle_mode_;
    bool seeking_;
    bool paused_;
    bool progress_bar_enabled_;
    int skips_;
    int previous_page_; ///< store the previous page in page view
    int fixed_grid_rows_;
};

}   // namespace player

#endif

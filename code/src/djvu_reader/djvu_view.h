#ifndef DJVU_VIEW_H_
#define DJVU_VIEW_H_

#include "djvu_utils.h"
#include "djvu_render_proxy.h"
#include "djvu_page.h"

using namespace vbf;
using namespace sketch;
namespace djvu_reader
{

class DjvuModel;
class ThumbnailView;
class DjvuView : public BaseView
{
    Q_OBJECT
public:
    explicit DjvuView(QWidget *parent = 0);
    virtual ~DjvuView(void);

    // Interfaces of BaseView
    virtual void attachModel(BaseModel *model);
    virtual void deattachModel();

public:
    inline DjvuModel* model();

    // Attach necessary views
    void attachMainWindow(MainWindow *main_window);
    void deattachMainWindow(MainWindow *main_window);
    void attachTreeView(TreeViewDialog *tree_view);
    void deattachTreeView(TreeViewDialog *tree_view);
    void attachThumbnailView(ThumbnailView *thumb_view);
    void deattachThumbnailView(ThumbnailView *thumb_view);

    // Return
    void returnToLibrary();

    // Flipping
    bool flip(int direction);

public Q_SLOTS:
    void onPageRenderReady(DjVuPagePtr page);
    void onPagebarClicked(const int, const int);
    void onPopupMenu();

    void slideShowNextPage();
    void onStylusChanges(const int type);
    void onRequestUpdateScreen();

    void autoFlipMultiplePages();

private Q_SLOTS:
    void onDocReady();
    void onDocError(QString msg, QString file_name, int line_no);
    void onDocInfo(QString msg);
    void onDocPageReady();
    void onDocThumbnailReady(int page_num);
    void onDocIdle();

    void onLayoutDone();
    void onNeedPage(const int page_number);
    void onNeedContentArea(const int page_number);

    void onContentAreaReady(DjVuPagePtr page, const QRect & content_area);
    void onSaveAllOptions();

    void onUpdateBookmark();

    // thumbnail
    void onNeedThumbnailForNewPage(const int page_num, const QSize &size);
    void onNeedNextThumbnail(const int page_num, const QSize &size);
    void onNeedPreviousThumbnail(const int page_num, const QSize &size);
    void onThumbnailReturnToReading(const int page_num);

Q_SIGNALS:
    void currentPageChanged(const int, const int);
    void rotateScreen();
    void itemStatusChanged(const StatusBarItemType, const int);
    void requestUpdateParent(bool update_now);
    void popupJumpPageDialog();
    void fullScreen(bool need_full_screen);

    void testSuspend();
    void testWakeUp();

private:
    inline void clearVisiblePages();
    void initLayout();
    void resetLayout();
    void switchLayout(PageLayoutType mode);

    bool generateRenderSetting(vbf::PagePtr page, RenderSettingPtr setting);
    void updateCurrentPage(const int page_number);
    bool hitTest(const QPoint &point);
    bool hitTestBookmark(const QPoint &point);
    void paintPage(QPainter & painter, DjVuPagePtr page);

    // handle page ready events
    void handleNormalPageReady(DjVuPagePtr page);
    void handleThumbnailReady(DjVuPagePtr page);

    // configurations
    bool loadConfiguration(Configuration & conf);
    bool saveConfiguration(Configuration & conf);

    // GUI event handlers.
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *re);

    // sketch
    void attachSketchProxy();
    void deattachSketchProxy();
    void updateSketchProxy();

    // Actions
    bool updateActions();
    void generateZoomSettings( std::vector<ZoomFactor> & zoom_settings );

    // Reading tools
    void displayOutlines( bool );
    void gotoPage(const int page_number);

    // Zoom in
    void zoomIn(const QRect &zoom_rect);
    bool zooming( double zoom_setting );
    void zoomInPress( QMouseEvent *me );
    void zoomInMove( QMouseEvent *me );
    void zoomInRelease( QMouseEvent *me );

    // Zooming
    void selectionZoom();

    // Pan
    void enableScrolling();
    void panPress( QMouseEvent *me );
    void panMove( QMouseEvent *me );
    void panRelease( QMouseEvent *me );
    void scroll(int offset_x, int offset_y);

    // Sketch
    void setSketchMode( const SketchMode mode, bool selected );
    void setSketchColor( const SketchColor color );
    void setSketchShape( const SketchShape shape );
    void paintSketches( QPainter & painter, int page_no );

    // Bookmarks
    void paintBookmark( QPainter & painter );
    void displayBookmarks();
    bool addBookmark();
    bool deleteBookmark();

    // Reading history
    void saveReadingContext();
    void back();
    void forward();

    // Music Player
    void openMusicPlayer();

    // Slide Show
    void startSlideShow();
    void stopSlideShow();

    // rotate
    void rotate();

    // thumbnail
    void displayThumbnailView();

    // check the view mode: portrait or landscape
    bool isLandscape() { return (view_setting_.rotate_orient == ROTATE_90_DEGREE ||
                                 view_setting_.rotate_orient == ROTATE_270_DEGREE); }

    // full screen
    bool isFullScreenCalculatedByWidgetSize();

private:
    DjvuModel               *model_;                    ///< Djvu model
    scoped_ptr<PageLayout>  layout_;                    ///< pages layout
    PageLayoutType          read_mode_;                 ///< current reading mode
    vbf::MarginArea         cur_margin_;                ///< content margins

    SketchProxy             sketch_proxy_;              ///< sketch proxy
    StrokeArea              stroke_area_;               ///< stroke area
    PanArea                 pan_area_;                  ///< pan area
    StatusManager           status_mgr_;                ///< status manager

    ViewSetting             view_setting_;              ///< current view setting
    VisiblePages            layout_pages_;              ///< layout pages
    int                     cur_page_;                  ///< index of current image
    QVector<int>            rendering_pages_;           ///< list of pages' indexes for rendering

    DjvuRenderProxy         render_proxy_;              ///< render proxy
    DisplayPages<QDjVuPage> display_pages_;             ///< vector of displaying pages

    // Popup menu actions
    ZoomSettingActions      zoom_setting_actions_;
    ViewActions             view_actions_;
    ReadingToolsActions     reading_tools_actions_;
    SketchActions           sketch_actions_;
    SystemActions           system_actions_;

    // controller of rendering and repainting
    QTimer                  repaint_timer_;
    QTimer                  slide_timer_;               ///< time controller of the slides show
    scoped_ptr<QRubberBand> rubber_band_;               ///< Rubber band is used in zoom-in mode

    // reading history
    ReadingHistory          reading_history_;
    int                     restore_count_;

    // bookmark
    scoped_ptr<QImage>      bookmark_image_;
    QTimer                  update_bookmark_timer_;
    scoped_ptr<NotesDialog> notes_dialog_;

    // auto flip
    QTimer                  flip_page_timer_;
    int                     auto_flip_current_page_;
    int                     auto_flip_step_;

    // current waveform
    onyx::screen::ScreenProxy::Waveform  current_waveform_;
};

/// clear all of the visible pages
inline void DjvuView::clearVisiblePages()
{
    display_pages_.clear();
}

};

#endif // DJVU_VIEW_H_

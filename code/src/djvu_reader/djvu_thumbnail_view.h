#ifndef DJVU_THUMBNAIL_VIEW_H
#define DJVU_THUMBNAIL_VIEW_H

#include "djvu_utils.h"

using namespace onyx::ui;
using namespace ui;
using namespace vbf;
using namespace sketch;

namespace djvu_reader
{

class DjvuModel;
class ThumbnailView : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailView(QWidget *parent = 0);
    ~ThumbnailView(void);

    void attachMainWindow(MainWindow *main_window);
    void deattachMainWindow(MainWindow *main_window);

    void attachSketchProxy(SketchProxy *sketch_proxy);
    void deattachSketchProxy();

    // render context
    void setModel(DjvuModel *model) { model_ = model; }
    void setCurrentPage(int page_num);
    void setThumbnail(ThumbPtr thumb);
    void setNextThumbnail(ThumbPtr thumb);
    void setPreviousThumbnail(ThumbPtr thumb);

Q_SIGNALS:
    void needThumbnailForNewPage(const int page_num, const QSize &size);
    void needNextThumbnail(const int page_num, const QSize &size);
    void needPreviousThumbnail(const int page_num, const QSize &size);
    void returnToReading(const int page_num = -1);
    void popupJumpPageDialog();

    void positionChanged(const int current, const int total);
    void needFullScreen(bool enable);

private Q_SLOTS:
    void onPagebarClicked(const int percent, const int value);
    void onPopupContextMenu();

private:
    // message handlers
    bool event(QEvent * event);
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *re);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *ke);

private:
    void paintPage(QPainter &painter, ThumbPtr page, int index);
    void paintTitle(QPainter &painter, ThumbPtr page, int index);
    void paintSketches(QPainter &painter, ThumbPtr page, int index);
    void paintBoundingRect(QPainter &painter, const ThumbnailPage& thumb);

    bool reachedFirstScreen(ThumbPtr page);
    bool reachedLastScreen(ThumbPtr page);
    void nextScreen();
    void prevScreen();
    void readyToShow();
    void rotate();

    QRect getPageDisplayArea(ThumbPtr page, int index);

    void moveCurrentPage(const int next_num);

    void initializePopupMenuActions();
    void popupMenu();

    // get thumbnail layout page by key
    ThumbnailPage& getLayoutPage(const int key);

private:
    DjvuModel*                  model_;
    ThumbnailLayout             layout_;
    SketchProxy                 *sketch_proxy_;
    DisplayPages<BaseThumbnail> display_pages_;
    DisplayPages<BaseThumbnail> waiting_pages_;
    int                         cur_thumb_index_;

    QPoint                      mouse_press_pos_;
    SystemActions               system_actions_;
};

};
#endif

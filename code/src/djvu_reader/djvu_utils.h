#ifndef DJVU_UTILS_H_
#define DJVU_UTILS_H_

#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "onyx/base/down_cast.h"
#include "onyx/base/base_model.h"

#include "onyx/ui/ui.h"
#include "onyx/ui/thumbnail_layout.h"
#include "onyx/ui/base_thumbnail.h"
#include "onyx/ui/base_view.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/main_window.h"
#include "onyx/ui/display_pages_container.h"
#include "onyx/ui/page_layout.h"
#include "onyx/ui/continuous_page_layout.h"
#include "onyx/ui/single_page_layout.h"
#include "onyx/ui/notes_dialog.h"

#include "onyx/cms/content_thumbnail.h"

#include "onyx/data/bookmark.h"
#include "onyx/data/reading_history.h"

#include "onyx/ui/status_manager.h"

#include "onyx/data/sketch_proxy.h"

#include "onyx/sys/sys.h"
#include "onyx/sys/sys_conf.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

#include "libdjvu/ddjvuapi.h"
#include "libdjvu/miniexp.h"

namespace djvu_reader
{

enum DjvuViewType
{
    DJVU_VIEW = 0,
    TOC_VIEW,
    THUMBNAIL_VIEW
};

enum OutlineProperty
{
    OUTLINE_ITEM = Qt::UserRole + 1
};

enum ThumbnailRenderDirection
{
    THUMBNAIL_RENDER_INVALID = -1,
    THUMBNAIL_RENDER_CURRENT_PAGE = 0,
    THUMBNAIL_RENDER_NEXT_PAGE,
    THUMBNAIL_RENDER_PREVIOUS_PAGE
};

struct ViewSetting
{
    RotateDegree rotate_orient;
    ZoomFactor   zoom_setting;
    ViewSetting(): rotate_orient(ROTATE_0_DEGREE), zoom_setting(ZOOM_HIDE_MARGIN) {}
};

// Pan area
class PanArea
{
public:
    PanArea() : p1(), p2() {}
    ~PanArea() {}
    inline void setStartPoint(const QPoint &pos);
    inline void setEndPoint(const QPoint &pos);
    inline void getOffset(int &offset_x, int &offset_y);
    inline const QPoint & getStart() const;
    inline const QPoint & getEnd() const;
private:
    /// pan start point
    QPoint p1;

    /// pan end point
    QPoint p2;
};

class StrokeArea
{
public:
    StrokeArea() : area_() {}
    ~StrokeArea() {}

    void initArea(const QPoint &point);
    void expandArea(const QPoint &point);
    const QRect & getRect();
    const QPoint & getOriginPosition() { return origin_pos_; }
private:
    QRect area_;           /// stroke area
    QPoint origin_pos_;    /// original position of the area
};

/// Set start point of pan area
inline void PanArea::setStartPoint(const QPoint &pos)
{
    p1 = pos;
}

/// Set end point of pan area
inline void PanArea::setEndPoint(const QPoint &pos)
{
    p2 = pos;
}

/// Get the offset of pan area
inline void PanArea::getOffset(int &offset_x, int &offset_y)
{
    offset_x = p1.x() - p2.x();
    offset_y = p1.y() - p2.y();
}

inline const QPoint & PanArea::getStart() const
{
    return p1;
}

inline const QPoint & PanArea::getEnd() const
{
    return p2;
}

/// Initialize the stroke area
inline void StrokeArea::initArea(const QPoint &point)
{
    origin_pos_ = point;
    area_.setTopLeft(point);
    area_.setBottomRight(point);
}

/// Expand the stroke area
inline void StrokeArea::expandArea(const QPoint &point)
{
    if (area_.left() > point.x())
    {
        area_.setLeft(point.x());
    }
    else if (area_.right() < point.x())
    {
        area_.setRight(point.x());
    }

    if (area_.top() > point.y())
    {
        area_.setTop(point.y());
    }
    else if (area_.bottom() < point.y())
    {
        area_.setBottom(point.y());
    }
}

/// Get the bounding rectangle of stroke area
inline const QRect & StrokeArea::getRect()
{
    area_ = area_.normalized();
    return area_;
}

};

#endif

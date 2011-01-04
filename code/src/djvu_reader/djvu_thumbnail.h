#ifndef DJVU_THUMBNAIL_H
#define DJVU_THUMBNAIL_H

#include "onyx/ui/base_thumbnail.h"
#include "djvu_utils.h"
#include "djvu_page.h"

using namespace onyx::ui;
using namespace ui;

namespace djvu_reader
{

class DjvuThumbnail : public BaseThumbnail
{
public:
    DjvuThumbnail();
    DjvuThumbnail(DjVuPagePtr page, ZoomFactor zoom_value);
    virtual ~DjvuThumbnail();

    const QRect& displayArea() const;
    QImage* image() const { return image_.get(); }
    const QString & path() const;
    const QString & name() const;
    int key() const;
    ZoomFactor zoom() { return zoom_value_; }

    void setImage(QImage* image) { image_.reset(new QImage(*image)); rect_ = image_->rect(); }

private:
    scoped_ptr<QImage> image_;       /// qt image
    ZoomFactor         zoom_value_;  /// zoom_value
    QString            name_;
    int                position_;
    QRect              rect_;
};

DjvuThumbnail::DjvuThumbnail()
{
}

DjvuThumbnail::DjvuThumbnail(DjVuPagePtr page, ZoomFactor zoom_value)
    : image_(0)
    , zoom_value_(zoom_value)
    , name_()
    , position_(page->pageNum())
    , rect_(QPoint(0, 0), cms::thumbnailSize(THUMBNAIL_LARGE))
{
    name_.setNum(position_ + 1);
    setImage(page->image());
}

DjvuThumbnail::~DjvuThumbnail()
{
}

const QString & DjvuThumbnail::path() const
{
    return name_;
}

const QString & DjvuThumbnail::name() const
{
    return name_;
}

int DjvuThumbnail::key() const
{
    return position_;
}

const QRect& DjvuThumbnail::displayArea() const
{
    // NOT implement
    return rect_;
}

};

#endif

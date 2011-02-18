#include "djvu_page.h"
#include "djvu_document.h"

namespace djvu_reader
{

QDjVuPage::ContentAreaMap QDjVuPage::content_areas_;
QDjVuPage::PageTextEntityMap QDjVuPage::page_texts_;
static QVector<QRgb> COLOR_TABLE;

static void initialColorTable()
{
    if (COLOR_TABLE.size() <= 0)
    {
        for(int i = 0; i < 256; ++i)
        {
            COLOR_TABLE.push_back(qRgba(i, i, i, 255));
        }
    }
}

// ----------------------------------------
// QDJVUPAGE

/*! \class QDjVuPage
    \brief Represent a \a ddjvu_page_t object. */

/*! Construct a \a QDjVuPage object for page \a page_no_
    of document \a doc. Argument \a parent indicates its 
    parent in the \a QObject hierarchy. */

QDjVuPage::QDjVuPage(QDjVuDocument *doc, int page_no, QObject *parent)
  : QObject(parent)
  , page_(0)
  , page_no_(page_no)
  , render_needed_(false)
  , content_area_needed_(false)
  , is_ready_(false)
  , is_thumbnail_(false)
  , thumbnail_direction_(THUMBNAIL_RENDER_INVALID)
{
    initialColorTable();
    page_ = ddjvu_page_create_by_pageno(*doc, page_no_);
    if (!page_)
    {
        qWarning("QDjVuPage: invalid page number");
        return;
    }
    ddjvu_page_set_user_data(page_, (void*)this);
    doc->add(this);

    if (ddjvu_page_decoding_done(page_))
    {
        is_ready_ = true;
        updateInfo();
    }
}

QDjVuPage::~QDjVuPage()
{
    page_no_ = -1;
    if (page_ != 0)
    {
        ddjvu_page_set_user_data(page_, 0);
        ddjvu_page_release(page_);
        page_ = 0;
    }
}

/*! Processes DDJVUAPI messages for this page. 
    The default implementation emits signals for
    the \a m_error, \a m_info, \a m_pageinfo, \a m_chunk
    and \a m_relayout and \a m_redisplay messsages. 
    The return value is a boolean indicating
    if the message has been processed or rejected. */

bool QDjVuPage::handle(ddjvu_message_t *msg)
{
    switch(msg->m_any.tag)
    {
    case DDJVU_PAGEINFO:
        {
            is_ready_ = true;
            updateInfo();
            emit pageInfo(this);
        }
        return true;
    case DDJVU_CHUNK:
        {
            emit chunk(this, QString::fromAscii(msg->m_chunk.chunkid));
        }
        return true;
    case DDJVU_RELAYOUT:
        {
            updateInfo();
            emit relayout(this);
        }
        return true;
    case DDJVU_REDISPLAY:
        {
            emit redisplay(this);
        }
        return true;
    case DDJVU_ERROR:
        {
            emit error(this,
                       QString::fromLocal8Bit(msg->m_error.message),
                       QString::fromLocal8Bit(msg->m_error.filename), 
                       msg->m_error.lineno);
        }
      return true;
    case DDJVU_INFO:
        {
            emit info(this, QString::fromLocal8Bit(msg->m_info.message));
        }
        return true;
    default:
      break;
    }
    return false;
}

bool QDjVuPage::implRender(const RenderSetting & setting, ddjvu_format_t * render_format)
{
    if (isReady() && isDecodeDone())
    {
        ddjvu_rect_t page_rect = {0, 0, setting.contentArea().width(), setting.contentArea().height()};
        ddjvu_rect_t render_rect = page_rect;

        QImage image(setting.contentArea().size(), QImage::Format_RGB888);
        image.setColorTable(COLOR_TABLE);
        int ret = ddjvu_page_render(page_,
                                    DDJVU_RENDER_COLOR,
                                    &page_rect,
                                    &render_rect,
                                    render_format,
                                    image.bytesPerLine(),
                                    (char*)image.bits());
        if (ret > 0)
        {
            image_ = image;
            render_needed_ = false;
            return true;
        }

        if (!image_.isNull())
        {
            image_ = QImage();
        }
    }
    render_needed_ = true;
    return false;
}

bool QDjVuPage::render(const RenderSetting & setting, ddjvu_format_t * render_format)
{
    if (render_setting_ != setting || image_.isNull())
    {
        // re-render the page
        render_setting_ = setting;
        return implRender(setting, render_format);
    }
    return true;
}

bool QDjVuPage::render(ddjvu_format_t * render_format)
{
    return implRender(render_setting_, render_format);
}

void QDjVuPage::lock()
{
}

void QDjVuPage::unlock()
{
}

void QDjVuPage::updateInfo()
{
    if (page_ != 0)
    {
        info_.page_size.setWidth(ddjvu_page_get_width(page_));
        info_.page_size.setHeight(ddjvu_page_get_height(page_));
        info_.resolution        = ddjvu_page_get_resolution(page_);
        info_.gamma             = ddjvu_page_get_gamma(page_);
        info_.version           = ddjvu_page_get_version(page_);
        info_.type              = ddjvu_page_get_type(page_);
    }
}

/*! Returns the page number associated with this page. */

int QDjVuPage::pageNum()
{
    return page_no_;
}

bool QDjVuPage::isDecodeDone()
{
    bool ret = ddjvu_page_decoding_done(page_);
    return ret;
}

QRect QDjVuPage::contentArea(int page_no)
{
    if (QDjVuPage::content_areas_.contains(page_no))
    {
        return QDjVuPage::content_areas_[page_no];
    }
    return QRect();
}

static bool getContentFromPage(const QImage & page,
                               const int width,
                               const int height,
                               QRect & content)
{
    static const int LINE_STEP        = 1;
    static const int SHRINK_STEP      = 1;
    static const double STOP_RANGE    = 0.3f;
    static const int BACKGROUND_COLOR = 0xffffffff;

    // top left
    int x1 = 0;
    int y1 = 0;
    // bottom right
    int x2 = width - 1;
    int y2 = height - 1;

    int left_edge = static_cast<int>(STOP_RANGE * x2);
    int right_edge = static_cast<int>((1.0f - STOP_RANGE) * x2);
    int top_edge = static_cast<int>(STOP_RANGE * y2);
    int bottom_edge = static_cast<int>((1.0f - STOP_RANGE) * y2);

    // current pixel
    QRgb cur_pix;
    bool stop[4] = {false, false, false, false};
    while (!stop[0] || !stop[1] || !stop[2] || !stop[3])
    {
        // check top line
        int x_cur = x1;
        while (x_cur < x2 && !stop[0])
        {
            cur_pix = page.pixel(x_cur, y1);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[0] = true;
                break;
            }
            x_cur += LINE_STEP;
        }

        // check bottom line
        x_cur = x1;
        while (x_cur < x2 && !stop[1])
        {
            cur_pix = page.pixel(x_cur, y2);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[1] = true;
                break;
            }
            x_cur += LINE_STEP;
        }

        // check left line
        int y_cur = y1;
        while (y_cur < y2 && !stop[2])
        {
            cur_pix = page.pixel(x1, y_cur);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[2] = true;
                break;
            }
            y_cur += LINE_STEP;
        }

        // check right line
        y_cur = y1;
        while (y_cur < y2 && !stop[3])
        {
            cur_pix = page.pixel(x2, y_cur);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[3] = true;
                break;
            }
            y_cur += LINE_STEP;
        }

        // shrink the rectangle
        if (!stop[2])
        {
            if (x1 >= left_edge)
            {
                stop[2] = true;
            }
            else
            {
                x1 += SHRINK_STEP;
            }
        }

        if (!stop[3])
        {
            if (x2 <= right_edge)
            {
                stop[3] = true;
            }
            else
            {
                x2 -= SHRINK_STEP;
            }
        }

        if (!stop[0])
        {
            if (y1 >= top_edge)
            {
                stop[0] = true;
            }
            else
            {
                y1 += SHRINK_STEP;
            }
        }

        if (!stop[1])
        {
            if (y2 <= bottom_edge)
            {
                stop[1] = true;
            }
            else
            {
                y2 -= SHRINK_STEP;
            }
        }

    }

    if (stop[0] && stop[1] && stop[2] && stop[3])
    {
        content.setTopLeft(QPoint(x1, y1));
        content.setBottomRight(QPoint(x2, y2));
        return true;
    }
    return false;
}

static void expandContentArea(const int page_width,
                              const int page_height,
                              QRect & content_area)
{
    static const int EXPAND_STEP = 1;
    // expand the content area to avoid content covering
    int inc_right = 0;
    int inc_bottom = 0;

    // expand left
    if (content_area.left() > EXPAND_STEP)
    {
        content_area.setLeft(content_area.left() - EXPAND_STEP);
        inc_right = EXPAND_STEP;
    }
    else
    {
        inc_right = content_area.left();
        content_area.setLeft(0);
    }

    // expand top
    if (content_area.top() > EXPAND_STEP)
    {
        content_area.setTop(content_area.top() - EXPAND_STEP);
        inc_bottom = EXPAND_STEP;
    }
    else
    {
        inc_bottom = content_area.top();
        content_area.setTop(0);
    }

    // expand right
    content_area.setRight(content_area.right() + inc_right + 1);
    if (content_area.right() > page_width)
    {
        content_area.setRight(page_width);
    }

    // expand bottom
    content_area.setBottom(content_area.bottom() + inc_bottom + 1);
    if (content_area.bottom() > page_height)
    {
        content_area.setBottom(page_height);
    }
}

QRect QDjVuPage::getContentArea(ddjvu_format_t * render_format)
{
    content_area_needed_ = false;
    QRect content_area = QDjVuPage::contentArea(page_no_);
    if (content_area.isValid())
    {
        return content_area;
    }

    if (!isReady() || !isDecodeDone())
    {
        content_area_needed_ = true;
        return content_area;
    }

    // intialize the content area
    content_area.setTopLeft(QPoint(0, 0));
    content_area.setSize(info_.page_size);

    static const ZoomFactor MAX_SAMPLE_SIZE = 200.0f;
    ZoomFactor zoom = std::min(MAX_SAMPLE_SIZE /
                               static_cast<ZoomFactor>(info_.page_size.width()),
                               MAX_SAMPLE_SIZE /
                               static_cast<ZoomFactor>(info_.page_size.height()));

    int width = static_cast<int>(zoom * static_cast<ZoomFactor>(info_.page_size.width()));
    int height = static_cast<int>(zoom * static_cast<ZoomFactor>(info_.page_size.height()));

    ddjvu_rect_t page_rect = {0, 0, width, height};
    ddjvu_rect_t render_rect = page_rect;

    QImage image(QSize(width, height), QImage::Format_RGB888);
    image.setColorTable(COLOR_TABLE);
    int ret = ddjvu_page_render(page_,
                                DDJVU_RENDER_COLOR,
                                &page_rect,
                                &render_rect,
                                render_format,
                                image.bytesPerLine(),
                                (char*)image.bits());
    if (ret > 0 &&
        getContentFromPage(image,
                           width,
                           height,
                           content_area))
    {
        expandContentArea(width, height, content_area);
        content_area.setTopLeft(QPoint(
            static_cast<ZoomFactor>(content_area.left()) / zoom,
            static_cast<ZoomFactor>(content_area.top()) / zoom));
        content_area.setBottomRight(QPoint(
            static_cast<ZoomFactor>(content_area.right()) / zoom,
            static_cast<ZoomFactor>(content_area.bottom()) / zoom));
        QDjVuPage::content_areas_[page_no_] = content_area;
    }
    return content_area;
}

PageTextEntities & QDjVuPage::pageTextEntities(int page_no, bool & existed)
{
    if (!QDjVuPage::page_texts_.contains(page_no))
    {
        PageTextEntities entities;
        QDjVuPage::page_texts_[page_no] = entities;
        existed = false;
    }
    else
    {
        existed = true;
    }
    return QDjVuPage::page_texts_[page_no];
}

DjvuPageInfo::DjvuPageInfo()
    : resolution(0)
    , gamma(0.0)
    , version(0)
    , type(DDJVU_PAGETYPE_UNKNOWN)
{
}

DjvuPageInfo::~DjvuPageInfo()
{
}

}

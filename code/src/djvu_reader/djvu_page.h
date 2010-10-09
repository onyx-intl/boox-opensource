#ifndef DJVU_PAGE_H_
#define DJVU_PAGE_H_

#include "djvu_utils.h"

using namespace ui;
namespace djvu_reader
{

/// The class RenderSetting contains all of the render settings of an image
class RenderSetting
{
public:
    RenderSetting()
        : content_area_()
        , rotation_(ROTATE_0_DEGREE) {}
    ~RenderSetting() {}

    RenderSetting(const RenderSetting &right)
    {
        *this = right;
    }

    RenderSetting& operator = (const RenderSetting &right)
    {
        if (*this == right)
        {
            return *this;
        }

        content_area_ = right.content_area_;
        clip_area_    = right.clip_area_;
        rotation_     = right.rotation_;
        clip_image_   = right.clip_image_;
        return *this;
    }

    bool operator == (const RenderSetting &right) const
    {
        return (clip_image_ == right.clip_image_
                && (clip_image_ ? clip_area_ == right.clip_area_ : 1)
                && content_area_ == right.content_area_
                && rotation_ == right.rotation_);
    }

    bool operator != (const RenderSetting & right) const
    {
        return !(*this == right);
    }

    // set the content area
    void setContentArea(const QRect &content_area) { content_area_.setSize(content_area.size()); }
    const QRect& contentArea() const {return content_area_;}

    // set rotation
    void setRotation(const RotateDegree r) { rotation_ = r; }
    RotateDegree rotation() const { return rotation_; }

    // clipping
    void setClipArea(const QRect &clip_area) { clip_area_ = clip_area; }
    const QRect& clipArea() const { return clip_area_; }
    void setClipImage(bool c) {clip_image_ = c;}
    bool isClipImage() const { return clip_image_; }

private:
    QRect        content_area_;     ///< the rectangle of the render area
    RotateDegree rotation_;         ///< the rotation degrees
    QRect        clip_area_;        ///< clip area
    bool         clip_image_;       ///< clip the image?
};

typedef shared_ptr<RenderSetting> RenderSettingPtr;

class DjvuPageInfo
{
public:
    DjvuPageInfo();
    ~DjvuPageInfo();

public:
    QSize               page_size;
    int                 resolution;
    double              gamma;
    int                 version;
    ddjvu_page_type_t   type;
    QString             short_description;
    QString             long_description;
};

class TextEntity
{
public:
    TextEntity() {}
    ~TextEntity() {}
public:
    QRect    area;
    QString  text;
};

typedef shared_ptr<TextEntity> TextEntityPtr;
typedef QList<TextEntityPtr>   PageTextEntities;

class QDjVuDocument;
class QDjVuPage : public QObject
{
  Q_OBJECT
public:
    virtual ~QDjVuPage();
    QDjVuPage(QDjVuDocument *doc, int page_no, QObject *parent = 0);
    operator ddjvu_page_t*() { return page_; }
    bool isValid() { return page_ != 0; }
    bool isReady() { return is_ready_; }
    bool renderNeeded() { return render_needed_; }
    bool contentAreaNeeded() { return content_area_needed_; }
    QImage * image() { return &image_; }

    bool isDecodeDone();
    int  pageNum();

    const RenderSetting & renderSetting() const { return render_setting_; }
    bool render(const RenderSetting & setting, ddjvu_format_t * render_format);
    bool render(ddjvu_format_t * render_format);
    QRect getContentArea(ddjvu_format_t * render_format);

    void lock();
    void unlock();

    static PageTextEntities & pageTextEntities(int page_no, bool & existed);

protected:
    virtual bool handle(ddjvu_message_t*);

Q_SIGNALS:
    void error(int page_no, QString msg, QString filename, int lineno);
    void info(int page_no, QString msg);
    void chunk(int page_no, QString chunkid);
    void pageInfo(int page_no);
    void relayout(int page_no);
    void redisplay(int page_no);

    void contentAreaReady(const QRect & content_area);

private:
    bool implRender(const RenderSetting & setting, ddjvu_format_t * render_format);
    void updateInfo();

    static QRect contentArea(int page_no);

private:
    typedef QMap<int, QRect>            ContentAreaMap;
    typedef ContentAreaMap::iterator    ContentAreaIter;

    typedef QMap<int, PageTextEntities> PageTextEntityMap;
    typedef PageTextEntityMap::iterator PageTextEntityIter;
private:
    ddjvu_page_t     *page_;
    int              page_no_;            // might become private pointer in the future
    bool             render_needed_;
    bool             content_area_needed_;
    bool             is_ready_;
    RenderSetting    render_setting_;
    DjvuPageInfo     info_;
    QImage           image_;
    static ContentAreaMap content_areas_;
    static PageTextEntityMap page_texts_;

private:
    friend class QDjVuContext;
    friend class QDjVuDocument;
};

typedef shared_ptr<QDjVuPage> DjVuPagePtr;

};
#endif // DJVU_PAGE_H_

#ifndef DJUV_RENDER_PROXY_H_
#define DJUV_RENDER_PROXY_H_

#include "djvu_utils.h"
#include "djvu_page.h"

namespace djvu_reader
{

typedef QMap<int, RenderSettingPtr> PageRenderSettings;

class QDjVuDocument;
class DjvuRenderProxy : public QObject
{
    Q_OBJECT
public:
    DjvuRenderProxy();
    ~DjvuRenderProxy();

    void render(PageRenderSettings & render_pages, QDjVuDocument * doc);
    void renderThumbnail(int page_num,
                         const RenderSetting & render_setting,
                         ThumbnailRenderDirection direction,
                         QDjVuDocument * doc);
    void requirePageContentArea(int page_no, QDjVuDocument * doc);
    bool getPageRenderSetting(int page_no, RenderSetting & render_setting);

Q_SIGNALS:
    void pageRenderReady(DjVuPagePtr page);
    void relayout(DjVuPagePtr page);
    void contentAreaReady(DjVuPagePtr page, const QRect & content_area);

private Q_SLOTS:
    void onPageError(QDjVuPage * from, QString msg, QString file_name, int line_no);
    void onInfo(QDjVuPage * from, QString msg);
    void onPageChunk(QDjVuPage * from, QString chunk_id);
    void onPageInfo(QDjVuPage * from);
    void onRelayout(QDjVuPage * from);
    void onRedisplay(QDjVuPage * from);

private:
    DjVuPagePtr getPage(QDjVuDocument * doc, int page_no);

private:
    typedef QMap<int, DjVuPagePtr> DjvuPages;
    typedef DjvuPages::iterator    DjvuPageIter;

private:
    DjvuPages      pages_;
    ddjvu_format_t *render_format_;

};

};

#endif

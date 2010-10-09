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
    void requirePageContentArea(int page_no, QDjVuDocument * doc);
    bool getPageRenderSetting(int page_no, RenderSetting & render_setting);

Q_SIGNALS:
    void pageRenderReady(DjVuPagePtr page);
    void relayout(DjVuPagePtr page);
    void contentAreaReady(const int page_number, const QRect & content_area);

private Q_SLOTS:
    void onPageError(int page_no, QString msg, QString file_name, int line_no);
    void onInfo(int page_no, QString msg);
    void onPageChunk(int page_no, QString chunk_id);
    void onPageInfo(int page_no);
    void onRelayout(int page_no);
    void onRedisplay(int page_no);

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

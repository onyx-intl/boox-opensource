#include "djvu_render_proxy.h"
#include "djvu_model.h"

namespace djvu_reader
{

DjvuRenderProxy::DjvuRenderProxy()
    : render_format_(0)
{
    render_format_ = ddjvu_format_create(DDJVU_FORMAT_GREY8, 0, 0);
    ddjvu_format_set_row_order(render_format_, true);
    ddjvu_format_set_y_direction(render_format_, true);
    ddjvu_format_set_ditherbits(render_format_, 8);
    ddjvu_format_set_gamma(render_format_, 2.2);
}

DjvuRenderProxy::~DjvuRenderProxy()
{
    if (render_format_ != 0)
    {
        ddjvu_format_release(render_format_);
    }
}

void DjvuRenderProxy::render(PageRenderSettings & render_pages, QDjVuDocument * doc)
{
    // remove redundant pages
    DjvuPageIter page_idx = pages_.begin();
    while (page_idx != pages_.end())
    {
        if (!render_pages.contains(page_idx.key()))
        {
            page_idx = pages_.erase(page_idx);
        }
        else
        {
            page_idx++;
        }
    }

    // render pages
    PageRenderSettings::iterator render_idx = render_pages.begin();
    while (render_idx != render_pages.end())
    {
        DjVuPagePtr page = getPage(doc, render_idx.key());
        RenderSettingPtr render_setting = render_idx.value();
        if (page->render(*render_setting, render_format_))
        {
            emit pageRenderReady(page);
        }
        render_idx++;
    }
}

void DjvuRenderProxy::onPageError(int page_no, QString msg, QString file_name, int line_no)
{
}

void DjvuRenderProxy::onInfo(int page_no, QString msg)
{
}

void DjvuRenderProxy::onPageChunk(int page_no, QString chunk_id)
{
}

void DjvuRenderProxy::onPageInfo(int page_no)
{
    if (pages_.contains(page_no))
    {
        DjVuPagePtr page = pages_[page_no];

        // continue retrieving the content area
        if (page->contentAreaNeeded())
        {
            const QRect & content_area = page->getContentArea(render_format_);
            if (content_area.isValid())
            {
                emit contentAreaReady(page_no, content_area);
            }
        }

        // continue rendering the page
        if (page->renderNeeded() && page->render(render_format_))
        {
            emit pageRenderReady(page);
        }
    }
}

void DjvuRenderProxy::onRelayout(int page_no)
{
    if (pages_.contains(page_no))
    {
        DjVuPagePtr page = pages_[page_no];
        emit relayout(page);
    }
}

void DjvuRenderProxy::onRedisplay(int page_no)
{
    if (pages_.contains(page_no))
    {
        DjVuPagePtr page = pages_[page_no];

        // continue retrieving the content area
        if (page->contentAreaNeeded())
        {
            const QRect & content_area = page->getContentArea(render_format_);
            if (content_area.isValid())
            {
                emit contentAreaReady(page_no, content_area);
            }
        }

        // continue rendering the page
        if (page->renderNeeded() && page->render(render_format_))
        {
            emit pageRenderReady(page);
        }
    }
}

DjVuPagePtr DjvuRenderProxy::getPage(QDjVuDocument * doc, int page_no)
{
    if (!pages_.contains(page_no))
    {
        DjVuPagePtr new_page(new QDjVuPage(doc, page_no));

        connect(new_page.get(), SIGNAL(relayout(int)), this, SLOT(onRelayout(int)));
        connect(new_page.get(), SIGNAL(redisplay(int)), this, SLOT(onRedisplay(int)));
        connect(new_page.get(), SIGNAL(error(int, QString, QString, int)),
                this, SLOT(onPageError(int, QString, QString, int)));
        connect(new_page.get(), SIGNAL(info(int, QString)), this, SLOT(onInfo(int, QString)));
        connect(new_page.get(), SIGNAL(chunk(int, QString)), this, SLOT(onPageChunk(int, QString)));
        connect(new_page.get(), SIGNAL(pageInfo(int)), this, SLOT(onPageInfo(int)));

        pages_[page_no] = new_page;
    }
    return pages_[page_no];
}

bool DjvuRenderProxy::getPageRenderSetting(int page_no, RenderSetting & render_setting)
{
    if (pages_.contains(page_no))
    {
        DjVuPagePtr page = pages_[page_no];
        render_setting = page->renderSetting();
        return true;
    }
    return false;
}

void DjvuRenderProxy::requirePageContentArea(int page_no, QDjVuDocument * doc)
{
    DjVuPagePtr page = getPage(doc, page_no);
    const QRect & content_area = page->getContentArea(render_format_);
    if (content_area.isValid())
    {
        emit contentAreaReady(page_no, content_area);
    }
}

}

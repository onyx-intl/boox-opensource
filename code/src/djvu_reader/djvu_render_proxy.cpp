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
        page->setToBeThumbnail(false);
        page->setThumbnailDirection(THUMBNAIL_RENDER_INVALID);
        RenderSettingPtr render_setting = render_idx.value();
        if (page->render(*render_setting, render_format_))
        {
            emit pageRenderReady(page);
        }
        render_idx++;
    }
}

void DjvuRenderProxy::renderThumbnail(int page_num,
                                      const RenderSetting & render_setting,
                                      ThumbnailRenderDirection direction,
                                      QDjVuDocument * doc)
{
    // create a new thumbnail page
    DjVuPagePtr thumbnail = getPage(doc, page_num);
    thumbnail->setToBeThumbnail(true);
    thumbnail->setThumbnailDirection(direction);
    if (thumbnail->render(render_setting, render_format_))
    {
        emit pageRenderReady(thumbnail);
    }
}

void DjvuRenderProxy::onPageError(QDjVuPage * from, QString msg, QString file_name, int line_no)
{
}

void DjvuRenderProxy::onInfo(QDjVuPage * from, QString msg)
{
}

void DjvuRenderProxy::onPageChunk(QDjVuPage * from, QString chunk_id)
{
}

void DjvuRenderProxy::onPageInfo(QDjVuPage * from)
{
    DjVuPagePtr page;
    if (pages_.contains(from->pageNum()))
    {
        page = pages_[from->pageNum()];
    }
    else
    {
        page = DjVuPagePtr(from);
    }

    // continue retrieving the content area
    if (page->contentAreaNeeded())
    {
        const QRect & content_area = page->getContentArea(render_format_);
        if (content_area.isValid())
        {
            emit contentAreaReady(page, content_area);
        }
    }

    // continue rendering the page
    if (page->renderNeeded() && page->render(render_format_))
    {
        emit pageRenderReady(page);
    }
}

void DjvuRenderProxy::onRelayout(QDjVuPage * from)
{
    DjVuPagePtr page;
    if (pages_.contains(from->pageNum()))
    {
        page = pages_[from->pageNum()];
    }
    else
    {
        page = DjVuPagePtr(from);
    }
    emit relayout(page);
}

void DjvuRenderProxy::onRedisplay(QDjVuPage * from)
{
    DjVuPagePtr page;
    if (pages_.contains(from->pageNum()))
    {
        page = pages_[from->pageNum()];
    }
    else
    {
        page = DjVuPagePtr(from);
    }

    // continue retrieving the content area
    if (page->contentAreaNeeded())
    {
        const QRect & content_area = page->getContentArea(render_format_);
        if (content_area.isValid())
        {
            emit contentAreaReady(page, content_area);
        }
    }

    // continue rendering the page
    if (page->renderNeeded() && page->render(render_format_))
    {
        emit pageRenderReady(page);
    }
}

DjVuPagePtr DjvuRenderProxy::getPage(QDjVuDocument * doc, int page_no)
{
    if (!pages_.contains(page_no))
    {
        DjVuPagePtr new_page(new QDjVuPage(doc, page_no));

        connect(new_page.get(), SIGNAL(relayout(QDjVuPage *)), this, SLOT(onRelayout(QDjVuPage *)));
        connect(new_page.get(), SIGNAL(redisplay(QDjVuPage *)), this, SLOT(onRedisplay(QDjVuPage *)));
        connect(new_page.get(), SIGNAL(error(QDjVuPage *, QString, QString, int)),
                this, SLOT(onPageError(QDjVuPage *, QString, QString, int)));
        connect(new_page.get(), SIGNAL(info(QDjVuPage *, QString)), this, SLOT(onInfo(QDjVuPage *, QString)));
        connect(new_page.get(), SIGNAL(chunk(QDjVuPage *, QString)), this, SLOT(onPageChunk(QDjVuPage *, QString)));
        connect(new_page.get(), SIGNAL(pageInfo(QDjVuPage *)), this, SLOT(onPageInfo(QDjVuPage *)));

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
        emit contentAreaReady(page, content_area);
    }
}

}

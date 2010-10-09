#ifndef DJVU_MODEL_H_
#define DJVU_MODEL_H_

#include "djvu_utils.h"
#include "djvu_document.h"

using namespace ui;
using namespace vbf;

namespace djvu_reader
{

class DjvuModel : public BaseModel
{
    Q_OBJECT
public:
    DjvuModel(const QString & program_name);
    virtual ~DjvuModel();

    inline QDjVuDocument* document() { return &doc_; }
    bool save();

    // Load & Save configurations
    bool loadOptions();
    bool saveOptions();

    // Document loading and closing
    bool open(const QString & path);
    bool close();
    bool isReady() const { return is_ready_; }
    bool isTheDocument(const QString &path);

    // Retrieve the metadata.
    bool metadata(const MetadataTag tag, QVariant &value);
    QImage getThumbnail(const int width, const int height);
    Configuration & getConf() { return conf_; }

    // Basic information
    int firstPageNumber();
    int getPagesTotalNumber();
    shared_ptr<ddjvu_pageinfo_t> getPageInfo(int page_no);
    const QString & path() const { return path_; }

    // Bookmarks
    QString getPageText(int page_no);
    bool saveBookmarks();
    bool loadBookmarks();
    bool addBookmark(const int page_start, const int page_end);
    bool deleteBookmark(const int page_start, const int page_end);
    bool hasBookmark(const int page_start, const int page_end);
    bool updateBookmark(const int page_start, const int page_end, const QString & name);
    QString getFirstBookmarkTitle(const int page_start, const int page_end);
    void getBookmarksModel(QStandardItemModel & bookmarks_model);

    // TOC
    bool hasOutlines();
    QStandardItemModel* getOutlineModel();
    QString getDestByTOCIndex(const QModelIndex & index);

Q_SIGNALS:
    void docError(QString msg, QString file_name, int line_no);
    void docInfo(QString msg);
    void docReady();
    void docPageReady();
    void docThumbnailReady(int page_num);
    void docIdle();

    void requestSaveAllOptions();

private:
    bool openCMS();
    void loadOutlineItem(QStandardItem * parent, miniexp_t exp, int offset);

private:
    // configuration
    cms::ContentManager content_manager_;
    Configuration       conf_;
    Bookmarks           bookmarks_;
    bool                need_save_bookmarks_;

    // document info
    bool                is_ready_;
    QString             path_;
    QDjVuDocument       doc_;

    scoped_ptr<QStandardItemModel> outline_model_;
};

};

#endif // DJVU_MODEL_H_

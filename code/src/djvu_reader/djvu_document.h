#ifndef DJVU_DOCUMENT_H_
#define DJVU_DOCUMENT_H_

#include "djvu_page.h"
#include "djvu_context.h"

namespace djvu_reader
{

class QDjVuDocumentPrivate : public QObject
{
    Q_OBJECT
public:
    QDjVuDocumentPrivate();

    void add(QObject *p);
    void add(QDjVuPage *p);

public Q_SLOTS:
    void onDocInfo();

protected Q_SLOTS:
    void onRemove(QObject *p);
    void onPageInfo(QDjVuPage *page);
    void onEmitIdle();

Q_SIGNALS:
    void idle();

private:
    QMutex              mutex_;
    bool                auto_delete_;
    int                 ref_count_;
    int                 page_num_;
    QSet<QObject*>      running_;
    bool                doc_ready_;
    QDjVuDocument       *doc_pointer_;
    minivar_t           document_outline_;
    minivar_t           document_annotations_;
    QVector<minivar_t>  page_annotations_;
    QVector<minivar_t>  page_text_;

private:
    friend class QDjVuDocument;
};

class QDjVuContext;
class QDjVuDocument : public QObject
{
    Q_OBJECT
public:
    QDjVuDocument(const QString & program_name, QObject *parent);
    QDjVuDocument(const QString & program_name, bool auto_delete = false, QObject *parent = 0);
    virtual ~QDjVuDocument();

    void add(QObject *p);
    void add(QDjVuPage *p);

    void ref();
    void deref();
    bool setFileName(QString file_name, bool cache = true);

    void streamWrite(int stream_id, const char *data, unsigned long len );
    void streamClose(int stream_id, bool stop = false);
    operator ddjvu_document_t*() { return document_; }
    virtual bool isValid() { return document_ != 0; }

    int runningProcesses(void);
    int getPageCount(void);
    miniexp_t getDocumentAnnotations();
    miniexp_t getDocumentOutline();
    miniexp_t getPageAnnotations(int page_no, bool start=true);
    miniexp_t getPageText(int page_no, bool start=true);

protected:
  virtual bool handle(ddjvu_message_t*);

protected:
    bool setUrl(QDjVuContext *ctx, QUrl url);
    bool setUrl(QDjVuContext *ctx, QUrl url, bool cache);
    virtual void newStream(int stream_id, QString name, QUrl url);

Q_SIGNALS:
    void error(QString msg, QString file_name, int line_no);
    void info(QString msg);
    void docInfo(void);
    void pageInfo(void);
    void thumbnail(int page_num);
    void idle(void);

private:
    QDjVuContext         ctx_;
    ddjvu_document_t     *document_;
    QDjVuDocumentPrivate *priv_;

private:
    friend class QDjVuContext;
    friend class QDjVuPage;
    friend class QDjVuJob;
};

};
#endif // DJVU_DOCUMENT_H_

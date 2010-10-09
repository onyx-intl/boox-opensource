#include "djvu_document.h"
#include "djvu_context.h"

namespace djvu_reader
{

QDjVuDocumentPrivate::QDjVuDocumentPrivate()
  : mutex_(QMutex::Recursive)
  , auto_delete_(false)
  , ref_count_(0)
  , page_num_(0)
  , doc_ready_(false)
  , doc_pointer_(0)
{
}

void QDjVuDocumentPrivate::add(QObject *p)
{
    connect(p, SIGNAL(destroyed(QObject*)), this, SLOT(onRemove(QObject*)));
    mutex_.lock();
    running_.insert(p);
    mutex_.unlock();
}

void QDjVuDocumentPrivate::onEmitIdle()
{
    emit idle();
}

void QDjVuDocumentPrivate::onRemove(QObject *p)
{
    mutex_.lock();
    running_.remove(p);
    int size = running_.size();
    disconnect(p, 0, this, 0);
    mutex_.unlock();

    if ( size == 0 )
    {
        QTimer::singleShot(0, this, SLOT(onEmitIdle()));
    }
}

void QDjVuDocumentPrivate::add(QDjVuPage *p)
{
    if (ddjvu_page_decoding_done(*p)) return;
    connect(p, SIGNAL(pageInfo(int)), this, SLOT(onPageInfo(int)));
    add((QObject*)(p));
}

void QDjVuDocumentPrivate::onPageInfo(int)
{
    QDjVuPage *p = qobject_cast<QDjVuPage*>(sender());
    if (p && ddjvu_page_decoding_done(*p))
    {
        onRemove(p);
    }
}


void QDjVuDocumentPrivate::onDocInfo()
{
    if (!doc_ready_ && ddjvu_document_decoding_done(*doc_pointer_))
    {
        QMutexLocker locker(&mutex_);
        page_num_ = ddjvu_document_get_pagenum(*doc_pointer_);
        document_outline_ = miniexp_dummy;
        document_annotations_ = miniexp_dummy;

        page_annotations_.resize(page_num_);
        page_text_.resize(page_num_);
        for (int i = 0; i < page_num_; i++)
        {
            page_annotations_[i] = page_text_[i] = miniexp_dummy;
        }
        doc_ready_ = true;
    }
}

// ----------------------------------------
// QDJVUDOCUMENT


/*! \class QDjVuDocument
    \brief Represents a \a ddjvu_document_t object. */

/*! Construct an empty \a QDjVuDocument object.
  Argument \a parent indicates its parent in the \a QObject hierarchy. 
  When argument \a auto_delete_ is set to true, the object also 
  maintains a reference count that can be modified using functions
  \a ref() and \a deref(). The object is deleted when function \a deref()
  decrements the reference count to zero. */

QDjVuDocument::QDjVuDocument(const QString & program_name, bool auto_delete, QObject *parent)
    : QObject(parent)
    , ctx_(program_name.toLocal8Bit().constData())
    , document_(0)
    , priv_(new QDjVuDocumentPrivate)
{
    priv_->auto_delete_ = auto_delete;
    priv_->doc_pointer_ = this;
    connect(priv_, SIGNAL(idle()), this, SIGNAL(idle()));
    connect(this, SIGNAL(docInfo()), priv_, SLOT(onDocInfo()));
}

/*! \overload */

QDjVuDocument::QDjVuDocument(const QString & program_name, QObject *parent)
    : QObject(parent)
    , ctx_(program_name.toLocal8Bit().constData())
    , document_(0)
    , priv_(new QDjVuDocumentPrivate)
{
    priv_->auto_delete_ = false;
    priv_->doc_pointer_ = this;
    connect(priv_, SIGNAL(idle()), this, SIGNAL(idle()));
    connect(this, SIGNAL(docInfo()), priv_, SLOT(onDocInfo()));
}

QDjVuDocument::~QDjVuDocument()
{
    if (document_)
    {
        ddjvu_document_set_user_data(document_, 0);
        ddjvu_document_release(document_);
        document_ = 0;
    }

    if (priv_)
    {
        delete priv_;
        priv_ = 0;
    }
}

void QDjVuDocument::add(QObject *p)
{
    priv_->add(p);
}

void QDjVuDocument::add(QDjVuPage *p)
{
    priv_->add(p);
}

/*! Increments the reference count. */

void QDjVuDocument::ref()
{
    priv_->mutex_.lock();
    priv_->ref_count_++;
    priv_->mutex_.unlock();
}

/*! Decrements the reference count. 
  If the object was created with flag \a auto_delete_,
  function \a deleteLater() is called when the 
  reference count reaches 0. */

void QDjVuDocument::deref()
{
    priv_->mutex_.lock();
    bool finished = !(--priv_->ref_count_) && priv_->auto_delete_;
    priv_->mutex_.unlock();
    if (finished)
    {
        delete this;
    }
}


/*! Associates the \a QDjVuDocument object with the 
    \a QDjVuContext object \ctx in order to decode
    the DjVu file \a f. */

bool QDjVuDocument::setFileName(QString f, bool cache)
{
    QMutexLocker locker(&priv_->mutex_);
    if (isValid())
    {
        ddjvu_document_set_user_data(document_, 0);
        ddjvu_document_release(document_);
        priv_->running_.clear();
        document_ = 0;
    }

    QFileInfo info(f);
    QByteArray b = QFile::encodeName(f);
    if (!info.isReadable())
    {
        qWarning("QDjVuDocument::setFileName: cannot read file");
        return false;
    }
    if (!(document_ = ddjvu_document_create_by_filename(*(&ctx_), b, cache)))
    {
        qWarning("QDjVuDocument::setFileName: cannot create decoder");    
        return false;
    }
    ddjvu_document_set_user_data(document_, (void*)this);
    priv_->onDocInfo();
    return true;
}

/*! Associates the \a QDjVuDocument object with
    with the \a QDjVuContext object \ctx in order
    to decode the DjVu data located at URL \a url.
    This is only useful inside a subclass of this class
    because you must redefine virtual function \a newstream
    in order to provide access to the data. */

bool QDjVuDocument::setUrl(QDjVuContext *ctx, QUrl url, bool cache)
{
    QMutexLocker locker(&priv_->mutex_);
    if (isValid())
    {
        ddjvu_document_set_user_data(document_, 0);
        ddjvu_document_release(document_);
        priv_->running_.clear();
        document_ = 0;
    }
    QByteArray b = url.toEncoded();
    if (! b.size())
    {
      qWarning("QDjVuDocument::setUrl: invalid url");
      return false;
    }
  
    document_ = ddjvu_document_create(*ctx, b, cache);
    if (! document_)
    {
      qWarning("QDjVuDocument::setUrl: cannot create");
      return false;
    }
    ddjvu_document_set_user_data(document_, (void*)this);
    priv_->onDocInfo();
    return true;
}


static bool string_is_on(QString val)
{
    QString v = val.toLower();
    return v == "yes" || v == "on" || v == "true" || v == "1";
}

static bool string_is_off(QString val)
{
    QString v = val.toLower();
    return v == "no" || v == "off" || v == "false" || v == "0";
}


/*! Overloaded version of \a setUrl.
    Cache setup is determined heuristically from the url
    and the url arguments. */

bool QDjVuDocument::setUrl(QDjVuContext *ctx, QUrl url)
{
    bool cache = true;
    if (url.path().section('/', -1).indexOf('.') < 0)
    {
        cache = false;
    }

    bool djvuopts = false;
    QPair<QString,QString> pair;
    foreach(pair, url.queryItems())
    {
        if (pair.first.toLower() == "djvuopts")
        {
            djvuopts = true;
        }
        else if (!djvuopts || pair.first.toLower() != "cache")
        {
            continue;
        }
        else if (string_is_on(pair.second))
        {
            cache = true;
        }
        else if (string_is_off(pair.second))
        {
            cache = false;
        }
    }
    return setUrl(ctx, url, cache);
}


/*! This virtual function is called when receiving
    a DDJVUAPI \a m_newstream message. This happens
    when the decoder has been setup with functon \a setUrl.
    You must then override this virtual function in order
    to setup the data transfer.  Data is passed to the decoder
    using the \a streamWrite and \a streamClose member functions
    with the specified \a streamid. See also the DDJVUAPI 
    documentation for the \a m_newstream message. */

void QDjVuDocument::newStream(int, QString, QUrl)
{
    qWarning("QDjVuDocument::newstream called but not implemented");
}

/*! Write data into the decoder stream \a streamid. */

void  QDjVuDocument::streamWrite(int stream_id, const char *data, unsigned long len )
{
    QMutexLocker locker(&priv_->mutex_);
    if (!isValid())
    {
        qWarning("QDjVuDocument::streamWrite: invalid document_");
    }
    else
    {
        ddjvu_stream_write(document_, stream_id, data, len);
    }
}

/*! Close the decoder stream \a streamid.
    Setting argument \a stop to \a true indicates
    that the stream was closed because the data
    transfer was interrupted by the user. */

void QDjVuDocument::streamClose(int stream_id, bool stop)
{
    QMutexLocker locker(&priv_->mutex_);
    if (!isValid())
    {
        qWarning("QDjVuDocument::streamClose: invalid document_");
    }
    else
    {
        ddjvu_stream_close(document_, stream_id, stop);
    }
}

/*! Processes DDJVUAPI messages for this document_. 
    The default implementation emits signals for
    the \a m_error, \a m_info, \a m_docinfo, \a m_pageingo
    and \a m_thumbnail messsages. It also calls
    the virtual function \a newstream when processing
    a \m newstream message. The return value is a boolean indicating
    if the message has been processed or rejected. */

bool QDjVuDocument::handle(ddjvu_message_t *msg)
{
    switch(msg->m_any.tag)
    {
    case DDJVU_DOCINFO:
        ddjvu_document_check_pagedata(document_, 0);
        emit docInfo();
        return true;
    case DDJVU_PAGEINFO:
        emit pageInfo();
        return true;
    case DDJVU_THUMBNAIL:
        emit thumbnail(msg->m_thumbnail.pagenum);
        return true;
    case DDJVU_NEWSTREAM:
        {
            QUrl url;
            QString name;
            if (msg->m_newstream.url)
            {
              url = QUrl::fromEncoded(msg->m_newstream.url);
            }
            if (msg->m_newstream.name)
            {
              name = QString::fromLatin1(msg->m_newstream.name);
            }
            newStream(msg->m_newstream.streamid, name, url);
        }
        return true;
    case DDJVU_ERROR:
        emit error(QString::fromLocal8Bit(msg->m_error.message),
                   QString::fromLocal8Bit(msg->m_error.filename), 
                   msg->m_error.lineno);
        return true;
    case DDJVU_INFO:
        emit info(QString::fromLocal8Bit(msg->m_info.message));
        return true;
    default:
        break;
    }
    return false;
}

/*! Return number of decoding threads running_ for this document_. */

int QDjVuDocument::runningProcesses(void)
{
    return priv_->running_.size();
}

/*! Return number of pages. */
int QDjVuDocument::getPageCount()
{
    return priv_->page_num_;
}

/*! Obtains the cached document_ annotations.
  This function returns \a miniexp_dummy if this 
  information is not yet available.
  Check again some time after 
  receiving signal \a docInfo(). */

miniexp_t QDjVuDocument::getDocumentAnnotations()
{
    QMutexLocker locker(&priv_->mutex_);
    if (! priv_->doc_ready_)
    {
        return miniexp_dummy;
    }

    if (priv_->document_annotations_ != miniexp_dummy)
    {
        return priv_->document_annotations_;
    }

#if DDJVUAPI_VERSION <= 17
    priv_->document_annotations_ = miniexp_nil;
#else
    priv_->document_annotations_ = ddjvu_document_get_anno(document_,1);
    ddjvu_miniexp_release(document_, priv_->document_annotations_);
#endif

    return priv_->document_annotations_;
}

/*! Obtains the cached document_ outline.
  This function returns \a miniexp_dummy if this 
  information is not yet available.
  Check again some time after 
  receiving signal \a docInfo(). */

miniexp_t QDjVuDocument::getDocumentOutline()
{
    QMutexLocker locker(&priv_->mutex_);
    if (! priv_->doc_ready_)
    {
        return miniexp_dummy;
    }

    if (priv_->document_outline_ != miniexp_dummy)
    {
        return priv_->document_outline_;
    }

    priv_->document_outline_ = ddjvu_document_get_outline(document_);
    ddjvu_miniexp_release(document_, priv_->document_outline_);
    return priv_->document_outline_;
}

/*! Obtains the cached annotations for page \a page_no.
  If this information is not yet available, 
  this function returns \a miniexp_dummy 
  and, if \a start is true, starts loading the page data.
  Check again after receiving signal \a pageInfo(). */

miniexp_t QDjVuDocument::getPageAnnotations(int page_no, bool start)
{
    QMutexLocker locker(&priv_->mutex_);
    if (!priv_->doc_ready_)
    {
        return miniexp_dummy;
    }

    if (page_no<0 || page_no>=priv_->page_annotations_.size())
    {
        return miniexp_dummy;
    }

    minivar_t expr = priv_->page_annotations_[page_no];
    if (expr != miniexp_dummy)
    {
        return expr;
    }

    if (!(start || ddjvu_document_check_pagedata(*this, page_no)))
    {
        return expr;
    }

    expr = ddjvu_document_get_pageanno(document_, page_no);
    ddjvu_miniexp_release(document_, expr);
    if (expr != miniexp_dummy)
    {
        priv_->page_annotations_[page_no] = expr;
    }
    return expr;
}

/*! Obtains the cached hidden text for page \a page_no.
  If this information is not yet available, 
  this function returns \a miniexp_dummy 
  and, if \a start is true, starts loading the page data.
  Check again after receiving signal \a pageInfo(). */

miniexp_t QDjVuDocument::getPageText(int page_no, bool start)
{
    QMutexLocker locker(&priv_->mutex_);
    if (!priv_->doc_ready_)
    {
        return miniexp_dummy;
    }

    if (page_no < 0 || page_no >= priv_->page_text_.size())
    {
        return miniexp_dummy;
    }

    minivar_t expr = priv_->page_text_[page_no];
    if (expr != miniexp_dummy)
    {
        return expr;
    }

    if (!(start || ddjvu_document_check_pagedata(*this, page_no)))
    {
        return expr;
    }

    expr = ddjvu_document_get_pagetext(document_, page_no, 0);
    ddjvu_miniexp_release(document_, expr);
    if (expr != miniexp_dummy)
    {
        priv_->page_text_[page_no] = expr;
    }
    return expr;
}

}

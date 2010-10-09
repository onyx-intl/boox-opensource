#include "djvu_context.h"
#include "djvu_page.h"
#include "djvu_job.h"
#include "djvu_document.h"

namespace djvu_reader
{

// ----------------------------------------
// QDJVUCONTEXT


/*! \class QDjVuContext
    \brief Represents a \a ddjvu_context_t object.
    
    This QObject subclass holds a \a ddjvu_context_t object and 
    transparently hooks the DDJVUAPI message queue into 
    the Qt messaging system. DDJVUAPI messages are then
    forwarded to the virtual function \a QDjVuContext::handle
    which transforms then into signals emitted by 
    the proper object from the message loop thread. 
*/

/*! Construct a \a QDJVuContext object.
    Argument \a programname is the name of the program.
    This name is used to report error messages and locate
    localized messages. Argument \a parent defines its
    parent in the \a QObject hierarchy. */

QDjVuContext::QDjVuContext(const char *programname, QObject *parent)
  : QObject(parent), context(0), flag(false)
{
  context = ddjvu_context_create(programname);
  ddjvu_message_set_callback(context, callback, (void*)this);
  ddjvu_cache_set_size(context, 30*1024*1024);
}

QDjVuContext::~QDjVuContext()
{
  ddjvu_context_release(context);
  context = 0;
}

/*! \property QDjVuContext::cacheSize
    \brief The size of the decoded page cache in bytes. 
    The default cache size is 30 megabytes. */

long 
QDjVuContext::cacheSize() const
{
  return ddjvu_cache_get_size(context);
}

void 
QDjVuContext::setCacheSize(long size)
{
  ddjvu_cache_set_size(context, size);
}

void 
QDjVuContext::callback(ddjvu_context_t *, void *closure)
{
  QDjVuContext *qcontext = (QDjVuContext*)closure;
  if (! qcontext->flag)
    {
      qcontext->flag = true;
      QEvent *qevent = new QEvent(QEvent::User);
	  QCoreApplication::postEvent(qcontext, qevent);
    }
}


bool
QDjVuContext::event(QEvent *event)
{
  if (event->type() == QEvent::User)
    {
      flag = false;
      ddjvu_message_t *message;
      while ((message = ddjvu_message_peek(context)))
        {
          handle(message);
          ddjvu_message_pop(context);
        }
      return true;
    }
  return QObject::event(event);
}


/*! Processes DDJVUAPI messages.
    The Qt message loop automatically passes all DDJVUAPI messages 
    to this virtual function . This function then forwards them
    to the \a handle function of the suitable \a QDjVuDocument,
    \a QDjVuPage or \a QDjVuJob object. Most messages eventually
    result into the emission of a suitable signal. */

bool
QDjVuContext::handle(ddjvu_message_t *msg)
{
  if (msg->m_any.page)
    {
      QObject *p = (QObject*)ddjvu_page_get_user_data(msg->m_any.page);
      QDjVuPage *q = (p) ? qobject_cast<QDjVuPage*>(p) : 0;
      if (q && q->handle(msg))
        return true;
    }
  if (msg->m_any.job)
    {
      ddjvu_job_t *djob = 0;
      ddjvu_job_t *pjob = 0;
      if (msg->m_any.document)
        djob = ddjvu_document_job(msg->m_any.document);
      if (msg->m_any.page)
        pjob = ddjvu_page_job(msg->m_any.page);
      if (msg->m_any.job != djob && msg->m_any.job != pjob)
        {
          QObject *p = (QObject*)ddjvu_job_get_user_data(msg->m_any.job);
          QDjVuJob *q = (p) ? qobject_cast<QDjVuJob*>(p) : 0;
          if (q && q->handle(msg))
            return true;
        }
    }
  if (msg->m_any.document)
    {
      QObject *p = (QObject*)ddjvu_document_get_user_data(msg->m_any.document);
      QDjVuDocument *q = (p) ? qobject_cast<QDjVuDocument*>(p) : 0;
      if (q && q->handle(msg))
        return true;
    }
  switch(msg->m_any.tag)
    {
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

}

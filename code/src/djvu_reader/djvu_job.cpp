#include "djvu_job.h"

namespace djvu_reader
{

// ----------------------------------------
// QDJVUJOB

/*! \class QDjVuJob
    \brief Represents a \a ddjvu_job_t object. */

/*! Construct a \a QDjVuJob object.
    Argument \a job is the \a ddjvu_job_t object.
    Argument \a parent defines its parent in the \a QObject hierarchy. */

QDjVuJob::QDjVuJob(ddjvu_job_t *job, QObject *parent)
    : QObject(parent)
    , job(job)
{
    if (!job)
    {
        qWarning("QDjVuJob: invalid job");
    }
    else
    {
        ddjvu_job_set_user_data(job, (void*)this);
    }
}

QDjVuJob::~QDjVuJob()
{
    ddjvu_job_set_user_data(job, 0);
    ddjvu_job_release(job);
    job = 0;
}

/*! Processes DDJVUAPI messages for this job. 
    The default implementation emits signals for
    the \a m_error, \a m_info, and \a m_progress
    messages.  The return value is a boolean indicating
    if the message has been processed or rejected. */

bool QDjVuJob::handle(ddjvu_message_t *msg)
{
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
    case DDJVU_PROGRESS:
        emit progress(msg->m_progress.percent);
        return true;
    default:
        break;
    }
    return false;
}

}

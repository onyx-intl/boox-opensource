#ifndef DJVU_JOB_H_
#define DJVU_JOB_H_

#include "djvu_utils.h"

namespace djvu_reader
{

class QDjVuJob : public QObject
{
    Q_OBJECT
public:
    QDjVuJob(ddjvu_job_t *job, QObject *parent=0);
    virtual ~QDjVuJob();
    operator ddjvu_job_t*() { return job; }

Q_SIGNALS:
    void error(QString msg, QString file_name, int line_no);
    void info(QString msg);
    void progress(int percent);

protected:
    virtual bool handle(ddjvu_message_t*);

private:
    ddjvu_job_t  *job;

private:
    friend class QDjVuContext;
    friend class QDjVuDocument;
};

};
#endif // DJVU_JOB_H_

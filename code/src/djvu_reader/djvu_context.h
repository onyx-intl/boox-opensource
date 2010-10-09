#ifndef DJVU_CONTEXT_H_
#define DJVU_CONTEXT_H_

#include "djvu_utils.h"

namespace djvu_reader
{

class QDjVuContext : public QObject 
{
    Q_OBJECT
    Q_PROPERTY(long cacheSize READ cacheSize WRITE setCacheSize)

private:
    static void callback(ddjvu_context_t *context, void *closure);
    ddjvu_context_t *context;
    bool flag;  // might become private pointer in the future
  
protected:
    virtual bool handle(ddjvu_message_t*);

public:
    virtual ~QDjVuContext();
    QDjVuContext(const char *programname=0, QObject *parent=0);
    long cacheSize() const;
    void setCacheSize(long);
    virtual bool event(QEvent*);
    operator ddjvu_context_t*() { return context; }
  
signals:
    void error(QString msg, QString filename, int lineno);
    void info(QString msg);
};

};

#endif // DJVU_CONTEXT_H_

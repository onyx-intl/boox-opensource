#ifndef DS_VIEW_H_
#define DS_VIEW_H_

#include <QtGui/QtGui>
#include "onyx/ui/ui.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/wireless/wifi_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "vsftpd.h"

using namespace ui;

class DSView : public QWidget
{
    Q_OBJECT

public:
    DSView(QWidget *parent = 0);
    ~DSView();

public:
    bool start();
    bool stop();
    bool exec(const QStringList & args);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *ke);
    virtual void closeEvent(QCloseEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void onStartClicked();
    void onCloseClicked();

private:
    void createLayout();
    void configNetwork();
    QString currentState();

    WifiDialog & wifiDialog();

private:
    scoped_ptr<WifiDialog> conf_dialog_;
    QHBoxLayout layout_;
    OnyxPushButton start_;
    OnyxPushButton close_;
    VsftpdServer vsftpd_;
};


#endif

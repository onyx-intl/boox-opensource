#ifndef MDIALOG_H
#define MDIALOG_H

#include <QtGui/QDialog>

#include "onyx/ui/catalog_view.h"

class QHBoxLayout;

using namespace ui;

class MDialog : public QDialog
{
    Q_OBJECT
public:
    MDialog(QWidget* parent = 0);
    void enableScreenUpdate(bool flag);

signals:
    void ActiveKey(qint32);
    void ActiveModeKey(qint32);

protected:
    virtual bool event(QEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent* );

private slots:
    void onItemActivated(CatalogView *, ContentView *, int);

private:
    void createLayout();
    void createKeysGroup(CatalogView &, const int );

private:
    QHBoxLayout *layout_;
    CatalogView mark_buttons_;
    CatalogView set_buttons_;
    bool enable_flush_flag_;
};

#endif // MDIALOG_H

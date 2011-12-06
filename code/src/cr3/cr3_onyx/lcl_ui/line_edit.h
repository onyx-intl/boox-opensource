#ifndef LCL_ONYX_LINE_EDIT_H_
#define LCL_ONYX_LINE_EDIT_H_

#include <QtGui/QtGui>

namespace ui
{

/// Line edit for eink device. Remove unnecessary updates.
class lcl_OnyxLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    lcl_OnyxLineEdit(QWidget *parent);
    lcl_OnyxLineEdit(const QString & text, QWidget *parent);
    ~lcl_OnyxLineEdit();

    void setName(QString name_) { name = name_; }
    void setMaxVal(int maxval_) { maxval = maxval_; }

protected:
    void focusInEvent(QFocusEvent *e);

Q_SIGNALS:
    void getFocus(lcl_OnyxLineEdit *object);
    void setCheckByMouse(lcl_OnyxLineEdit *object);
    void outOfRange(QKeyEvent *ke);
    void valueChanged(lcl_OnyxLineEdit *object);

protected:
    void keyReleaseEvent(QKeyEvent *ke);
    void keyPressEvent(QKeyEvent * ke);
    void mouseReleaseEvent(QMouseEvent * event);

private:
    bool out_of_range_;
    QString name;
    int maxval;
};

};

#endif  // ONYX_LINE_EDIT_H_

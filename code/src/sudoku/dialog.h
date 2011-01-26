/*#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/ui.h"

class QStackedWidget;
class QComboBox;
class QSettings;
class QSpinBox;
class QDialogButtonBox;
class QVBoxLayout;
class QLabel;
class QFormLayout;
//class OnyxDialog;
//using namespace ui;
namespace onyx {
namespace simsu {
class Dialog : public QDialog {
    Q_OBJECT
public:
    Dialog ( QWidget* parent = 0);
    virtual ~Dialog() {};
public slots:
    void accept();
signals:
    void setGameSignal ( int, int, int);
protected:
    void mouseMoveEvent ( QMouseEvent* event );
    bool event ( QEvent * event );
private:
    QStackedWidget* preview;
    QComboBox* symmetry_box;
    QSettings settings;
    QComboBox* algorithm_box;
    QSpinBox* seed_box;
    QDialogButtonBox* buttons;
    QVBoxLayout* layout;
};
}
}

#endif // DIALOG_H
*/
// kate: indent-mode cstyle; space-indent on; indent-width 0;

#ifndef ONYX_NUMBER_DIALOG_H_
#define ONYX_NUMBER_DIALOG_H_

#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/number_widget.h"
#include "onyx/ui/line_edit.h"
#include "onyx/ui/context_dialog_base.h"

namespace ui
{

class NumberDialog : public OnyxDialog
{
    Q_OBJECT
public:
    explicit NumberDialog(QWidget *parent, QString title);
    ~NumberDialog(void);

    /// Set value.
    void setValue(const int value);

    /// Retrieve number from dialog.
    int value() const { return value_ > total_ ? total_ : value_; }

    /// Show modal dialog. The return value can be
    /// accpeted or rejected.
    int popup(const int value, const int total);

protected:
    bool event(QEvent * event);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void onCloseClicked();

private:
    void createLayout();
    void clear();
    void clickFocusButton();
    void focusUpDownChild(bool up);

private Q_SLOTS:
    void onNumberClicked(const int);
    void onBackspaceClicked();
    void onOKClicked();

private:
    int                     value_;          ///< Current value
    int                     total_;          ///< Limitation of the total number
    QIntValidator           validator_;
    OnyxLineEdit           number_edit_;    ///< Number edit.
    NumberWidget            number_widget_;  ///< Number widget.

private:
    NO_COPY_AND_ASSIGN(NumberDialog);
};

}   // namespace ui

#endif      // ONYX_NUMBER_DIALOG_H_

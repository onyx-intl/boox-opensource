#ifndef ONYX_CLOCK_DIALOG_H_
#define ONYX_CLOCK_DIALOG_H_

#include "onyx/ui/onyx_dialog.h"
#include "onyx/sys/sys.h"

using namespace ui;

/// Clock dialog.
class ClockDialog : public OnyxDialog
{
    Q_OBJECT

public:
    explicit ClockDialog(const QDateTime & start, QWidget *parent = 0);
    ~ClockDialog(void);

public:
    int exec();

private:
    void createLayout();
    void updateText();

private:
    QDateTime start_;
    QVBoxLayout layout_;
    QHBoxLayout time_layout_;
    QHBoxLayout reading_layout_;
    OnyxLabel time_number_;
    OnyxLabel year_label_;
    QLabel separator_;
    OnyxLabel reading_label_;
};

/// Full screen dialog
class FullScreenClock : public QDialog
{
    Q_OBJECT

public:
    FullScreenClock(QWidget *parent);
    ~FullScreenClock(void);

public:
    int exec();

private:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent *e);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void createLayout();
    void paintEvent(QPaintEvent *);
    void drawTime(QPainter *);
    void drawDate(QPainter *);

private Q_SLOTS:
    void updateFSClock();
    void onReturn();
    void onOkClicked(bool);
    void onCloseClicked();

private:
    bool need_GC_;
};


#endif  // ONYX_CLOCK_DIALOG_H_

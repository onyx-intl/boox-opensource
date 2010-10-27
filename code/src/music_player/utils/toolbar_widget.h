#ifndef TOOLBAR_WIDGET_H_
#define TOOLBAR_WIDGET_H_

#include "player_utils.h"

namespace player
{

class ToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    ToolbarWidget(QWidget *parent = 0);
    ~ToolbarWidget();

    void pushButton(ui::OnyxPushButton *button);

private:
    void paintEvent(QPaintEvent *pe);
    void updatePath();

private:
    QHBoxLayout    layout_;
    QPainterPath   background_;
};

};

#endif

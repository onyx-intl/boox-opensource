#include "toolbar_widget.h"

using namespace ui;

namespace player
{

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , layout_(this)
{
    layout_.setSpacing(8);
    layout_.setContentsMargins(24, 0, 24, 0);

}

ToolbarWidget::~ToolbarWidget()
{
}

void ToolbarWidget::pushButton(OnyxPushButton *button)
{
    layout_.addWidget(button);
}

void ToolbarWidget::paintEvent(QPaintEvent *pe)
{
    updatePath();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(background_, Qt::gray);
}

void ToolbarWidget::updatePath()
{
    int height  = rect().height();
    int x_start = rect().left();
    int x_end   = rect().right();
    int y       = rect().top();

    const int ARC_RADIUS = 10;
    int diameter = (ARC_RADIUS << 1);

    QPainterPath path;
    path.moveTo(x_end, y + ARC_RADIUS);
    path.arcTo((x_end - diameter), y, diameter, diameter, 0.0, 90.0);
    path.lineTo(x_start + ARC_RADIUS, y);
    path.arcTo(x_start, y, diameter, diameter, 90.0, 90.0);
    //path.lineTo(x_start, y + (height - ARC_RADIUS));
    //path.arcTo(x_start, y + (height - diameter), diameter, diameter, 180.0, 90.0);
    //path.lineTo((x_end - ARC_RADIUS), y + height);
    //path.arcTo((x_end - diameter), y + (height - diameter), diameter, diameter, 270.0, 90.0);
    path.lineTo(x_start, y + height);
    path.lineTo(x_end, y + height);

    path.closeSubpath();

    background_ = path;
}

}

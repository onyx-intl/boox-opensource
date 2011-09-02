#include "button_view.h"
#include "onyx/data/data_tags.h"

CalculatorFactory::CalculatorFactory()
{
}

CalculatorFactory::~CalculatorFactory()
{
}

ContentView * CalculatorFactory::createView(QWidget *parent, const QString &type)
{
   ContentView * target = 0;

   if (type == "ButtonView")
   {
       target = new ButtonView(parent);
   }

   return target;
}

ButtonView::ButtonView(QWidget *parent)
: ContentView(parent)
{
}

ButtonView::~ButtonView()
{
}

const QString ButtonView::type()
{
    return "ButtonView";
}

void ButtonView::updateView()
{
    update();
}


void ButtonView::keyPressEvent(QKeyEvent *e)
{
    ContentView::keyPressEvent(e);
}

void ButtonView::keyReleaseEvent(QKeyEvent *e)
{
    ContentView::keyReleaseEvent(e);
}

void ButtonView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }
        else
        {
            QPen pen;
            pen.setWidth(2);
            pen.setColor(Qt::black);
            painter.setPen(pen);
            painter.drawRect(rect());
        }

    }

    drawTitle(painter, rect());
}

void ButtonView::drawTitle(QPainter & painter, QRect rect)
{
    int alignment = Qt::AlignCenter;
    if (data()->contains(TAG_ALIGN))
    {
        bool ok;
        int val = data()->value(TAG_ALIGN).toInt(&ok);
        if (ok)
        {
            alignment = val;
        }
    }
    ContentView::drawTitle(painter, rect, alignment);
}

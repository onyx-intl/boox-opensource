#ifndef RSS_CONTENT_VIEW_H_
#define RSS_CONTENT_VIEW_H_

#include "onyx/ui/content_view.h"
#include "onyx/ui/factory.h"
#include "button_view.h"

using namespace ui;

class CalculatorFactory : public Factory
{
public:
    CalculatorFactory();
    virtual ~CalculatorFactory();

public:
    virtual ContentView * createView(QWidget *parent, const QString &type = QString());
};

class ButtonView : public ContentView
{
    Q_OBJECT

public:
    ButtonView(QWidget *parent);
    virtual ~ButtonView();

    static const QString type();

public:
    virtual void updateView();

protected:
    void paintEvent(QPaintEvent * event);

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

private:
    QLabel label_title_;

    void drawTitle(QPainter & painter, QRect rect);
};

#endif //BUTTON_VIEW_H_

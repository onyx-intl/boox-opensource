#ifndef TEXT_WRAP_VIEW_H
#define TEXT_WRAP_VIEW_H

#include "onyx/base/base.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/content_view.h"
#include "onyx/ui/factory.h"

using namespace ui;

/// Cover view provides a cover and title support.
class TextWrapView : public ContentView
{
    Q_OBJECT

public:
    TextWrapView(QWidget *parent);
    virtual ~TextWrapView();

    static const QString type();

public:
    virtual void updateView();

protected:
    void paintEvent(QPaintEvent * event);
    void drawCover(QPainter & painter, QRect rect);
    void drawTitle(QPainter & painter, QRect rect);

};

class TextWrapViewFactory : public Factory
{
public:
    TextWrapViewFactory();
    ~TextWrapViewFactory();

public:
    virtual ContentView * createView(QWidget *parent, const QString &type);

};

#endif

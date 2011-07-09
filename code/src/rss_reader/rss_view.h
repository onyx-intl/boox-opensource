#ifndef RSS_CONTENT_VIEW_H_
#define RSS_CONTENT_VIEW_H_

#include "onyx/ui/content_view.h"
#include "onyx/ui/factory.h"
#include "onyx/ui/buttons.h"

using namespace ui;

namespace rss_reader
{

class RssFactory : public Factory
{
public:
    RssFactory();
    virtual ~RssFactory();

public:
    virtual ContentView * createView(QWidget *parent, const QString &type = QString());
};

class RssView : public ContentView
{
    Q_OBJECT

public:
    RssView(QWidget *parent);
    virtual ~RssView();

    static const QString type();

    static const QString TAG_NEW_ITEMS;
    static const QString TAG_ALL_ITEMS;
    static const QString TAG_UPDATE_TIME;

    enum {NONE, UPDATE};

public:
    virtual void updateView();

protected:
    void mouseReleaseEvent(QMouseEvent * event);
    void keyReleaseEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent * event);
    void drawTitle(QPainter & painter, QRect rect);

private:
    void createLayout();

private:
    QHBoxLayout h_layout_;
    QVBoxLayout v_layout_left_;
    QVBoxLayout v_layout_right_;

    QLabel label_title_;
    QLabel label_url_;
    QLabel label_updating_;
    QLabel label_update_;
    QLabel label_items_;
    QLabel label_time_;
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

class EditView : public ContentView
{
    Q_OBJECT

public:
    EditView(QWidget *parent);
    virtual ~EditView();

    static const QString type();

    enum ACTION {NONE, ADD, REMOVE, EDIT, CHECKBOX};

public:
    virtual void updateView();

protected:
    void showEvent ( QShowEvent * event );
    void keyReleaseEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent * event);
    void drawTitle(QPainter & painter, QRect rect);

    void mouseReleaseEvent(QMouseEvent * event);

private slots:
    void onCheckBoxClicked(bool state);
    void onRemove();
    void onEdit();

private:
    void createLayout();

private:
    QHBoxLayout h_layout_;

    QCheckBox checkbox_;

    QLabel label_title_;
    QLabel label_edit_;
    QLabel label_remove_;
};

};
#endif //RSS_CONTENT_VIEW_H_

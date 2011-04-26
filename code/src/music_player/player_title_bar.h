#ifndef ONYX_PLAYER_TITLE_BAR_H_
#define ONYX_PLAYER_TITLE_BAR_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/label.h"

using namespace ui;

namespace player
{

class PlayerTitleBar: public QWidget
{
    Q_OBJECT

public:
    explicit PlayerTitleBar(QWidget *parent = 0);
    ~PlayerTitleBar();

private:
    void createLayout();
    void paintEvent(QPaintEvent * event);

private:
    QHBoxLayout layout_;

    OnyxLabel window_icon_label_;
    QTextLayout text_layout_;
};

}   // namespace player

#endif

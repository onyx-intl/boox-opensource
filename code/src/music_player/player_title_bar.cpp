#include "player_title_bar.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/text_layout.h"

namespace player
{

static const int HEIGTH = 36;
static const int MARGIN  = 8;

PlayerTitleBar::PlayerTitleBar(QWidget *parent)
    : QWidget(parent)
    , layout_(this)
{
    text_layout_.setText("");
    createLayout();
}

PlayerTitleBar::~PlayerTitleBar()
{
}

void PlayerTitleBar::createLayout()
{
    window_icon_label_.setPixmap(QPixmap(":/player_icons2/music_player.png"));
    window_icon_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    window_icon_label_.setFixedHeight(HEIGTH);

    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.addSpacing(6);
    layout_.addWidget(&window_icon_label_);
}

void PlayerTitleBar::paintEvent(QPaintEvent * event)
{
    QString music_player_title(QCoreApplication::tr("Music Player"));
    QFont font;
    font.setPointSize(22);
    ui::calculateSingleLineLayout(text_layout_, font, music_player_title,
                Qt::AlignLeft | Qt::AlignVCenter,
                rect().adjusted(MARGIN, 0, -MARGIN, 0), Qt::ElideLeft);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath roundRectPath;
    QRect rc = rect();
    static const int radius = 15;

    roundRectPath.moveTo(rc.bottomRight());
    roundRectPath.lineTo(rc.right(), rc.top() + radius);
    QRect r1(rc.right() - radius, rc.top(), radius, radius);
    roundRectPath.arcTo(r1, 0, 90);
    roundRectPath.lineTo(rc.left() + radius, rc.top());
    QRect r2(rc.left(), rc.top(), radius, radius);
    roundRectPath.arcTo(r2, 90, 90);
    roundRectPath.lineTo(rc.bottomLeft());
    roundRectPath.lineTo(rc.bottomRight());

    QBrush brush(Qt::white);
    brush.setColor(QColor(64, 63, 59));
    painter.fillPath(roundRectPath, brush);
    painter.drawPath(roundRectPath);

    QPen pen(Qt::white);
    pen.setWidth(2);
    int start_x = 50;
    painter.setPen(pen);
    text_layout_.draw(&painter, QPoint(start_x, 0));
}

}   // namespace player

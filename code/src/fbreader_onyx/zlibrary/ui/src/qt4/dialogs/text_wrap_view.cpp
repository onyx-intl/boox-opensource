#include "text_wrap_view.h"
#include "onyx/data/data_tags.h"

static const int MARGIN = 4;
static const int LEFT_MARGIN = 10;
static const int TOP_MARGIN = 10;

TextWrapView::TextWrapView(QWidget *parent)
    : ContentView(parent)
{
}

TextWrapView::~TextWrapView()
{
}

const QString TextWrapView::type()
{
    return "TextWrapView";
}

void TextWrapView::updateView()
{
    update();
}

void TextWrapView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (isChecked())
        {
             painter.fillRect(rect().adjusted(penWidth(), penWidth(),
                     -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }

        drawCover(painter, rect());
        if (isPressed() || isChecked())
        {
            painter.setPen(Qt::black);
        }
        drawTitle(painter, rect());
    }
}

void TextWrapView::drawCover(QPainter & painter, QRect rect)
{
    if (data() && data()->contains(TAG_COVER))
    {
        QPixmap pixmap(qVariantValue<QPixmap>(data()->value(TAG_COVER)));
        int x = (rect.width() - pixmap.width()) / 2;
        painter.drawPixmap(x, MARGIN, pixmap);
    }
}

void TextWrapView::drawTitle(QPainter & painter, QRect rect)
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
    QTextOption text_option;
    text_option.setAlignment((Qt::Alignment)alignment);
    text_option.setWrapMode(QTextOption::WordWrap);

    if (data() && data()->contains(TAG_TITLE))
    {
        QString family = data()->value(TAG_FONT_FAMILY).toString();
        if (family.isEmpty())
        {
            family = QApplication::font().family();
        }

        int size = data()->value(TAG_FONT_SIZE).toInt();
        if (size <= 0)
        {
            size = ui::defaultFontPointSize();
        }
        QFont font(family, size);

        int is_align_left = alignment & Qt::AlignLeft;
        if (is_align_left)
        {
            rect.adjust(10, 0, 0, 0);
        }

        int is_align_right = alignment & Qt::AlignRight;
        if (is_align_right)
        {
            rect.adjust(0, 0, -10, 0);
        }

        int is_align_top = alignment & Qt::AlignTop;
        if (is_align_top)
        {
            rect.adjust(0, 5, 0, 0);
        }

        QString text = data()->value(TAG_TITLE).toString();
        QFontMetrics metrics(font);
        QTextLayout text_layout(text);
        text_layout.setFont(font);
        text_layout.beginLayout();
        QTextLine line = text_layout.createLine();
        line.setLineWidth(rect.width());
        line.setPosition(QPointF(0, 0));

        line = text_layout.createLine();
        if (line.isValid())
        {
            line.setLineWidth(rect.width());
            line.setPosition(QPointF(0, 5+20));
        }
        text_layout.endLayout();
        text_layout.draw(&painter, rect.topLeft());
    }
}

TextWrapViewFactory::TextWrapViewFactory()
    : Factory()
{

}

TextWrapViewFactory::~TextWrapViewFactory()
{

}

ContentView * TextWrapViewFactory::createView(QWidget *parent,
        const QString &type)
{
    return new TextWrapView(parent);
}


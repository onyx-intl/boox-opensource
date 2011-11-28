#include <QApplication>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLabel>
#include "gomoku_widget.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

bool GomokuWidget::findFont(QString name)
{
    fontName = "";
    QFont font(name);
    if (font.family() != name)
        return false;
    fontName = name;
    return true;
}

void GomokuWidget::newGame()
{
    ruchy.clear();
    currentX = currentY = board.size() / 2;
    wygral = 0;
    board.clear();
    isEnd = false;
}

GomokuWidget::GomokuWidget(QWidget *parent)
    : QWidget(parent),
      isEnd(false),
      board(15)
{
    setWindowTitle(tr("BOOX Gomoku"));
    setStyleSheet("background-color: white;");
    setWindowFlags(Qt::FramelessWindowHint);
    //showMaximized();
    flashScreen = false;

    newGame();
    onyx::screen::watcher().addWatcher(this);

    if (findFont("Droid Serif")) return;
    if (findFont("Droid Sans")) return;
    if (findFont("DejaVu Sans")) return;
}

void GomokuWidget::refreshScreen()
{
    if (flashScreen)
    {
        flashScreen = false;
        onyx::screen::ScreenProxy::instance().flush(this, onyx::screen::ScreenProxy::GU);
    }
    else
    {
        onyx::screen::ScreenProxy::instance().flush(this, onyx::screen::ScreenProxy::DW);
    }
    if (wygral > 0 && !isEnd)
    {
        repaint();
        onyx::screen::instance().flush(onyx::screen::ScreenProxy::GC);
        isEnd = true;
    }
}

int GomokuWidget::cellSize()
{
    int w, h;
    w = width() - 0;
    h = height() - 0;
    if (w > h) w = h;
    else h = w;
    return w / board.size();
}

int GomokuWidget::fromLeft()
{
    if (width() > height())
        return width() - getSize() - 15;
    return (width() - getSize()) / 2;
}

void GomokuWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(Qt::black, 1.0));
    int half = cellSize() / 2;
    for (int x = 0; x < board.size(); x++)
        painter.drawLine(cellLeft(x) + half, fromTop() + half, cellLeft(x) + half, cellTop(board.size()) - half);
    for (int y = 0; y < board.size(); y++)
        painter.drawLine(fromLeft() + half, cellTop(y) + half, cellLeft(board.size()) - half, cellTop(y) + half);

    {
        QFont fontBit(fontName, 40, QFont::Normal);
        painter.setFont(fontBit);
        painter.setPen(QPen(Qt::black, 1.0));
        painter.drawText(QRect(15, 0, (width() < height() ? width() : fromLeft()) - 20, 60), width() < height() ? Qt::AlignHCenter : Qt::AlignLeft, QString::fromUtf8("Gomoku"));

        QFont font(fontName, 24, QFont::Normal);
        painter.setFont(font);
        if(width() < height())
        {
            QString s=tr("Your points: ") + QString::number(board.getPoints(1))+"        "+
                      tr("Boox’s points: ") + QString::number(board.getPoints(2));
            painter.drawText(QRect(0, 60, width(), 30), Qt::AlignHCenter, s);
        }
        else
        {
            painter.drawText(QRect(15, 65, fromLeft() - cellSize()/2, 30), Qt::AlignLeft, tr("Your points: ") + QString::number(board.getPoints(1)));
            painter.drawText(QRect(15, 100, fromLeft() - cellSize()/2, 30), Qt::AlignLeft, tr("Boox’s points: ") + QString::number(board.getPoints(2)));
        }

        painter.setOpacity(1);
        QFont font1(fontName, 40, QFont::Normal);
        painter.setFont(font1);
        painter.setPen(QPen(Qt::black, 1.0));
        if(width() < height())
        {
            if (wygral < 1)
                painter.drawText(0, 90, width(), 50, Qt::AlignHCenter, "...");
            else if (wygral == 1)
                painter.drawText(0, 90, width(), 50, Qt::AlignHCenter, tr("You won!"));
            else if (wygral == 2)
                painter.drawText(0, 90, width(), 50, Qt::AlignHCenter, tr("Boox won!"));
            else
                painter.drawText(0, 90, width(), 50, Qt::AlignHCenter, tr("Draw."));
        }
        else
        {
            if (wygral < 1)
                painter.drawText(15, 160, fromLeft() - cellSize()/2, 60, Qt::AlignLeft, "...");
            else if (wygral == 1)
                painter.drawText(15, 160, fromLeft() - cellSize()/2, 60, Qt::AlignLeft, tr("You won!"));
            else if (wygral == 2)
                painter.drawText(15, 160, fromLeft() - cellSize()/2, 60, Qt::AlignLeft, tr("Boox won!"));
            else
                painter.drawText(15, 160, fromLeft() - cellSize()/2, 60, Qt::AlignLeft, tr("Draw."));
        }
    }

    {
        int x1 = cellLeft(currentX) + 1;
        int y1 = cellTop(currentY) + 1;
        int x2 = cellLeft(currentX) + cellSize() - 1;
        int y2 = y1;
        int x3 = x1;
        int y3 = cellTop(currentY) + cellSize() - 1;
        int x4 = x2;
        int y4 = y3;
        int gap = cellSize() / 3;

        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(x1, y1, x1+gap, y1);
        painter.drawLine(x1, y1, x1, y1+gap);
        painter.drawLine(x2, y2, x2-gap, y2);
        painter.drawLine(x2, y2, x2, y2+gap);
        painter.drawLine(x3, y3, x3+gap, y3);
        painter.drawLine(x3, y3, x3, y3-gap);
        painter.drawLine(x4, y4, x4-gap, y4);
        painter.drawLine(x4, y4, x4, y4-gap);
    }

    for (int x = 0; x < board.size(); x++)
    for (int y = 0; y < board.size(); y++)
        if (board.is(1, x, y))
        {
            painter.setPen(QPen(Qt::black, 3));
            painter.setBrush(Qt::black);
            painter.drawEllipse(cellLeft(x) + 4, cellTop(y) + 4, cellSize() - 8, cellSize() - 8);
        }
        else if (board.is(2, x, y))
        {
            painter.setPen(QPen(Qt::black, 3));
            painter.setBrush(Qt::white);
            painter.drawEllipse(cellLeft(x) + 4, cellTop(y) + 4, cellSize() - 8, cellSize() - 8);
        }
}

void GomokuWidget::move()
{
    if (board.getField(currentX, currentY) > 0) return;
    board.getField(currentX, currentY) = 1;
    board.getMoveNumber(currentX, currentY) = ruchy.count() / 2 + 1;
    ruchy.push(currentX);
    ruchy.push(currentY);
    if (board.hasWon(1))
    {
        ruchy.push(-1);
        ruchy.push(-1);
        wygral = 1;
        return;
    }
    if (board.isDraw())
    {
        ruchy.push(-1);
        ruchy.push(-1);
        wygral = 3;
        return;
    }
    int x, y;
    board.getNextMove(x, y, 2);
    currentX = x;
    currentY = y;
    board.getMoveNumber(currentX, currentY) = ruchy.count() / 2 + 1;
    ruchy.push(currentX);
    ruchy.push(currentY);
    if (board.hasWon(2))
    {
        wygral = 2;
        return;
    }
    if (board.isDraw())
    {
        wygral = 3;
        return;
    }
}

void GomokuWidget::undo()
{
    if (ruchy.isEmpty()) return;
    int y = ruchy.pop();
    int x = ruchy.pop();
    if (x >= 0 && y >= 0) // in case of -1 (user won)
    {
        board.getField(currentX = x, currentY = y) = 0;
    }
    y = ruchy.pop();
    x = ruchy.pop();
    board.getField(currentX = x, currentY = y) = 0;
    wygral = 0;
}

bool GomokuWidget::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        refreshScreen();
    }
    return ret;
}

void GomokuWidget::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_PageDown:
        newGame();
        flashScreen = true;
        update();
        break;

    case Qt::Key_PageUp:
        undo();
        flashScreen = true;
        update();
        break;

    case Qt::Key_Right:
        if (wygral > 0) break;
        currentX++;
        currentX %= board.size();
        update();
        break;

    case Qt::Key_Left:
        if (wygral > 0) break;
        currentX += board.size()-1;
        currentX %= board.size();
        update();
        break;

    case Qt::Key_Up:
        if (wygral > 0) break;
        currentY += board.size()-1;
        currentY %= board.size();
        update();
        break;

    case Qt::Key_Down:
        if (wygral > 0) break;
        currentY++;
        currentY %= board.size();
        update();
        break;

    case Qt::Key_Return:
        if (wygral > 0)
        {
            break;
        }
        move();
        flashScreen = true;
        update();
        break;
    }
    refreshScreen();
}

void GomokuWidget::mousePressEvent(QMouseEvent *event)
{
    if (wygral > 0) return;
    int x = (event->pos().x() - fromLeft()) / cellSize();
    int y = (event->pos().y() - fromTop()) / cellSize();
    if (x >= 0 && x < board.size() && y >= 0 && y < board.size())
    {
        currentX = x;
        currentY = y;

        move();
        flashScreen = true;
        update();
        refreshScreen();
    }
}

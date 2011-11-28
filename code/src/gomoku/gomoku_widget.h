#ifndef GOMOKU_WIDGET_H
#define GOMOKU_WIDGET_H

#include <QWidget>
#include <Qt/qstack.h>

#include "board.h"

class GomokuWidget : public QWidget
{
private:
    int currentX;
    int currentY;
    int wygral;
    bool flashScreen;
    bool isEnd;
    QString fontName;
    QStack<int> ruchy;
    Board board;

private:
    bool findFont(QString name);

public:
    GomokuWidget(QWidget *parent = 0);

    void refreshScreen();
    void newGame();

private:
    int cellSize();
    int getSize() { return board.size() * cellSize(); }
    int fromTop() { return ((((height() - getSize()) / 2)>200 || width()>height()) ?((height() - getSize()) / 2) :150); }
    int fromLeft();
    int cellTop(int y) { return fromTop() + y * cellSize(); }
    int cellLeft(int x) { return fromLeft() + x * cellSize(); }
    void paintEvent(QPaintEvent *e);
    void move();
    void undo();
    bool event(QEvent * event);
    void keyReleaseEvent(QKeyEvent *ke);
    void mousePressEvent(QMouseEvent *event);
};

#endif // GOMOKU_WIDGET_H

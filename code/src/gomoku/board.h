#ifndef __BOARD_H__
#define __BOARD_H__

class Board
{
private:
    int boardSize;
    int ja;
    int przeciwnik;

    int *siatka;
    int *kolejnosc;

    int ok(int x, int y);
    int no(int x, int y);
    int points(int x, int y);
    bool check(int x, int y);

public:
    Board(int size);
    virtual ~Board();

    int size();
    void clear();

    int& getField(int x, int y);
    int& getMoveNumber(int x, int y);
    bool is(int e, int x, int y);

    void getNextMove(int& retx, int& rety, int czyj_ruch);
    bool hasWon(int gracz);
    bool isDraw();
    int getPoints(int gracz);
};

#endif

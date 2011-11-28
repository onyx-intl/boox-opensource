#include <cstdlib>
#include "board.h"

/**/
#define XXXX  (1 << 18) // bardziej ofensywny
#define XXX   (1 <<  9)
#define XX    (1 <<  6)
#define X     (1 <<  3)
#define OOOO  (1 << 15)
#define OOO   (1 << 12)
#define OO    (1 <<  3)
#define O     (1 <<  0)
/** /
#define XXXX  (1 << 21) // bardziej defensywny
#define XXX   (1 << 12)
#define XX    (1 <<  9)
#define X     (1 <<  0)
#define OOOO  (1 << 18)
#define OOO   (1 << 15)
#define OO    (1 <<  6)
#define O     (1 <<  3)
/** /
#define XXXX  (1 << 18) // mieszany
#define XXX   (1 << 9)
#define XX    (1 << (4 + (rand() % 2)))
#define X     (1 << (rand() % 3))
#define OOOO  (1 << 15)
#define OOO   (1 << 12)
#define OO    (1 << (4 + (rand() % 3)))
#define O     (1 << (rand() % 4))
/**/

Board::Board(int size)
{
    boardSize = size;
    siatka = new int[size * size];
    kolejnosc = new int[size * size];
}

Board::~Board()
{
    delete[] siatka;
    delete[] kolejnosc;
}

int Board::size()
{
    return boardSize;
}

void Board::clear()
{
    for (int x = 0; x < boardSize; x++)
    for (int y = 0; y < boardSize; y++)
    {
        getField(x, y) = 0;
        getMoveNumber(x, y) = 0;
    }
}

int& Board::getField(int x, int y)
{
    return siatka[x + boardSize * y];
}

int& Board::getMoveNumber(int x, int y)
{
    return kolejnosc[x + boardSize * y];
}

int Board::ok(int x, int y)
{
	if ((x < 0) || (y < 0) || (x > boardSize-1) || (y > boardSize-1)) return 0;
	return getField(x, y) == ja ? 1 : 0;
}

int Board::no(int x, int y)
{
	if ((x < 0) || (y < 0) || (x > boardSize-1) || (y > boardSize-1)) return 0;
	return getField(x, y) == przeciwnik ? 1 : 0;
}

int Board::points(int x, int y)
{
	int p = 0;

	for (int ix = -1; ix < 2; ix++)
	{
		for (int iy = -1; iy < 2; iy++)
		{
			if (!(ix || iy)) continue;
			int t = 1;
			for (int i = 1; i < 5; i++)
			{
				if (ok(x + ix * i, y + iy * i)) t++;
				else break;
			}
			for (int i = 1; i < 5; i++)
			{
				if (ok(x - ix * i, y - iy * i)) t++;
				else break;
			}
			switch (t)
			{
				case 9: case 8: case 7: case 6:
				case 5: p += XXXX; break;
				case 4: p +=  XXX; break;
				case 3: p +=   XX; break;
				case 2: p +=    X; break;
			}

			t = 1;
			for (int i = 1; i < 5; i++)
			{
				if (no(x + ix * i, y + iy * i)) t++;
				else break;
			}
			for (int i = 1; i < 5; i++)
			{
				if (no(x - ix * i, y - iy * i)) t++;
				else break;
			}
			switch (t)
			{
				case 9: case 8:	case 7: case 6:
				case 5: p += OOOO; break;
				case 4: p +=  OOO; break;
				case 3: p +=   OO; break;
				case 2: p +=    O; break;
			}
		}
	}

	return p;
}

void Board::getNextMove(int& retx, int& rety, int czyj_ruch)
{
	ja = czyj_ruch;
	przeciwnik = ja == 1 ? 2 : 1;

	int xmax = 0, ymax = 0;
	int pmax = 0;

	for (int x = 0; x < boardSize; x++)
	{
		for (int y = 0; y < boardSize; y++)
		{
			if (getField(x, y) != 0) continue;
			int p = points(x, y);
			if (p > pmax)
			{
				xmax = x;
				ymax = y;
				pmax = p;
			}
		}
	}

	if (pmax == 0)
	{
		do
		{
			xmax = rand() % boardSize;
			ymax = rand() % boardSize;
		}
		while (getField(xmax, ymax) != 0);
	}

    getField(retx = xmax, rety = ymax) = ja;
}

bool Board::is(int e, int x, int y)
{
    return e == getField(x, y);
}

bool Board::check(int x, int y)
{
    int e = getField(x, y);
	if (e == 0) return false;
	if (x < boardSize-4)
	{
		if (is(e, x + 1, y) &&
			is(e, x + 2, y) &&
			is(e, x + 3, y) &&
			is(e, x + 4, y)) return true;
	}
	if (y < boardSize-4)
	{
		if (is(e, x, y + 1) &&
			is(e, x, y + 2) &&
			is(e, x, y + 3) &&
			is(e, x, y + 4)) return true;
	}
	if (x < boardSize-4 && y < boardSize-4)
	{
		if (is(e, x + 1, y + 1) &&
			is(e, x + 2, y + 2) &&
			is(e, x + 3, y + 3) &&
			is(e, x + 4, y + 4)) return true;
	}
	if (x > 3 && y < 21)
	{
		if (is(e, x - 1, y + 1) &&
			is(e, x - 2, y + 2) &&
			is(e, x - 3, y + 3) &&
			is(e, x - 4, y + 4)) return true;
	}
	return false;
}

bool Board::hasWon(int gracz)
{
    for (int x = 0; x < boardSize; x++)
    for (int y = 0; y < boardSize; y++)
        if (is(gracz, x, y) && check(x, y)) return true;
    return false;
}

bool Board::isDraw()
{
    for (int x = 0; x < boardSize; x++)
    for (int y = 0; y < boardSize; y++)
        if (is(0, x, y)) return false;
    return true;
}

int Board::getPoints(int gracz)
{
    int p = 0;
	for (int x = 0; x < boardSize; x++)
    for (int y = 0; y < boardSize; y++)
    if (is(gracz, x, y))
    {
        int temp = points(x, y);
        temp &= temp - 1;
        while (temp > 0)
        {
            p++;
            temp /= 2;
        }
    }
    if (hasWon(gracz)) p *= 2;
    return p;
}


#ifndef PLAYER_RECYCLER_H_
#define PLAYER_RECYCLER_H_

#include <utils/player_utils.h>

namespace player
{

class Buffer;

/// The Recycler class provides a queue of audio buffers.
class Recycler
{
public:
    Recycler(unsigned int sz);
    ~Recycler();

    bool full() const;
    bool empty() const;
    int available() const;
    int used() const;
    Buffer *next();
    Buffer *get(unsigned long size); // get next in recycle
    void add(); // add to queue
    void done(); // add to recycle
    void clear(); // clear queue
    unsigned int size() const; // size in bytes

    QMutex *mutex(){ return &mtx; }
    QWaitCondition *cond(){ return &cnd; }

private:
    unsigned int buffer_count;
    unsigned int add_index;
    unsigned int done_index;
    unsigned int current_count;
    Buffer **buffers;
    QMutex mtx;
    QWaitCondition cnd;
};

};

#endif // PLAYER_RECYCLER_H_

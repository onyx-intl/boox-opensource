#ifndef PLAYER_BUFFER_H_
#define PLAYER_BUFFER_H_

#include "constants.h"

namespace player
{

/// Audio buffer class.
class Buffer
{
public:
    Buffer()
    {
        data = new unsigned char[Buffer::size()];
        nbytes = 0;
        rate = 0;
        exceeding = 0;
        seeking_finished = false;
    }
    ~Buffer()
    {
        delete data;
        data = 0;
        nbytes = 0;
        rate = 0;
        exceeding = 0;
    }

    unsigned char *data;      /*!< Audio data */
    unsigned long nbytes;     /*!< Audio data size */
    unsigned long rate;       /*!< Buffer bitrate */
    unsigned long exceeding;  /*!< The number of bytes on which the size of buffer exceeds the size of the block */
    bool          seeking_finished; /*!< Is seeking now */

    static unsigned long size()
    {
        return globalBlockSize;
    }
};

};

#endif

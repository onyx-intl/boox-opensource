#ifndef PLAYER_DECODER_MAD_H_
#define PLAYER_DECODER_MAD_H_

#include <utils/player_utils.h>
#include <core/decoder.h>
#include "decodermadfactory.h"

extern "C"
{
#include <mad.h>
}

namespace player
{

class DecoderMAD : public Decoder
{
public:
    DecoderMAD(QObject *parent = 0,
               DecoderFactory *d = 0,
               QIODevice *i = 0,
               Output *o = 0);
    virtual ~DecoderMAD();

    // standard decoder API
    bool initialize();
    qint64 totalTime();
    void stop();

private:
    // thread run function
    void run();

    enum mad_flow madOutput();
    enum mad_flow madError(struct mad_stream *, struct mad_frame *);

    // helper functions
    void flush(bool = FALSE);
    void deinit();
    bool findHeader();
    bool findXingHeader(struct mad_bitptr, unsigned int);
    uint findID3v2(uchar *data, ulong size);

private:
    // MAD decoder
    struct
    {
        int flags;
        unsigned long frames;
        unsigned long bytes;
        unsigned char toc[100];
        long scale;
    } xing;

    enum
    {
        XING_FRAMES = 0x0001,
        XING_BYTES  = 0x0002,
        XING_TOC    = 0x0004,
        XING_SCALE  = 0x0008
    };

    struct mad_stream stream;
    struct mad_frame frame;
    struct mad_synth synth;

private:
    bool          inited_;
    bool          user_stop_;
    bool          done_;
    bool          finish_;
    bool          derror_;
    bool          eof_;
    bool          useeq_;
    qint64        total_time_;
    int           channels_;
    unsigned long bitrate_;
    long          freq_;
    long          len_;
    unsigned int  bks_;
    mad_fixed_t   eqbands_[32];

    // file input buffer
    char*         input_buf_;
    unsigned long input_bytes_;

    // output buffer
    char*         output_buf_;
    unsigned long output_bytes_;
    unsigned long output_at_;
    unsigned long output_size_;

};

};

#endif // PLAYER_DECODER_MAD_H_

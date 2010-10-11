#include <math.h>
#include <stdio.h>

#include <id3v2header.h>
#include <tbytevector.h>

#include <core/constants.h>
#include <core/buffer.h>
#include <core/output.h>

#include "decoder_mad.h"
#include "tagextractor.h"

namespace player
{

#define XING_MAGIC (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')
#define INPUT_BUFFER_SIZE (32*1024)


DecoderMAD::DecoderMAD(QObject *parent, DecoderFactory *d, QIODevice *i, Output *o)
    : Decoder(parent, d, i, o)
    , inited_(false)
    , user_stop_(false)
    , done_(false)
    , finish_(false)
    , derror_(false)
    , eof_(false)
    , useeq_(false)
    , total_time_(0)
    , channels_(0)
    , bks_(0)
    , bitrate_(0)
    , freq_(0)
    , len_(0)
    , input_buf_(0)
    , input_bytes_(0)
    , output_buf_(0)
    , output_bytes_(0)
    , output_at_(0)
    , output_size_(0)
{
}

DecoderMAD::~DecoderMAD()
{
    wait();
    deinit();
    mutex()->lock();
    if (input_buf_)
    {
        qDebug("DecoderMAD: deleting input_buf_");
        delete [] input_buf_;
    }
    input_buf_ = 0;

    if (output_buf_)
    {
        qDebug("DecoderMAD: deleting output_buf_");
        delete [] output_buf_;
    }
    output_buf_ = 0;
    mutex()->unlock();
}

bool DecoderMAD::initialize()
{
    bks_ = Buffer::size();

    inited_ = false;
    user_stop_ = false;
    done_ = false;
    finish_ = false;
    derror_ = false;
    eof_ = false;
    total_time_ = 0.;
    seek_time_ = -1.;
    channels_ = 0;
    bitrate_ = 0;
    freq_ = 0;
    len_ = 0;
    input_bytes_ = 0;
    output_bytes_ = 0;
    output_at_ = 0;
    output_size_ = 0;

    if (! input())
    {
        qWarning("DecoderMAD: cannot initialize.  No input.");
        return FALSE;
    }

    if (!input_buf_)
    {
        input_buf_ = new char[INPUT_BUFFER_SIZE];
    }

    if (!output_buf_)
    {
        output_buf_ = new char[globalBufferSize];
    }

    if (!input()->isOpen())
    {
        if (! input()->open(QIODevice::ReadOnly))
        {
            qWarning("DecoderMAD: %s", qPrintable(input()->errorString ()));
            return FALSE;
        }
    }

    if (input()->isSequential ()) //for streams only
    {
        TagExtractor extractor(input());
        stateHandler()->dispatch(extractor.id3v2tag());
    }

    mad_stream_init(&stream);
    mad_frame_init(&frame);
    mad_synth_init(&synth);

    if (! findHeader())
    {
        qDebug("DecoderMAD: Can't find a valid MPEG header.");
        return FALSE;
    }
    mad_stream_buffer(&stream, (unsigned char *) input_buf_, input_bytes_);
    stream.error = MAD_ERROR_NONE;
    stream.error = MAD_ERROR_BUFLEN;
    mad_frame_mute (&frame);
    stream.next_frame = NULL;
    stream.sync = 0;
    configure(freq_, channels_, 16);

    inited_ = TRUE;
    return TRUE;
}


void DecoderMAD::deinit()
{
    if (!inited_)
        return;

    mad_synth_finish(&synth);
    mad_frame_finish(&frame);
    mad_stream_finish(&stream);

    inited_ = false;
    user_stop_ = false;
    done_ = false;
    finish_ = false;
    derror_ = false;
    eof_ = false;
    useeq_ = false;
    total_time_ = 0.;
    seek_time_ = -1.;
    channels_ = 0;
    bks_ = 0;
    bitrate_ = 0;
    freq_ = 0;
    len_ = 0;
    input_bytes_ = 0;
    output_bytes_ = 0;
    output_at_ = 0;
    output_size_ = 0;
}

bool DecoderMAD::findXingHeader(struct mad_bitptr ptr, unsigned int bitlen)
{
    if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
    {
        goto fail;
    }

    xing.flags = mad_bit_read(&ptr, 32);
    bitlen -= 64;

    if (xing.flags & XING_FRAMES)
    {
        if (bitlen < 32)
            goto fail;

        xing.frames = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    if (xing.flags & XING_BYTES)
    {
        if (bitlen < 32)
            goto fail;

        xing.bytes = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    if (xing.flags & XING_TOC)
    {
        int i;

        if (bitlen < 800)
            goto fail;

        for (i = 0; i < 100; ++i)
            xing.toc[i] = mad_bit_read(&ptr, 8);

        bitlen -= 800;
    }

    if (xing.flags & XING_SCALE)
    {
        if (bitlen < 32)
            goto fail;

        xing.scale = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    return true;

fail:
    xing.flags = 0;
    xing.frames = 0;
    xing.bytes = 0;
    xing.scale = 0;
    return false;
}

bool DecoderMAD::findHeader()
{
    bool result = FALSE;
    int count = 0;
    bool has_xing = FALSE;
    bool is_vbr = FALSE;
    mad_timer_t duration = mad_timer_zero;
    struct mad_header header;
    mad_header_init (&header);

    while (TRUE)
    {
        input_bytes_ = 0;
        if (stream.error == MAD_ERROR_BUFLEN || !stream.buffer)
        {
            size_t remaining = 0;

            if (!stream.next_frame)
            {
                remaining = stream.bufend - stream.next_frame;
                memmove (input_buf_, stream.next_frame, remaining);
            }

            input_bytes_ = input()->read(input_buf_ + remaining, INPUT_BUFFER_SIZE - remaining);

            if (input_bytes_ <= 0)
                break;

            mad_stream_buffer(&stream, (unsigned char *) input_buf_ + remaining, input_bytes_);
            stream.error = MAD_ERROR_NONE;
        }

        if (mad_header_decode(&header, &stream) == -1)
        {
            if (stream.error == MAD_ERROR_BUFLEN)
                continue;
            else if (MAD_RECOVERABLE(stream.error))
                continue;
            else
            {
                qDebug ("DecoderMAD: Can't decode header: %s", mad_stream_errorstr(&stream));
                break;
            }
        }
        result = TRUE;

        if (input()->isSequential())
            break;

        count ++;
        //try to detect xing header
        if (count == 1)
        {
            frame.header = header;
            if (mad_frame_decode(&frame, &stream) != -1 &&
                    findXingHeader(stream.anc_ptr, stream.anc_bitlen))
            {
                is_vbr = TRUE;

                qDebug ("DecoderMAD: Xing header detected");

                if (xing.flags & XING_FRAMES)
                {
                    has_xing = TRUE;
                    count = xing.frames;
                    break;
                }
            }
        }
        //try to detect VBR
        if (!is_vbr && !(count > 15))
        {
            if (bitrate_ && header.bitrate != bitrate_)
            {
                qDebug ("DecoderMAD: VBR detected");
                is_vbr = TRUE;
            }
            else
                bitrate_ = header.bitrate;
        }
        else if (!is_vbr)
        {
            qDebug ("DecoderMAD: Fixed rate detected");
            break;
        }
        mad_timer_add (&duration, header.duration);
    }

    if (!result)
        return FALSE;

    if (!is_vbr && !input()->isSequential())
    {
        double time = (input()->size() * 8.0) / (header.bitrate);
        double timefrac = (double)time - ((long)(time));
        mad_timer_set(&duration, (long)time, (long)(timefrac*100), 100);
    }
    else if (has_xing)
    {
        mad_timer_multiply (&header.duration, count);
        duration = header.duration;
    }

    total_time_ = mad_timer_count(duration, MAD_UNITS_MILLISECONDS);
    qDebug ("DecoderMAD: Total time: %ld", long(total_time_));
    freq_ = header.samplerate;
    channels_ = MAD_NCHANNELS(&header);
    bitrate_ = header.bitrate / 1000;
    mad_header_finish(&header);
    input()->seek(0);
    input_bytes_ = 0;
    return TRUE;
}

qint64 DecoderMAD::totalTime()
{
    if (! inited_)
        return 0.;
    return total_time_;
}

void DecoderMAD::stop()
{
    user_stop_ = TRUE;
}

void DecoderMAD::flush(bool final)
{
    ulong min = final ? 0 : bks_;
    while (!done_ && (output_bytes_ > min) && seek_time_ == -1.)
    {
        output()->recycler()->mutex()->lock();

        while (!done_ && output()->recycler()->full())
        {
            mutex()->unlock();
            output()->recycler()->cond()->wait(output()->recycler()->mutex());

            mutex()->lock();
            done_ = user_stop_;
        }

        if (user_stop_)
        {
            inited_ = FALSE;
            done_ = TRUE;
        }
        else
        {
            output_bytes_ -= produceSound(output_buf_, output_bytes_, bitrate_, channels_);
            output_size_ += bks_;
            output_at_ = output_bytes_;
        }

        if (output()->recycler()->full())
        {
            output()->recycler()->cond()->wakeOne();
        }

        output()->recycler()->mutex()->unlock();
    }
}

void DecoderMAD::run()
{
    int skip_frames = 0;
    mutex()->lock();

    if (! inited_)
    {
        mutex()->unlock();
        return;
    }

    seeking_finished_ = false;
    mutex()->unlock();
    while (! done_ && ! finish_ && ! derror_)
    {
        mutex()->lock();

        if (seek_time_ >= 0.0 && total_time_ > 0)
        {
            qDebug("Seek Time:%d", (int)seek_time_);
            long seek_pos = long(seek_time_ * input()->size() / total_time_);
            input()->seek(seek_pos);
            output_size_ = long(seek_time_) * long(freq_ * channels_ * 16 / 2);
            mad_frame_mute(&frame);
            mad_synth_mute(&synth);
            stream.error = MAD_ERROR_BUFLEN;
            stream.sync = 0;
            input_bytes_ = 0;
            output_at_ = 0;
            output_bytes_ = 0;
            stream.next_frame = 0;
            skip_frames = 2;
            eof_ = false;
            seek_time_ = -1;
            seeking_finished_ = true;
        }
        finish_ = eof_;

        if (! eof_)
        {
            if (stream.next_frame)
            {
                input_bytes_ = &input_buf_[input_bytes_] - (char *) stream.next_frame;
                memmove(input_buf_, stream.next_frame, input_bytes_);
            }

            if (stream.error == MAD_ERROR_BUFLEN)
            {
                int len_ = input()->read((char *) input_buf_ + input_bytes_,
                                        INPUT_BUFFER_SIZE - input_bytes_);

                if (len_ == 0)
                {
                    qDebug("DecoderMAD: end of file");
                    eof_ = true;
                }
                else if (len_ < 0)
                {
                    qWarning("DecoderMAD: %s", qPrintable(input()->errorString ()));
                    derror_ = true;
                    break;
                }

                input_bytes_ += len_;
            }

            mad_stream_buffer(&stream, (unsigned char *) input_buf_, input_bytes_);
        }

        mutex()->unlock();

        // decode
        while (!done_ && !finish_ && !derror_ && seek_time_ == -1.)
        {
            if (mad_frame_decode(&frame, &stream) == -1)
            {
                if (stream.error == MAD_ERROR_LOSTSYNC)
                {
                    //skip ID3v2 tag
                    uint tagSize = findID3v2((uchar *)stream.this_frame,
                                             (ulong) (stream.bufend - stream.this_frame));
                    if (tagSize > 0)
                    {
                        mad_stream_skip(&stream, tagSize);
                        qDebug("DecoderMAD: %d bytes skipped", tagSize);
                    }
                    continue;
                }

                if (stream.error == MAD_ERROR_BUFLEN)
                    break;

                if (stream.error == MAD_ERROR_BUFLEN)
                    continue;

                // error in decoding
                if (!MAD_RECOVERABLE(stream.error))
                {
                    derror_ = true;
                    break;
                }
                continue;
            }
            mutex()->lock();

            if (seek_time_ >= 0.)
            {
                mutex()->unlock();
                break;
            }

            if (skip_frames)
            {
                skip_frames-- ;
                mutex()->unlock();
                continue;
            }
            mad_synth_frame(&synth, &frame);
            madOutput();
            mutex()->unlock();
        }
    }

    mutex()->lock();

    if (!user_stop_ && eof_)
    {
        flush(TRUE);

        if (output())
        {
            output()->recycler()->mutex()->lock();
            // end of stream
            while (! output()->recycler()->empty() && ! user_stop_)
            {
                output()->recycler()->cond()->wakeOne();
                mutex()->unlock();
                output()->recycler()->cond()->wait(output()->recycler()->mutex());
                mutex()->lock();
            }
            output()->recycler()->mutex()->unlock();
        }

        done_ = TRUE;
        if (!user_stop_)
            finish_ = TRUE;
    }

    if (finish_)
        finish();

    mutex()->unlock();

    if (input())
        input()->close();
    deinit();
}

uint DecoderMAD::findID3v2(uchar *data, ulong size) //retuns ID3v2 tag size
{
    if (size < 10)
        return 0;

    if (((data[0] == 'I' && data[1] == 'D' && data[2] == '3') || //ID3v2 tag
            (data[0] == '3' && data[1] == 'D' && data[2] == 'I')) && //ID3v2 footer
            data[3] < 0xff && data[4] < 0xff && data[6] < 0x80 &&
            data[7] < 0x80 && data[8] < 0x80 && data[9] < 0x80)
    {
        TagLib::ByteVector byteVector((char *)data, size);
        TagLib::ID3v2::Header header(byteVector);
        return header.tagSize();
    }
    return 0;
}

static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static inline signed long fix_sample(unsigned int bits, mad_fixed_t sample)
{
    mad_fixed_t quantized, check;
    // clip
    quantized = sample;
    check = (sample >> MAD_F_FRACBITS) + 1;
    if (check & ~1)
    {
        if (sample >= MAD_F_ONE)
            quantized = MAD_F_ONE - 1;
        else if (sample < -MAD_F_ONE)
            quantized = -MAD_F_ONE;
    }
    // quantize
    quantized &= ~((1L << (MAD_F_FRACBITS + 1 - bits)) - 1);
    // scale
    return quantized >> (MAD_F_FRACBITS + 1 - bits);
}

enum mad_flow DecoderMAD::madOutput()
{
    unsigned int samples, channels_;
    mad_fixed_t const *left, *right;

    samples = synth.pcm.length;
    channels_ = synth.pcm.channels;
    left = synth.pcm.samples[0];
    right = synth.pcm.samples[1];


    bitrate_ = frame.header.bitrate / 1000;
    done_ = user_stop_;

    while (samples-- && !user_stop_)
    {
        signed int sample;

        if (output_bytes_ + 4096 > globalBufferSize)
        {
            flush();
        }

        //mad_fixed_t const *temp = left;
        sample = fix_sample(16, *left++);
        *(output_buf_ + output_at_++) = ((sample >> 0) & 0xff);
        *(output_buf_ + output_at_++) = ((sample >> 8) & 0xff);
        output_bytes_ += 2;

        // Note: The Dule-Channel must be supported on our device.
        if (channels_ == 2)
        {
            sample = fix_sample(16, *right++);
            *(output_buf_ + output_at_++) = ((sample >> 0) & 0xff);
            *(output_buf_ + output_at_++) = ((sample >> 8) & 0xff);
            output_bytes_ += 2;
        }
        /*else if (channels_ == 1)
        {
            sample = fix_sample(16, *temp);
            *(output_buf_ + output_at_++) = ((sample >> 0) & 0xff);
            *(output_buf_ + output_at_++) = ((sample >> 8) & 0xff);
            output_bytes_ += 2;
        }*/
    }

    if (done_ || finish_)
    {
        return MAD_FLOW_STOP;
    }

    return MAD_FLOW_CONTINUE;
}

enum mad_flow DecoderMAD::madError(struct mad_stream *stream,
                                   struct mad_frame *)
{
    if (MAD_RECOVERABLE(stream->error))
        return MAD_FLOW_CONTINUE;
    qFatal("MAderror_!\n");
    return MAD_FLOW_STOP;
}

}

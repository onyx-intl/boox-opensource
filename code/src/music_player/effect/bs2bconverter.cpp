#include <math.h>
#include <stdlib.h>
#include "bs2bconverter.h"

namespace player
{

Bs2bConverter::Bs2bConverter(QObject* parent)
    : Effect(parent)
{
    bs2b_handler_ = bs2b_open();
    //QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    //bs2b_set_level(bs2b_handler_, settings.value("bs2b/level", BS2B_DEFAULT_CLEVEL).toUInt());
    bs2b_set_level(bs2b_handler_, BS2B_DEFAULT_CLEVEL);
}

Bs2bConverter::~Bs2bConverter()
{
    bs2b_clear(bs2b_handler_);
}

#define CASE_BS2B(bitsPerSample, dataType, functionToCall, samples, out_data) \
    case bitsPerSample: \
        { \
            dataType * data = reinterpret_cast<dataType *>(*out_data); \
            functionToCall(bs2b_handler_, data, samples); \
        } \
        break;

ulong Bs2bConverter::process(char *in_data, const ulong size, char **out_data)
{
    memcpy(*out_data, in_data, size);
    if (channels() != 2)
    {
        return size;
    }

    uint samples = size / (bitsPerSample() / 8) / 2;
    mutex_.lock();
    switch (bitsPerSample())
    {
        CASE_BS2B(8,  int8_t,  bs2b_cross_feed_s8, samples, out_data)
        CASE_BS2B(16, int16_t, bs2b_cross_feed_s16le, samples, out_data)
        CASE_BS2B(24, bs2b_int24_t, bs2b_cross_feed_s24, samples, out_data)
        CASE_BS2B(32, int32_t,  bs2b_cross_feed_s32le, samples, out_data)
    default:
        ; // noop
    }
    mutex_.unlock();
    return size;
}

void Bs2bConverter::configure(quint32 freq, int chan, int res)
{
    Effect::configure(freq, chan, res);
    bs2b_set_srate(bs2b_handler_,freq);
}

void Bs2bConverter::setCrossfeedLevel(uint32_t level)
{
    mutex_.lock();
    bs2b_set_level(bs2b_handler_, level);
    mutex_.unlock();
}

}

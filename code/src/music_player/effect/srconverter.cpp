#include <math.h>
#include <stdlib.h>
#include "srconverter.h"

namespace player
{

SRConverter::SRConverter(QObject* parent) : Effect(parent)
{
    is_src_alloc_ = FALSE;
    int converter_type_array[] = {SRC_SINC_BEST_QUALITY, SRC_SINC_MEDIUM_QUALITY, SRC_SINC_FASTEST,
                                  SRC_ZERO_ORDER_HOLD,  SRC_LINEAR};
    src_in_ = 0;
    src_out_ = 0;
    src_state_ = 0;
    src_error_ = 0;
    over_sampling_fs_ = 44100;
    converter_type_ = converter_type_array[4];
}

SRConverter::~SRConverter()
{
    src_reset (src_state_) ;
    freeSRC();
    src_data_.data_in = 0;
    src_data_.data_out = 0;
    src_data_.end_of_input = 0;
    src_data_.input_frames = 0;
    src_data_.output_frames = 0;
    if (is_src_alloc_)
    {
        free(src_in_);
        free(src_out_);
        free(w_out_);
        is_src_alloc_ = FALSE;
    }
}

ulong SRConverter::process(char *in_data, const ulong size, char **out_data)
{
    if (is_src_alloc_)
    {
        free(src_in_);
        free(src_out_);
        free(w_out_);
        is_src_alloc_ = FALSE;
    }
    ulong wbytes = 0;

    if (src_state_ && size > 0)
    {
        int lrLength = size/2;
        int overLrLength= (int)floor(lrLength*(src_data_.src_ratio+1));
        src_in_ = (float*) malloc(sizeof(float)*lrLength);
        src_out_ = (float*) malloc(sizeof(float)*overLrLength);
        w_out_ = (short int*) malloc(sizeof(short int)*overLrLength);
        src_short_to_float_array((short int*)in_data, src_in_, lrLength);
        is_src_alloc_ = TRUE;
        src_data_.data_in = src_in_;
        src_data_.data_out = src_out_;
        src_data_.end_of_input = 0;
        src_data_.input_frames = lrLength/2;
        src_data_.output_frames = overLrLength/2;
        if ((src_error_ = src_process(src_state_, &src_data_)) > 0)
        {
            qWarning("SRConverter: src_process(): %s\n", src_strerror(src_error_));
        }
        else
        {
            src_float_to_short_array(src_out_, w_out_, src_data_.output_frames_gen*2);
            wbytes = src_data_.output_frames_gen*4;
            *out_data = new char[wbytes];
            memcpy(*out_data, (char*) w_out_, wbytes);
        }
    }
    return wbytes;
}

void SRConverter::configure(quint32 freq, int chan, int res)
{
    freeSRC();
    uint rate = freq;
    {
        src_state_ = src_new(converter_type_, 2, &src_error_);
        if (src_state_)
        {
            src_data_.src_ratio = (float)over_sampling_fs_/(float)rate;
            rate = over_sampling_fs_;
        }
        else
        {
            qDebug("SRConverter: src_new(): %s", src_strerror(src_error_));
        }
    }
    Effect::configure(over_sampling_fs_, chan, res);
}

void SRConverter::freeSRC()
{
    if (src_state_ != NULL)
    {
        src_state_ = src_delete(src_state_);
    }
}

}

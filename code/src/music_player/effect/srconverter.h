#ifndef PLAYER_SR_CONVERTER_H_
#define PLAYER_SR_CONVERTER_H_

#include <libsr/samplerate.h>
#include <core/effect.h>

namespace player
{

class SRConverter : public Effect
{
    Q_OBJECT
public:
    SRConverter(QObject *parent = 0);
    virtual ~SRConverter();

    ulong process(char *in_data, const ulong size, char **out_data);
    void configure(quint32 freq, int chan, int res);

private:
    void freeSRC();

private:
    SRC_STATE *src_state_;
    SRC_DATA  src_data_;
    quint32   over_sampling_fs_;
    int       src_error_;
    int       converter_type_;
    bool      is_src_alloc_;
    float     *src_in_;
    float     *src_out_;
    short     *w_out_;
    quint32   freq_;
};

};

#endif

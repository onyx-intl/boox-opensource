#ifndef PLAYER_BS2B_CONVERTER_H_
#define PLAYER_BS2B_CONVERTER_H_

#include <libbs2b/bs2b.h>
#include <core/effect.h>

namespace player
{

class Bs2bConverter : public Effect
{
    Q_OBJECT
public:
    Bs2bConverter(QObject *parent = 0);
    virtual ~Bs2bConverter();

    ulong process(char *in_data, const ulong size, char **out_data);
    void configure(quint32 freq, int chan, int res);
    void setCrossfeedLevel(uint32_t level);

private:
    t_bs2bdp bs2b_handler_;
    QMutex   mutex_;
};

};

#endif

#ifndef PLAYER_OUTPUTWAVEOUT_H_
#define PLAYER_OUTPUTWAVEOUT_H_

#include <core/output.h>

namespace player
{

class OutputWaveOut : public Output
{
    Q_OBJECT
public:
    OutputWaveOut(QObject * parent = 0);
    ~OutputWaveOut();

    bool initialize();
    void configure(quint32, int, int);
    qint64 latency();
    void enable(bool e);

private:
    //output api
    qint64 writeAudio(unsigned char *data, qint64 maxSize);
    void flush();

    // helper functions
    void status();
    void uninitialize();

private:
    bool enabled_;
};

};

#endif // PLAYER_OUTPUTWAVEOUT_H

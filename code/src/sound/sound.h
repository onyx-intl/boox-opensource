#ifndef ONYX_SOUND_H_
#define ONYX_SOUND_H_

#include <QByteArray>

/// Manipulate sound card for linux based system.
/// Helpful link: http://www.oreilly.de/catalog/multilinux/excerpt/ch14-05.htm
class Sound
{
public:
    Sound(const char *dev = "/dev/dsp");
    ~Sound();

public:
    int volume();
    bool setVolume(const int volume);

    /// Set sample size, usually it's 8 or 16.
    bool setBitsPerSample(int bps);

    /// set channels. Usually it's mono or stereo.
    bool setChannels(int channels);

    /// Set sample rate.
    bool setSamplingRate(int rate);

    bool setRec();

    /// Play the data. Before using this function, make sure
    /// all parameters of sound chip are correctly configured.
    bool play(const char *data, int size);

    /// Check if the device is enabled or not.
    inline bool isEnabled() { return enable_; }
    inline void enable(bool enable = true) { enable_ = enable; }

private:
    bool open(const char *device_name);
    void close();

    QByteArray toStereo(const char *src, int size, const int bps);

private:
    int device_;
    bool enable_;   ///< Soft flag to enable or disable the device.
    int bps_;
    int channels_;
};


#endif // NABOO_SOUND_H_

#ifndef PLAYER_VOLUMECONTROL_H_
#define PLAYER_VOLUMECONTROL_H_

#include <utils/player_utils.h>

namespace player
{

/// The VolumeControl class provides the base interface class for volume control.
class VolumeControl : public QObject
{
    Q_OBJECT
public:
    VolumeControl(QObject *parent = 0);
    ~VolumeControl();

    virtual void setVolume(int left, int right) = 0;
    int left();
    int right();

    static VolumeControl *create(QObject *parent = 0);

Q_SIGNALS:
    void volumeChanged(int left, int right);

public Q_SLOTS:
    void checkVolume();

protected:
    virtual void volume(int *left, int *right) = 0;

private:
    int  left_;
    int  right_;
    bool prev_block_;

};

/// The SoftwareVolume class provides access to the software volume control.
class SoftwareVolume : public VolumeControl
{
    Q_OBJECT
public:
    SoftwareVolume(QObject *parent = 0);
    ~SoftwareVolume();

    void setVolume(int left, int right);

    static SoftwareVolume *instance();
    static void setEnabled(bool b);

protected:
    void volume(int *left, int *right);

private:
    int left_;
    int right_;

private:
    static SoftwareVolume *instance_;
};

};

#endif

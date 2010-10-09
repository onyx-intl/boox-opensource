#include "output.h"
#include "volumecontrol.h"

namespace player
{

VolumeControl::VolumeControl(QObject *parent)
    : QObject(parent)
{
    left_ = 0;
    right_ = 0;
    prev_block_ = FALSE;
}

VolumeControl::~VolumeControl()
{
}

VolumeControl *VolumeControl::create(QObject *parent)
{
    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    if (settings.value("Volume/software_volume", FALSE).toBool())
    {
        return new SoftwareVolume(parent);
    }

    VolumeControl *control = 0;
    if (Output::currentFactory())
    {
        control = Output::currentFactory()->createVolumeControl(parent);
    }
    if (!control)
    {
        return new SoftwareVolume(parent);
    }
    QTimer *timer_ = new QTimer(control);
    connect(timer_, SIGNAL(timeout()), control, SLOT(checkVolume()));
    timer_->start(125);
    return control;
}

int VolumeControl::left()
{
    return left_;
}

int VolumeControl::right()
{
    return right_;
}

void VolumeControl::checkVolume()
{
    int l = 0, r = 0;
    volume(&l, &r);
    l = (l > 100) ? 100 : l;
    r = (r > 100) ? 100 : r;
    l = (l < 0) ? 0 : l;
    r = (r < 0) ? 0 : r;
    if (left_ != l || right_ != r)
    {
        //volume has been changed
        left_ = l;
        right_ = r;
        emit volumeChanged(left_, right_);
    }
    else if(prev_block_ && !signalsBlocked ())
    {
        //signals have been unblocked
        emit volumeChanged(left_, right_);
    }
    prev_block_ = signalsBlocked();
}

SoftwareVolume *SoftwareVolume::instance_ = 0;

SoftwareVolume::SoftwareVolume(QObject *parent)
 : VolumeControl(parent)
{
    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    left_ = settings.value("Volume/left", 80).toInt();
    right_ = settings.value("Volume/right", 80).toInt();
    blockSignals(TRUE);
    checkVolume();
    blockSignals(FALSE);
    QTimer::singleShot(125, this, SLOT(checkVolume()));
    instance_ = this;
}

SoftwareVolume::~SoftwareVolume()
{
    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    settings.setValue("Volume/left", left_);
    settings.setValue("Volume/right", right_);
    instance_ = 0;
}

void SoftwareVolume::setVolume(int left, int right)
{
    left_ = left;
    right_ = right;
    checkVolume();
}

void SoftwareVolume::volume(int *left, int *right)
{
    *left = left_;
    *right = right_;
}
//static
SoftwareVolume *SoftwareVolume::instance()
{
    return instance_;
}

void  SoftwareVolume::setEnabled(bool b)
{
    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    settings.setValue("Volume/software_volume", b);
}

}

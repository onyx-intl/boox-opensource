#include "soundcore.h"
#include "statehandler.h"


namespace player
{

#define TICK_INTERVAL 250

StateHandler* StateHandler::instance_ = 0;

StateHandler::StateHandler(QObject *parent)
    : QObject(parent)
{
    if (!instance_)
    {
        instance_ = this;
    }

    elapsed_ = -1;
    bitrate_ = 0;
    frequency_ = 0;
    precision_ = 0;
    channels_ = 0;
    send_meta_ = false;
    state_ = PlayerUtils::Stopped;
}


StateHandler::~StateHandler()
{
    if (instance_ == this)
    {
        instance_ = 0;
    }
}


void StateHandler::dispatch(qint64 elapsed,
                            qint64 totalTime,
                            int bitrate,
                            quint32 frequency,
                            int precision,
                            int channels)
{
    mutex_.lock();
    if (qAbs(elapsed_ - elapsed) > TICK_INTERVAL)
    {
        elapsed_ = elapsed;
        emit (elapsedChanged(elapsed));
        if (bitrate_ != bitrate)
        {
            bitrate_ = bitrate;
            emit (bitrateChanged(bitrate));
        }
    }
    if (frequency_ != frequency)
    {
        frequency_ = frequency;
        emit (frequencyChanged(frequency));
    }
    if (precision_ != precision)
    {
        precision_ = precision;
        emit (precisionChanged(precision));
    }
    if (channels_ != channels)
    {
        channels_ = channels;
        emit (channelsChanged(channels));
    }
    mutex_.unlock();
}

void StateHandler::dispatch(const QMap<PlayerUtils::MetaData, QString> &metaData)
{
    mutex_.lock();
    QMap<PlayerUtils::MetaData, QString> tmp = metaData;
    foreach(QString value, tmp.values()) //remove empty keys
    {
        if (value.isEmpty() || value == "0")
            tmp.remove(tmp.key(value));
    }
    tmp.insert(PlayerUtils::URL, SoundCore::instance()->url());
    if (metadata_ != tmp)
    {
        metadata_ = tmp;
        if (state_ == PlayerUtils::Playing) //send metadata in play state only
            emit metaDataChanged ();
        else
            send_meta_ = true;

    }
    mutex_.unlock();
}

void StateHandler::dispatch(const PlayerUtils::State &state)
{
    mutex_.lock();
    //clear
    QList<PlayerUtils::State> clearStates;
    clearStates << PlayerUtils::Stopped << PlayerUtils::NormalError << PlayerUtils::FatalError;
    if (clearStates.contains(state))
    {
        elapsed_ = -1;
        bitrate_ = 0;
        frequency_ = 0;
        precision_ = 0;
        channels_ = 0;
        send_meta_ = false;
        metadata_.clear();
    }
    if (state_ != state)
    {
        QStringList states;
        states << "Playing" << "Paused" << "Stopped" << "Buffering" << "NormalError" << "FatalError";
        qDebug("StateHandler: Current state: %s; previous state: %s",
               qPrintable(states.at(state)), qPrintable(states.at(state_)));
        state_ = state;

        emit stateChanged(state);
        if (state_ == PlayerUtils::Playing && send_meta_)
        {
            send_meta_ = false;
            emit metaDataChanged ();
        }
    }
    mutex_.unlock();
}

qint64 StateHandler::elapsed()
{
    return elapsed_;
}

int StateHandler::bitrate()
{
    return bitrate_;
}

int StateHandler::frequency()
{
    return frequency_;
}

int StateHandler::precision()
{
    return precision_;
}

int StateHandler::channels()
{
    return channels_;
}

PlayerUtils::State StateHandler::state() const
{
    return state_;
}

QMap<PlayerUtils::MetaData, QString> StateHandler::metaData()
{
    return metadata_;
}

QString StateHandler::metaData(PlayerUtils::MetaData key)
{
    return metadata_.value(key);
}

StateHandler *StateHandler::instance()
{
    return instance_;
}

}


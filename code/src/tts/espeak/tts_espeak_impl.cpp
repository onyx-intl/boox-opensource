
#include "tts_espeak.h"

namespace tts
{
static const int BPS = 16;
static const int CHANNELS = 2;
static const int FREQ = 44100;

/// Callback function.
int ESpeakImpl::synthCallback(short *wav, int numsamples, espeak_EVENT *events)
{
    char * d = reinterpret_cast<char *>(wav);
    ESpeakImpl *object = static_cast<ESpeakImpl *>(events->user_data);
    if (object)
    {
        // Convert mono to stereo. The espeak only generates mono data.
        for(int i = 0; i < numsamples; ++i)
        {
            QByteArray x(d, 2);
            object->data_.append(x);
            object->data_.append(x);
            d += 2;
        }
    }
    return 0;
}


ESpeakImpl::ESpeakImpl()
{
    initialized_ = false;
    stop_ = false;

    // Use bs2b library to convert between different sample rate.
    bs2b_handler_ = bs2b_open();
    bs2b_set_level(bs2b_handler_, BS2B_DEFAULT_CLEVEL);
    bs2b_set_srate(bs2b_handler_, FREQ);
}

ESpeakImpl::~ESpeakImpl()
{
    bs2b_clear(bs2b_handler_);

    destroy();
}

bool ESpeakImpl::initialize(const QLocale & locale, Sound & sound)
{
    sound.setBitsPerSample(BPS);
    sound.setChannels(CHANNELS);
    sound.setSamplingRate(FREQ);
    sound.setRec();

    return create(locale);
}

ulong ESpeakImpl::process(char *in_data, const ulong size, char **out_data)
{
    memcpy(*out_data, in_data, size);
    uint samples = size / (BPS / 8) / 2;
    int16_t * data = reinterpret_cast<int16_t *>(*out_data);
    bs2b_cross_feed_s16le(bs2b_handler_, data, samples);
    return size;
}

bool ESpeakImpl::synthText(const QString & text)
{
    stop_ = false;
    data_.clear();

    QByteArray d = text.toUtf8();
    int synth_flags = espeakPHONEMES | espeakENDPAUSE | espeakCHARS_UTF8;
    espeak_Synth(d.constData(), d.size() + 1, 0, POS_CHARACTER, 0, synth_flags, 0, this);

    emit synthDone(true, data_);
    return true;
}

bool ESpeakImpl::create(const QLocale & locale)
{
    if (initialized_)
    {
        return true;
    }

    QDir dir(QDir::home());
    QString path = SHARE_ROOT;
    if (!path.isEmpty())
    {
        dir.cd(path);
    }

    qDebug("Resource path %s", qPrintable(dir.absolutePath()));
    espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0, dir.absolutePath().toLocal8Bit().constData(), 0);

    // Basically, we don't need to the following settings now.
    espeak_SetParameter(espeakRATE, 130, 0);
    espeak_SetParameter(espeakVOLUME, 100, 0);
    espeak_SetParameter(espeakPITCH, 50, 0);
    espeak_SetParameter(espeakRANGE, 50, 0);
    espeak_SetParameter(espeakCAPITALS, 0,0);
    espeak_SetParameter(espeakPUNCTUATION, espeakPUNCT_SOME,0);
    espeak_SetParameter(espeakWORDGAP, 0,0);
    espeak_SetVoiceByName("+f1");

    /*
    espeak_SetParameter(espeakLINELENGTH,0,0);
    espeak_SetPunctuationList(option_punctlist);
    espeak_SetPhonemeTrace(option_phonemes,f_phonemes_out);
    */

    espeak_SetSynthCallback(synthCallback);
    initialized_ = true;
    return true;
}

bool ESpeakImpl::destroy()
{
    if (!initialized_)
    {
        return false;
    }

    espeak_Terminate();

    initialized_ = false;
    return true;
}

}

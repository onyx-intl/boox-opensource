
#include "tts_ej.h"

using namespace ui;

namespace tts
{


static QStringList cn_speakers;
static QStringList en_speakers;

/// Output callback. This function is called repeatedly during synth.
/// Once the data is ready, it can be sent to player.
jtErrCode EJSound::outputCallback(void* parameter,
                                  long outputFormat,
                                  void* data,
                                  long size)
{
#ifdef BUILD_FOR_ARM
    EJSound *object = reinterpret_cast<EJSound *>(parameter);
    if (object)
    {
        if (object->isStopped())
        {
            jtTTS_SynthStop(object->handle_);
            return jtTTS_ERR_NONE;
        }
        object->data_.append(reinterpret_cast<const char *>(data), size);
    }
#endif
    return jtTTS_ERR_NONE;    // to  continue.
}


EJSound::EJSound()
: handle_(0)
, heap_(0)
, current_style_(SPEAK_STYLE_CLEAR)
, current_speed_(2)
{
    initialized_ = false;
    stop_ = false;

    if (cn_speakers.isEmpty())
    {
        cn_speakers << "ZhangNan";
        cn_speakers << "Shuyi";
        cn_speakers << "BaiSong";
    }

    if (en_speakers.isEmpty())
    {
        en_speakers << "Cameal";
        en_speakers << "Barron";
    }
    current_speaker_ = en_speakers.front();
}

EJSound::~EJSound()
{
    destroy();
}

bool EJSound::initialize(const QLocale & locale, Sound & sound)
{
    qDebug("Create EJSound plugin now.");
    return create(locale);
}

bool EJSound::synthText(const QString & text)
{
    if (!isInitialized())
    {
        return false;
    }

    jtErrCode error = jtTTS_ERR_NONE;
#ifdef BUILD_FOR_ARM
    // The trial version can not synthesize text more than 1024 bytes?
    stop_ = false;
    data_.clear();
    QByteArray d = text.toUtf8();
    const int block = 1024;
    int count = d.size() / block;
    for(int i = 0; i < count; ++i)
    {
        error = jtTTS_SynthesizeText(handle_, d.constData() + i * block, block);
    }

    // final block
    if (count * block < d.size())
    {
        int pos = count * block;
        error = jtTTS_SynthesizeText(handle_, d.constData() + pos, d.size() - pos);
    }
#endif
    emit synthDone(true, data_);
    return (error == jtTTS_ERR_NONE);
}

bool EJSound::create(const QLocale & locale)
{
    if (initialized_)
    {
        qDebug("Already initialized.");
        return true;
    }

#ifdef BUILD_FOR_ARM
    jtErrCode   dwError;

    // Check version.
    unsigned char   byMajor;
    unsigned char   byMinor;
    unsigned long   iRevision;
    dwError = jtTTS_GetVersion(&byMajor, &byMinor, &iRevision);
    if(dwError != jtTTS_ERR_NONE)
    {
        return false;
    }

    long size = 0;
    std::string cn, en;
    if (!package("cn", current_speaker_, cn) ||
        !package("en", current_speaker_, en))
    {
        qDebug("Fatal error: could not load resource!");
        return false;
    }
    dwError = jtTTS_GetExtBufSize((const signed char *)cn.c_str(), (const signed char *)en.c_str(), NULL, &size);
    qDebug("size %d cn %s en %s", size, cn.c_str(), en.c_str());

    heap_ = (unsigned char *)malloc(size);
    memset(heap_, 0, size);

    dwError = jtTTS_Init((const signed char *)cn.c_str(), (const signed char *)en.c_str(), NULL, &handle_, heap_);
    if (dwError != jtTTS_ERR_NONE)
    {
        return false;
    }

    // Always use unicode.
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_CODEPAGE, jtTTS_CODEPAGE_UTF8);
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_INPUTTXT_MODE, jtTTS_INPUT_TEXT_DIRECT);
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_OUTPUT_CALLBACK, (long)outputCallback);
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_CALLBACK_USERDATA, (long)this);
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_VOLUME, jtTTS_VOLUME_NORMAL);
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_SPEAK_STYLE, toLibrary(current_style_));
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_SPEED, speed_map().value(current_speed_));
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_PITCH, jtTTS_PITCH_NORMAL);
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_WAV_FORMAT, jtTTS_FORMAT_PCM_44K16B);

#endif
    initialized_ = true;
    return true;
}

bool EJSound::destroy()
{
    if (!isInitialized())
    {
        return false;
    }

#ifdef BUILD_FOR_ARM
    jtTTS_End(handle_);
    handle_ = 0;
#endif

    if (heap_)
    {
        free(heap_);
        heap_ = 0;
    }

    // Close the resource file.
    file_.close();
    initialized_ = false;
    return true;
}

QString EJSound::prefix()
{
#ifdef BUILD_FOR_ARM
    return "/usr/share/tts/";
#else
    return "c:/onyx/sdk/data/tts/";
#endif
}

QString EJSound::packageName(const QString &language, const QString & speaker)
{
    QString s;
    if (language.compare("cn", Qt::CaseInsensitive) == 0)
    {
        if (cn_speakers.contains(speaker, Qt::CaseInsensitive))
        {
            s = speaker;
        }
        else
        {
            s = cn_speakers.front();
        }
    }
    else if (language.compare("en", Qt::CaseInsensitive) == 0)
    {
        if (en_speakers.contains(speaker, Qt::CaseInsensitive))
        {
            s = speaker;
        }
        else
        {
            s = en_speakers.front();
        }
    }

    if (!s.isEmpty())
    {
        return s.append(".dat");
    }
    return s;
}

bool EJSound::package(const QString &language, const QString & speaker, std::string & name)
{
    package_ = prefix() + packageName(language, speaker);
    if (QFile::exists(package_))
    {
        name = package_.toLocal8Bit().constData();
        return true;
    }
    return false;
}

QMap<int, int> & EJSound::speed_map()
{
    if (speed_map_.size() <= 0)
    {
        speed_map_[0] = -20000;
        speed_map_[1] = -10000;
        speed_map_[2] = 0;
        speed_map_[3] = 10000;
        speed_map_[4] = 20000;
    }
    return speed_map_;
}

SpeakStyle EJSound::toStandard(int i)
{
    switch (i)
    {
    case jtTTS_SPEAK_STYLE_CLEAR:
        return SPEAK_STYLE_CLEAR;
    case jtTTS_SPEAK_STYLE_NORMAL:
        return SPEAK_STYLE_NORMAL;
    case jtTTS_SPEAK_STYLE_PLAIN:
        return SPEAK_STYLE_PLAIN;
    case jtTTS_SPEAK_STYLE_VIVID:
        return SPEAK_STYLE_VIVID;
    }
    return SPEAK_STYLE_INVALID;
}

int EJSound::toLibrary(SpeakStyle s)
{
    switch (s)
    {
    case SPEAK_STYLE_CLEAR:
        return jtTTS_SPEAK_STYLE_CLEAR;
    case SPEAK_STYLE_NORMAL:
        return jtTTS_SPEAK_STYLE_NORMAL;
    case SPEAK_STYLE_PLAIN:
        return jtTTS_SPEAK_STYLE_PLAIN;
    case SPEAK_STYLE_VIVID:
        return jtTTS_SPEAK_STYLE_VIVID;
    default:
        return jtTTS_SPEAK_STYLE_VIVID;
    }
}

void EJSound::reload()
{
    destroy();
    create(QLocale());
}

bool EJSound::speakers(QStringList & list)
{
    list.clear();
    list << cn_speakers;
    list << en_speakers;
    return true;
}

bool EJSound::currentSpeaker(QString & speaker)
{
    speaker = current_speaker_;
    return true;
}

bool EJSound::setSpeaker(const QString & speaker)
{
    if (!isInitialized())
    {
        return false;
    }

    if (speaker == current_speaker_)
    {
        return false;
    }
    current_speaker_ = speaker;
    reload();
    return true;
}

bool EJSound::speeds(QVector<int> & list)
{
    list.clear();
    for(int i = 0; i < 5; ++i)
        list << i;
    return true;
}

bool EJSound::currentSpeed(int & speed)
{
    speed = current_speed_;
    return true;
}

bool EJSound::setSpeed(int speed_index)
{
    if (!isInitialized())
    {
        return false;
    }

    if (speed_index < 0 || speed_index >= speed_map_.size())
    {
        return false;
    }
#ifdef BUILD_FOR_ARM
    current_speed_ = speed_index;
    jtErrCode dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_SPEED, speed_map_[speed_index]);
#endif
    return true;
}

bool EJSound::styles(QVector<int> & styles)
{
    styles.clear();
    styles.push_back(SPEAK_STYLE_CLEAR);
    styles.push_back(SPEAK_STYLE_NORMAL);
    styles.push_back(SPEAK_STYLE_PLAIN);
    styles.push_back(SPEAK_STYLE_VIVID);
    return true;
}

bool EJSound::currentStyle(int & style)
{
    style = current_style_;
    return true;
}

bool EJSound::setStyle(int style)
{
    if (!isInitialized())
    {
        return false;
    }

    if (current_style_ == style)
    {
        return false;
    }
    current_style_ = static_cast<SpeakStyle>(style);
#ifdef BUILD_FOR_ARM
    jtErrCode   dwError;
    dwError = jtTTS_SetParam(handle_, jtTTS_PARAM_SPEAK_STYLE, toLibrary(current_style_));
#endif
    return true;
}

}

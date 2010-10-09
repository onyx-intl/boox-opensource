
#include "tts_aisound.h"

namespace tts
{

static const int HEAP_SIZE = 58000;     ///< With sound effect, otherwise use 38000
static const int ivTTS_CACHE_SIZE       = 512;
static const int ivTTS_CACHE_COUNT      = 1024;
static const int ivTTS_CACHE_EXT        = 8;

#ifdef BUILD_FOR_ARM

void ivCall AISound::readResouceCallback(ivPointer        pParameter,     /* [in] user callback parameter */
                                         ivPointer        pBuffer,        /* [out] read resource buffer */
                                         ivResAddress     iPos,           /* [in] read start position */
                                         ivResSize        nSize)          /* [in] read size */
{
    AISound *object = reinterpret_cast<AISound *>(pParameter);
    if (object)
    {
        object->file_.seek(iPos);
        int size = object->file_.read(reinterpret_cast<char *>(pBuffer), nSize);
    }
}

/// Output callback. This function is called repeatedly during synth.
/// Once the data is ready, it can be sent to player.
ivTTSErrID ivCall AISound::outputCallback(ivPointer     pParameter, /* [in] user callback parameter */
                                          ivUInt16      nCode,      /* [in] output data code */
                                          ivCPointer    pcData,     /* [in] output data buffer */
                                          ivSize        nSize )     /* [in] output data size */
{
    AISound *object = reinterpret_cast<AISound *>(pParameter);
    if (object)
    {
        if (object->isStopped())
        {
            return ivTTS_ERR_EXIT;  // stop.
        }
        object->data_.append(reinterpret_cast<const char *>(pcData), nSize);
    }
    return ivTTS_ERR_OK;    // to  continue.
}

#endif

AISound::AISound()
#ifdef BUILD_FOR_ARM
: handle_(0)
, heap_(0)
#endif
{
    initialized_ = false;
    stop_ = false;
}

AISound::~AISound()
{
    destroy();
}

bool AISound::initialize(const QLocale & locale, Sound & sound)
{
    qDebug("Create aisound plugin now.");
    return create(locale);
}

bool AISound::synthText(const QString & text)
{
    stop_ = false;
    data_.clear();
#ifdef BUILD_FOR_ARM
    // Make sure the tts has been correctly initialized.
    ivTTSErrID      ivReturn = ivTTS_ERR_OK;
    ivReturn = ivTTS_SynthText(handle_, ivText(text.toUtf8().data()), -1);
#endif
    emit synthDone(true, data_);
    return true;
}

bool AISound::create(const QLocale & locale)
{
    if (initialized_)
    {
        qDebug("Already initialized.");
        return true;
    }

#ifdef BUILD_FOR_ARM
    // Check resource.
    QDir dir(QDir::home());
    QString path = SHARE_ROOT;
    if (!path.isEmpty())
    {
        dir.cd(path);
    }

    if (!dir.cd("tts"))
    {
        qDebug("Could not change to tts folder");
        return false;
    }

    file_.setFileName(dir.absoluteFilePath("resource.irf"));
    if (!file_.open(QIODevice::ReadOnly))
    {
        qWarning("Can not load resource.irf file.");
        return false;
    }
    qDebug("load resource file done.");

    // Allocate memory.
    if (heap_ == 0)
    {
        heap_ = new unsigned char[HEAP_SIZE];
        memset(heap_, 0, HEAP_SIZE);
    }

    ivTTSErrID      ivReturn = ivTTS_ERR_OK;
    resource_desc_.pCBParam = this;
    resource_desc_.pfnRead = readResouceCallback;
    resource_desc_.pfnMap = NULL;
    resource_desc_.nSize = 0;
    resource_desc_.pCacheBlockIndex = (ivPUInt8)malloc(ivTTS_CACHE_COUNT + ivTTS_CACHE_EXT);
    resource_desc_.pCacheBuffer = (ivPUInt8)malloc((ivTTS_CACHE_COUNT + ivTTS_CACHE_EXT)*(ivTTS_CACHE_SIZE));
    resource_desc_.nCacheBlockSize = ivTTS_CACHE_SIZE;
    resource_desc_.nCacheBlockCount = ivTTS_CACHE_COUNT;
    resource_desc_.nCacheBlockExt = ivTTS_CACHE_EXT;

    // create tts engine instance.
    ivReturn = ivTTS_Create(&handle_, (ivPointer)heap_, HEAP_SIZE, this, &resource_desc_, 1);

    // callback function for output.
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)outputCallback);

    // code page
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_UTF8);

    // Language
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_CHINESE);

    // Volume
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_MAX);

    // speaker
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_ROLE, ivTTS_ROLE_ANITA);

    // speed
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_VOICE_SPEED, ivTTS_SPEED_NORMAL);

    // pitch
    ivReturn = ivTTS_SetParam(handle_, ivTTS_PARAM_VOICE_PITCH, ivTTS_PITCH_NORMAL);

    // Style
    ivReturn = ivTTS_SetParam(handle_,  ivTTS_PARAM_SPEAK_STYLE, ivTTS_STYLE_PLAIN);

#endif

    initialized_ = true;
    return true;
}

bool AISound::destroy()
{
    if (!initialized_)
    {
        return false;
    }

#ifdef BUILD_FOR_ARM

    ivTTS_Destroy(handle_);
    handle_ = 0;

    if ( resource_desc_.pCacheBlockIndex )
    {
        free(resource_desc_.pCacheBlockIndex);
    }
    if ( resource_desc_.pCacheBuffer )
    {
        free(resource_desc_.pCacheBuffer);
    }

    if ( heap_ )
    {
        delete [] heap_;
        heap_ = 0;
    }

    // Close the resource file.
    file_.close();

#endif

    initialized_ = false;
    return true;
}

}

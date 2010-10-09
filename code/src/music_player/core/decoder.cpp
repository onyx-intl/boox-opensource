#include "effect.h"
#include "effectfactory.h"

// create the mad docoder factory now
#include <input/decodermadfactory.h>

#include "constants.h"
#include "buffer.h"
#include "output.h"
#include "decoderfactory.h"
#include "streamreader.h"
#include "fileinfo.h"

extern "C"
{
#include "equ/iir.h"
}
#include "decoder.h"

namespace player
{

Decoder::Decoder(QObject *parent, DecoderFactory *d, QIODevice *i, Output *o)
    : QThread(parent)
    , seek_time_(-1)
    , seeking_finished_(false)
    , factory_(d)
    , input_(i)
    , output_(o)
    , eq_inited_(false)
    , use_eq_(false)
{
    init();
}

Decoder::Decoder(QObject *parent, DecoderFactory *d, Output *o)
    : QThread(parent)
    , seek_time_(-1)
    , seeking_finished_(false)
    , factory_(d)
    , input_(0)
    , output_(o)
    , eq_inited_(false)
    , use_eq_(false)
{
    init();
}

Decoder::~Decoder()
{
    factory_    = 0;
    input_      = 0;
    output_     = 0;
    block_size_ = 0;
}

void Decoder::init()
{
    if (output_)
    {
        output_->recycler()->clear();
    }

    double b[] = {0,0,0,0,0,0,0,0,0,0};
    setEQ(b, 0);
    qRegisterMetaType<PlayerUtils::State>("PlayerUtils::State");
    block_size_ = Buffer::size();
    effects_    = Effect::create(this);
    handler_    = 0;
}

DecoderFactory *Decoder::factory() const
{
    return factory_;
}

QIODevice *Decoder::input()
{
    return input_;
}

Output *Decoder::output()
{
    return output_;
}

QMutex *Decoder::mutex()
{
    return &mutex_;
}

QWaitCondition *Decoder::cond()
{
    return &wait_condition_;
}

StateHandler *Decoder::stateHandler()
{
    return handler_;
}

void Decoder::setStateHandler(StateHandler *handler)
{
    handler_ = handler;
}

void Decoder::setEQ(double bands[10], double preamp)
{
    set_preamp(0, 1.0 + 0.0932471 * preamp + 0.00279033 * preamp * preamp);
    set_preamp(1, 1.0 + 0.0932471 * preamp + 0.00279033 * preamp * preamp);
    for (int i=0; i<10; ++i)
    {
        double value = bands[i];
        set_gain(i, 0, 0.03 * value + 0.000999999 * value * value);
        set_gain(i, 1, 0.03 * value + 0.000999999 * value * value);
    }
}

void Decoder::setEQEnabled(bool on)
{
    use_eq_ = on;
}

void Decoder::seek(qint64 pos)
{
    seek_time_ = pos;
    seeking_finished_ = false;
}

void Decoder::configure(quint32 srate, int chan, int bps)
{
    Effect* effect = 0;
    foreach(effect, effects_)
    {
        effect->configure(srate, chan, bps);
        srate = effect->sampleRate();
        chan  = effect->channels();
        bps   = effect->bitsPerSample();
    }

    if (output_)
    {
        output_->configure(srate, chan, bps);
    }
}

qint64 Decoder::produceSound(char *data, qint64 size, quint32 brate, int chan)
{
    ulong sz = size < block_size_ ? size : block_size_;
    if (use_eq_)
    {
        if (!eq_inited_)
        {
            init_iir();
            eq_inited_ = true;
        }
        iir((void*) data, sz, chan);
    }
    char *out_data = data;
    char *prev_data = data;
    qint64 w = sz;
    Effect* effect = 0;
    foreach(effect, effects_)
    {
        w = effect->process(prev_data, sz, &out_data);
        if (w <= 0)
        {
            // copy data if plugin can not process it
            w = sz;
            out_data = new char[w];
            memcpy(out_data, prev_data, w);
        }
        if (data != prev_data)
        {
            delete prev_data;
        }
        prev_data = out_data;
    }

    Buffer *b = output()->recycler()->get(w);
    memcpy(b->data, out_data, w);
    if (w < block_size_ + b->exceeding)
    {
        memset(b->data + w, 0, block_size_ + b->exceeding - w);
    }
    b->nbytes = w;
    b->rate = brate;
    b->seeking_finished = seeking_finished_;
    if (seeking_finished_)
    {
        seeking_finished_ = false;
    }

    output()->recycler()->add();

    if (data != out_data)
    {
        delete out_data;
    }

    size -= sz;
    memmove(data, data + sz, size);
    return sz;
}

void Decoder::finish()
{
    if (output())
    {
        output()->mutex()->lock ();
        output()->finish();
        output()->mutex()->unlock();
        /*output()->recycler()->mutex()->lock ();
        output()->recycler()->cond()->wakeAll();
        output()->recycler()->mutex()->unlock();
        output()->wait();*/
    }
    emit playbackFinished();
}

// static methods
QList<DecoderFactory*> *Decoder::factories_ = 0;
DecoderFactory *Decoder::last_factory_ = 0;
QStringList Decoder::files_;

void Decoder::checkFactories()
{
    // TODO. Do NOT use the plugin now.
    // Create the MAD decoder factory directly
    if (!factories_)
    {
        files_.clear();
        factories_ = new QList<DecoderFactory *>;

        DecoderFactory *factory = new DecoderMADFactory();
        if (factory)
        {
            factories_->append(factory);
            qApp->installTranslator(factory->createTranslator(qApp));
        }
    }
}

QStringList Decoder::files()
{
    checkFactories();
    return files_;
}

bool Decoder::supports(const QString &source)
{
    checkFactories();

    DecoderFactory *fact;
    foreach(fact, *factories_)
    {
        if (fact->supports(source) && isEnabled(fact))
        {
            return true;
        }
    }
    return false;
}

DecoderFactory *Decoder::findByPath(const QString& source)
{
    checkFactories();
    DecoderFactory *fact = last_factory_;
    if (fact && fact->supports(source) && isEnabled(fact)) //try last factory
    {
        return fact;
    }

    foreach(fact, *factories_)
    {
        if (fact->supports(source) && isEnabled(fact))
        {
            last_factory_ = fact;
            return fact;
        }
    }
    return 0;
}

DecoderFactory *Decoder::findByMime(const QString& type)
{
    checkFactories();
    DecoderFactory *fact;
    foreach(fact, *factories_)
    {
        if (isEnabled(fact))
        {
            QStringList types = fact->properties().content_type_.split(";");
            for (int j=0; j < types.size(); ++j)
            {
                if (type == types[j] && !types[j].isEmpty())
                {
                    return fact;
                }
            }
        }
    }
    qDebug("Decoder: unable to find factory by mime");
    return 0;
}

DecoderFactory *Decoder::findByContent(QIODevice *input)
{
    checkFactories();
    foreach(DecoderFactory *fact, *factories_)
    {
        if (fact->canDecode(input) && isEnabled(fact))
        {
            return fact;
        }
    }
    qDebug("Decoder: unable to find factory by content");
    return 0;
}

DecoderFactory *Decoder::findByURL(const QUrl &url)
{
    checkFactories();
    foreach(DecoderFactory *fact, *factories_)
    {
        if (fact->supports(url.path()) && isEnabled(fact) &&
            fact->properties().protocols_.split(" ").contains(url.scheme()))
        {
            return fact;
        }
    }
    qDebug("Decoder: unable to find factory by url");
    return 0;
}

void Decoder::setEnabled(DecoderFactory* factory, bool enable)
{
    //checkFactories();
    //if (!factories_->contains(factory))
    //{
    //    return;
    //}

    //QString name = factory->properties().shortName;
    //QSettings settings ( PlayerUtils::configFile(), QSettings::IniFormat );
    //QStringList disabledList = settings.value("Decoder/disabled_plugins").toStringList();

    //if (enable)
    //    disabledList.removeAll(name);
    //else
    //{
    //    if (!disabledList.contains(name))
    //        disabledList << name;
    //}
    //settings.setValue("Decoder/disabled_plugins", disabledList);

    // TODO. Implement this function by current configure mechanism
}

bool Decoder::isEnabled(DecoderFactory* factory)
{
    checkFactories();
    if (!factories_->contains(factory))
    {
        return false;
    }

    //QString name = factory->properties().shortName;
    //QSettings settings ( PlayerUtils::configFile(), QSettings::IniFormat );
    //QStringList disabledList = settings.value("Decoder/disabled_plugins").toStringList();
    //return !disabledList.contains(name);

    // TODO. Implement this function by current configure mechanism
    return true;
}

bool Decoder::createPlayList(const QString &file_name,
                             QList<FileInfo *> &results,
                             bool use_metadata)
{
    results.clear();
    DecoderFactory *fact = 0;

    if (QFile::exists(file_name))
    {
        //is it file?
        fact = Decoder::findByPath(file_name);
    }
    else if (file_name.contains("://"))
    {
        //looks like url
        fact = Decoder::findByURL(QUrl(file_name));
    }

    if (fact)
    {
        fact->createPlayList(file_name, use_metadata, results);
    }
    else if (QUrl(file_name).scheme() == "http")
    {
        //create empty FileInfo for stream TODO transports support
        results << new FileInfo(file_name);
    }

    //append path if it is empty
    foreach(FileInfo *info, results)
    {
        if (info->path().isEmpty())
        {
            info->setPath(file_name);
        }
    }
    return true;
}

QStringList Decoder::filters()
{
    checkFactories();
    QStringList filters;
    foreach(DecoderFactory *fact, *factories_)
    {
        if (isEnabled(fact) && !fact->properties().filter_.isEmpty())
        {
            filters << fact->properties().description_ + " (" + fact->properties().filter_ + ")";
        }
    }
    return filters;
}

QStringList Decoder::nameFilters()
{
    checkFactories();
    QStringList filters;
    for (int i=0; i<factories_->size(); ++i)
    {
        if (isEnabled(factories_->at(i)))
        {
            filters << factories_->at(i)->properties().filter_.split(" ", QString::SkipEmptyParts);
        }
    }
    return filters;
}

QList<DecoderFactory*> *Decoder::factories()
{
    checkFactories();
    return factories_;
}

}

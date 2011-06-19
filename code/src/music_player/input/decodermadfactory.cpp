#include <mad.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/apetag.h>
#include <taglib/tfile.h>
#include <taglib/mpegfile.h>

#include <core/fileinfo.h>

#include "decoder_mad.h"
#include "decodermadfactory.h"

namespace player
{
bool isRussianCode(const std::string & strIn)
{
    unsigned char ch1;
    unsigned char ch2;
    int num = 0;
    if (strIn.size() >= 2)
    {
        int i;
        for (i = 0; i < strIn.size(); ++i)
        {
            ch1 = (unsigned char)strIn.at(i);

            if (ch1 >= 0x80)
            {
                num ++;
            }

        }
    }
    return  num > strIn.size() * 0.7;
} 

bool isGB2312Code(const std::string & strIn)
{
    unsigned char ch1;
    unsigned char ch2;
    int num = 0;
    if (strIn.size() >= 2)
    {
        int i;
        for (i = 0; i < strIn.size(); ++i)
        {
            ch1 = (unsigned char)strIn.at(i);
            if (ch1 < 0x7f)
            {
                num ++;
                continue;
            }

            if (ch1 > 0xd7)
            {
                return false;
            }

            if (ch1 >= 0xB0 && ch1 <= 0xd7 && i < strIn.size() - 1)
            {
                ch2 = (unsigned char)strIn.at(++i);
                if (ch2 > 0xFe)
                {
                    return false;
                }
                if (ch2 >= 0xa0 && ch2 <= 0xfe)
                {
                    num += 2;
                }
            }

        }
    }
    
    return num > strIn.size() * 0.7;
}

DecoderMADFactory::DecoderMADFactory()
{
}

DecoderMADFactory::~DecoderMADFactory()
{
}

bool DecoderMADFactory::supports(const QString &source) const
{
    QString ext = source.right(4).toLower();
    if (ext == ".mp1" || ext == ".mp2" || ext == ".mp3")
        return TRUE;
    else if (ext == ".wav") //check for mp3 wav files
    {
        QFile file(source);
        file.open(QIODevice::ReadOnly);
        char buf[22];
        file.peek(buf,sizeof(buf));
        file.close();
        if (!memcmp(buf + 8, "WAVE", 4) && !memcmp(buf + 20, "U" ,1))
        {
            return TRUE;
        }
    }
    return FALSE;
}

bool DecoderMADFactory::canDecode(QIODevice *input) const
{
    char buf[16 * 512];

    if (input->peek(buf,sizeof(buf)) == sizeof(buf))
    {
        struct mad_stream stream;
        struct mad_header header;
        int dec_res;

        mad_stream_init (&stream);
        mad_header_init (&header);
        mad_stream_buffer (&stream, (unsigned char *) buf, sizeof(buf));
        stream.error = MAD_ERROR_NONE;

        while ((dec_res = mad_header_decode(&header, &stream)) == -1
                && MAD_RECOVERABLE(stream.error))
            ;
        return dec_res != -1 ? TRUE: FALSE;
    }
    return FALSE;
}

const DecoderProperties DecoderMADFactory::properties() const
{
    DecoderProperties properties;
    properties.name_ = tr("MPEG Plugin");
    properties.short_name_ = "mad";
    properties.filter_ = "*.mp1 *.mp2 *.mp3 *.wav";
    properties.description_ = tr("MPEG Files");
    properties.content_type_ = "audio/mp3;audio/mpeg";
    properties.has_about_ = TRUE;
    properties.has_settings_ = TRUE;
    return properties;
}

Decoder *DecoderMADFactory::create(QObject *parent, QIODevice *input, Output *output, const QString &)
{
    return new DecoderMAD(parent, this, input, output);
}

//FileInfo *DecoderMADFactory::createFileInfo(const QString &source)
bool DecoderMADFactory::createPlayList(const QString &file_name,
                                       bool use_metadta,
                                       QList<FileInfo *> &results)
{
    FileInfo *info = new FileInfo(file_name);
    TagLib::Tag *tag = 0;
    TagLib::MPEG::File fileRef(file_name.toLocal8Bit ().constData());
    results.clear();

    if (use_metadta)
    {
        QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
        settings.beginGroup("MAD");

        QTextCodec *codec = 0;

        uint tag_array[3];
        tag_array[0] = settings.value("tag_1", ID3v2).toInt();
        tag_array[1] = settings.value("tag_2", Disabled).toInt();
        tag_array[2] = settings.value("tag_3", Disabled).toInt();

        for (int i = 0; i < 3; ++i)
        {
            switch ((uint) tag_array[i])
            {
            case ID3v1:
            {
                codec = QTextCodec::codecForName(settings.value("ID3v1_encoding","ISO-8859-1").toByteArray ());
                tag = fileRef.ID3v1Tag();
                break;
            }
            case ID3v2:
            {
                QByteArray name;
                name = settings.value("ID3v2_encoding","UTF-8").toByteArray ();
                if (name.contains("UTF"))
                    codec = QTextCodec::codecForName ("UTF-8");
                else
                    codec = QTextCodec::codecForName(name);
                //codec =QTextCodec::codecForLocale();
                tag = fileRef.ID3v2Tag();
                break;
            }
            case APE:
            {
                codec = QTextCodec::codecForName ("UTF-8");
                tag = fileRef.APETag();
                break;
            }
            case Disabled:
            {
                break;
            }
            }
            if (tag && !tag->isEmpty())
                break;
        }
        settings.endGroup();

        if (!codec)
            codec = QTextCodec::codecForName ("UTF-8");

        if (tag && codec)
        {
            TagLib::String album = tag->album();
            TagLib::String artist = tag->artist();
            TagLib::String comment = tag->comment();
            TagLib::String genre = tag->genre();
            TagLib::String title = tag->title();

            std::string str = tag->album().to8Bit(false);

            if(isGB2312Code(str))
            {
                codec = QTextCodec::codecForName ("gb2312");
            }
            else if (isRussianCode(str))
            {
                codec = QTextCodec::codecForName ("windows-1251");
            }
            else
            {
                codec = QTextCodec::codecForName ("UTF-8");
            }
            bool utf = codec->name ().contains("UTF");
            qDebug() << "codec->name " << codec->name ();

            info->setMetaData(PlayerUtils::ALBUM,
                              codec->toUnicode(album.toCString(utf)).trimmed());

            str =  tag->artist().to8Bit(false);
            if(isGB2312Code(str))
            {
                codec = QTextCodec::codecForName ("gb2312");
            }
            else if (isRussianCode(str))
            {
                codec = QTextCodec::codecForName ("windows-1251");
            }
            else
            {
                codec = QTextCodec::codecForName ("UTF-8");
            }
            utf = codec->name ().contains("UTF");
            qDebug() << "codec->name " << codec->name ();

            info->setMetaData(PlayerUtils::ARTIST,
                              codec->toUnicode(artist.toCString(utf)).trimmed());
            info->setMetaData(PlayerUtils::COMMENT,
                              codec->toUnicode(comment.toCString(utf)).trimmed());
            info->setMetaData(PlayerUtils::GENRE,
                              codec->toUnicode(genre.toCString(utf)).trimmed());

            str =  tag->title().to8Bit(false);
            if(isGB2312Code(str))
            {
                codec = QTextCodec::codecForName ("gb2312");
            }
            else if (isRussianCode(str))
            {
                codec = QTextCodec::codecForName ("windows-1251");
            }
            else
            {
                codec = QTextCodec::codecForName ("UTF-8");
            }
            utf = codec->name ().contains("UTF");
            qDebug() << "codec->name " << codec->name ();

            info->setMetaData(PlayerUtils::TITLE,
                              codec->toUnicode(title.toCString(utf)).trimmed());
            info->setMetaData(PlayerUtils::YEAR,
                              tag->year());
            info->setMetaData(PlayerUtils::TRACK,
                              tag->track());
        }
    }
    if (fileRef.audioProperties())
    {
        info->setLength(fileRef.audioProperties()->length());
    }
    results << info;
    return true;
}

QTranslator *DecoderMADFactory::createTranslator(QObject *parent)
{
    QTranslator *translator = new QTranslator(parent);
    QString locale = PlayerUtils::systemLanguageID();
    translator->load(QString(":/mad_plugin_") + locale);
    return translator;
}

QObject* DecoderMADFactory::showDetails(QWidget *parent,
                                        const QString &path)
{
    return 0;
}

void DecoderMADFactory::showSettings(QWidget *parent)
{
}

void DecoderMADFactory::showAbout(QWidget *parent)
{
}

}

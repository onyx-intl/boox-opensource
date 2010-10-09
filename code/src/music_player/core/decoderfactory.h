#ifndef PLAYER_DECODERFACTORY_H_
#define PLAYER_DECODERFACTORY_H_

#include <utils/player_utils.h>

namespace player
{

class Decoder;
class Output;
class FileInfo;

class DecoderProperties
{
public:
    DecoderProperties()
    {
        has_about_    = FALSE;
        has_settings_ = FALSE;
        no_input_     = FALSE;
        no_output_    = FALSE;
    }

    QString name_;         /*!< Input plugin full name */
    QString short_name_;   /*!< Input plugin short name for internal usage */
    QString filter_;       /*!< File filter (example: "*.mp3 *.ogg") */
    QString description_;  /*!< File filter description */
    QString content_type_; /*!< Supported content types */
    QString protocols_;    /*!< Supported protocols. Should be empty if plugin uses stream input. */
    bool    has_about_;    /*!< Should be \b true if plugin has about dialog, otherwise returns \b false */
    bool    has_settings_; /*!< Should be \b true if plugin has settings dialog, otherwise returns \b false */
    bool    no_input_;     /*!< Should be \b true if plugin has own input, otherwise returns \b false */
    bool    no_output_;    /*!< Should be \b true if plugin has own output, otherwise returns \b false */
};

/// Input plugin interface (decoder factory).
class DecoderFactory
{
public:
    virtual ~DecoderFactory() {}
    virtual bool supports(const QString &source) const = 0;
    virtual bool canDecode(QIODevice *d) const = 0;
    virtual const DecoderProperties properties() const = 0;
    virtual Decoder *create(QObject *parent,
                            QIODevice *input = 0,
                            Output *output = 0,
                            const QString &path = QString()) = 0;
    virtual bool createPlayList(const QString &file_name,
                                bool use_metadta,
                                QList<FileInfo *> &results) = 0;
    virtual QObject* showDetails(QWidget *parent,
                                 const QString &path) = 0;
    virtual void showSettings(QWidget *parent) = 0;
    virtual void showAbout(QWidget *parent) = 0;
    virtual QTranslator *createTranslator(QObject *parent) = 0;
};

};

#endif

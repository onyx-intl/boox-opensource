#ifndef PLAYER_DECODERMADFACTORY_H_
#define PLAYER_DECODERMADFACTORY_H_

#include <core/decoder.h>
#include <core/output.h>
#include <core/decoderfactory.h>

namespace player
{

class DecoderMADFactory : public QObject,
                          public DecoderFactory
{
    Q_OBJECT
public:
    DecoderMADFactory();
    virtual ~DecoderMADFactory();

    bool supports(const QString &source) const;
    bool canDecode(QIODevice *input) const;
    const DecoderProperties properties() const;
    Decoder *create(QObject *, QIODevice *, Output *, const QString &);
    //FileInfo *createFileInfo(const QString &source);
    QObject* showDetails(QWidget *parent,
                         const QString &path);
    void showSettings(QWidget *parent);
    void showAbout(QWidget *parent);

    bool createPlayList(const QString &file_name,
                        bool use_metadta,
                        QList<FileInfo *> &results);
    QTranslator *createTranslator(QObject *parent);
private:
    enum TagType {ID3v1 = 0, ID3v2, APE, Disabled};
};

};

#endif

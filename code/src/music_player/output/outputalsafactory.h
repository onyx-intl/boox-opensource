#ifndef PLAYER_OUTPUT_ALSA_FACTORY_H_
#define PLAYER_OUTPUT_ALSA_FACTORY_H_

#include <core/output.h>
#include <core/outputfactory.h>

namespace player
{

class OutputAlsaFactory : public QObject,
                          public OutputFactory
{
    Q_OBJECT
public:
    OutputAlsaFactory();
    virtual ~OutputAlsaFactory();

    const OutputProperties properties() const;
    Output* create(QObject* parent);
    VolumeControl *createVolumeControl(QObject *parent);
    void showSettings(QWidget* parent);
    void showAbout(QWidget *parent);
    QTranslator *createTranslator(QObject *parent);

};

};

#endif

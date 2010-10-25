#ifndef PLAYER_OUTPUTPULSEAUDIOFACTORY_H_
#define PLAYER_OUTPUTPULSEAUDIOFACTORY_H_

#include <core/output.h>
#include <core/outputfactory.h>

namespace player
{

class OutputWaveOutFactory : public QObject,
                             public OutputFactory
{
    Q_OBJECT
public:
    OutputWaveOutFactory();
    virtual ~OutputWaveOutFactory();

    const OutputProperties properties() const;
    Output* create(QObject* parent);
    VolumeControl *createVolumeControl(QObject *parent);
    void showSettings(QWidget* parent);
    void showAbout(QWidget *parent);
    QTranslator *createTranslator(QObject *parent);

};

};

#endif

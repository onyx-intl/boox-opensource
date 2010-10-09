#ifndef DICTIONARY_PLUGIN_H_
#define DICTIONARY_PLUGIN_H_

#include <QtCore/QtCore>

struct DictionaryInfo
{
    QString name;
    QString author;
    QString description;
};

/// Represent a single dictionary.
class Dictionary
{
public:
    Dictionary(){}
    virtual ~Dictionary(){}

public:
    virtual bool info(DictionaryInfo& info) = 0;

    virtual bool load(const QString & working_directory) = 0;
    virtual bool isLoaded() = 0;
    virtual bool close() = 0;

    virtual bool translate(const QString &word, QString& result) = 0;
    virtual bool similarWords(const QString &word, QStringList & result, const int offset, const int count) = 0;
};
typedef Dictionary * DictionaryPtr;


/// Represent a kind of dictionary plugin. For example, the plugin
/// using star dictionary backend or other backend.
class DictionaryPlugin : public QObject
{
    Q_OBJECT
public:
    DictionaryPlugin(const QStringList &directories){}
    virtual ~DictionaryPlugin(){}

public:

    /// This method returns all available dictionaries.
    /// The dictionary is not necessary loaded at this moment.
    virtual bool dictionaries(QStringList & list) = 0;

    /// Load the specified dictionary.
    virtual bool load(const QString & name) = 0;

    /// Load all dictionaries.
    virtual int loadAll() = 0;

    /// Get the dictionary specified by the name.
    virtual Dictionary * dictionary(const QString & name) = 0;

};
typedef DictionaryPlugin * PluginPtr;

#endif // DICTIONARY_PLUGIN_H_

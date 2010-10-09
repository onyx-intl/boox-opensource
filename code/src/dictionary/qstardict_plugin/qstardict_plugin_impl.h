
#ifndef QSTARDICT_PLUGIN_IMPL_H_
#define QSTARDICT_PLUGIN_IMPL_H_

#include "../dictionary_plugin.h"
#include "stardict_backend.h"
#include <QVector>

namespace stardict
{

/// Implement dictionary with qstardict backend.
class StarDictionaryImpl : public Dictionary
{
public:
    StarDictionaryImpl();
    ~StarDictionaryImpl();

public:
    virtual bool info(DictionaryInfo& info);

    virtual bool load(const QString & working_directory);
    virtual bool isLoaded();
    virtual bool close();

    virtual bool translate(const QString &word, QString& result);
    virtual bool similarWords(const QString &word, QStringList & result, const int offset, const int count);

private:
    bool find(const QString & word, long & index);
    bool fuzzyFind(const QString & word, long & index);

private:
    Dict dict_impl_;
    bool is_loaded_;
};
typedef StarDictionaryImpl * DictionaryImplPtr;


/// Represent a kind of dictionary plugin. For example, the plugin
/// using star dictionary backend or other backend.
class StarDictionaryPluginImpl : public DictionaryPlugin
{
    Q_OBJECT
public:
    StarDictionaryPluginImpl(const QStringList &directories);
    ~StarDictionaryPluginImpl();

public:

    /// This method returns all available dictionaries.
    /// The dictionary is not necessary loaded at this moment.
    virtual bool dictionaries(QStringList & list);

    /// Load the specified dictionary.
    virtual bool load(const QString & name);

    /// Load all dictionaries.
    virtual int loadAll();

    /// Get the dictionary specified by the name.
    virtual Dictionary * dictionary(const QString & name);

private:
    DictionaryImplPtr create(const QString &dir);

private:
    QStringList directories_;
    QVector<DictionaryImplPtr> dict_impls_;
};

}   // namespace stardict

#endif  // QSTARDICT_PLUGIN_IMPL_H_

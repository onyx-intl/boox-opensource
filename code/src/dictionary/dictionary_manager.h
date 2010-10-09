#ifndef DICTIONARY_MANAGER_H_
#define DICTIONARY_MANAGER_H_

#include <QVector>
#include <QString>
#include "dictionary_plugin.h"

/// Dictionary manager for naboo project.
/// It's designed to be able to use differnet dictionary backend.
class DictionaryManager
{
public:
    DictionaryManager(const QString& dict_root = QString());
    ~DictionaryManager();

public:
    int loadDictionaries();
    void dictionaries(QStringList & list) { list = all_dictionaries_; }

    bool info(const QString &name, DictionaryInfo &info);
    bool select(const QString &name);
    const QString & selected() { return selected_dictionary_; }

    bool translate(const QString &word, QString& result);
    bool similarWords(const QString &word, QStringList & result, const int offset, const int count);

private:
    void loadPlugins();
    void unloadPlugins();

    int load3rdPartyPlugins(const QString & folder);
    PluginPtr loadPluginFromSharedLibrary(const QString &path);

    DictionaryPtr dictionary(const QString &name);

private:
    QStringList root_dirs_;
    QString selected_dictionary_;
    QStringList all_dictionaries_;

    QVector<PluginPtr> plugins_;
};


#endif // DICTIONARY_MANAGER_H_

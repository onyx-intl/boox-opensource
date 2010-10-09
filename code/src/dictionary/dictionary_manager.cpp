
#include <QDir>
#include "onyx/sys/sys_conf.h"
#include "dictionary_manager.h"
#include "qstardict_plugin/qstardict_plugin_impl.h"



DictionaryManager::DictionaryManager(const QString& dict_root)
{
    // The root can be empty, when empty, load the default dictionaries.
    if (dict_root.isEmpty())
    {
        sys::SystemConfig conf;
        conf.dictionaryRoots(root_dirs_);
        selected_dictionary_ = conf.selectedDictionary();
    }
    else
    {
        root_dirs_ << dict_root;
    }
}

DictionaryManager::~DictionaryManager()
{
}

/// Get dictionary information.
bool DictionaryManager::info(const QString &name, DictionaryInfo &info)
{
    DictionaryPtr dict = dictionary(name);
    if (dict)
    {
        return dict->info(info);
    }
    return false;
}

/// Load all dictionaries.
int DictionaryManager::loadDictionaries()
{
    // Make sure all plugins have been loaded.
    loadPlugins();

    int count = 0;
    foreach(PluginPtr ptr, plugins_)
    {
        count += ptr->loadAll();
    }

    // Update dictionary list.
    all_dictionaries_.clear();
    foreach(PluginPtr ptr, plugins_)
    {
        ptr->dictionaries(all_dictionaries_);
    }

    // Use the first one as current dictionary if it's not available yet.
    if (selected_dictionary_.isEmpty() && !all_dictionaries_.isEmpty())
    {
        selected_dictionary_ = all_dictionaries_.front();
    }

    return count;
}

/// This function searches in all the dictionaries. It returns the first
/// matched result. The result string contains the original data read
/// from dictionary file.
bool DictionaryManager::translate(const QString &word,
                                  QString& result)
{
    if (word.isEmpty())
    {
        result.clear();
        return false;
    }

    // Check dictionary in used.
    if (selected_dictionary_.isEmpty())
    {
        qWarning("Not dictionary is selected.");
        return false;
    }

    DictionaryPtr dict = dictionary(selected_dictionary_);
    if (dict == 0)
    {
        qWarning("No dictionary %s found", qPrintable(selected_dictionary_));
        return false;
    }

    // Use try...catch in case if there is problem with dictionary plugin.
    try
    {
        dict->translate(word, result);
    }
    catch(...)
    {
        qWarning("Dictionary exception catched.");
        return false;
    }

    return true;
}

/// This function returns the similar words around the specified word.
/// \count How many similar words that need to return.
bool DictionaryManager::similarWords(const QString &word,
                                     QStringList & result,
                                     const int offset,
                                     const int count)
{
    if (word.isEmpty())
    {
        qWarning("Word is empty.");
        return false;
    }

    // Check dictionary in used.
    if (selected_dictionary_.isEmpty())
    {
        qWarning("Not dictionary is selected.");
        return false;
    }

    DictionaryPtr dict = dictionary(selected_dictionary_);
    if (dict == 0)
    {
        qWarning("No dictionary %s found", qPrintable(selected_dictionary_));
        return false;
    }

    // Use try...catch in case if there is problem with dictionary plugin.
    try
    {
        dict->similarWords(word, result, offset, count);
    }
    catch(...)
    {
        qWarning("Dictionary exception catched.");
        return false;
    }

    return true;
}

/// Select the specified dictionary.
bool DictionaryManager::select(const QString &name)
{
    if (all_dictionaries_.contains(name))
    {
        selected_dictionary_ = name;
        sys::SystemConfig conf;
        conf.selectDictionary(selected_dictionary_);
        return true;
    }
    return false;
}

void DictionaryManager::loadPlugins()
{
    if (plugins_.size() > 0)
    {
        return;
    }

    // So far, only support star dictionary backend.
    // More backend will be added later.
    plugins_.push_back(new stardict::StarDictionaryPluginImpl(root_dirs_));

    // Load all the other 3rd party plugins.
    QDir dir(SHARE_ROOT);
    if (!dir.cd("dicts"))
    {
        qDebug("Could not cd dicts.");
        return;
    }
    QDir::Filters filters = QDir::Dirs|QDir::NoDotAndDotDot;
    QFileInfoList all = dir.entryInfoList(filters);
    foreach(QFileInfo info, all)
    {
        load3rdPartyPlugins(info.absolutePath());
    }
}

void DictionaryManager::unloadPlugins()
{
}

/// Get the dictionary by specified name.
DictionaryPtr DictionaryManager::dictionary(const QString &name)
{
    DictionaryPtr dictionary = 0;
    foreach(PluginPtr ptr, plugins_)
    {
        dictionary = ptr->dictionary(name);
        if (dictionary)
        {
            break;
        }
    }
    return dictionary;
}

int DictionaryManager::load3rdPartyPlugins(const QString & folder)
{
    QDir dir(folder);
    QDir::Filters filters = QDir::Files|QDir::NoDotAndDotDot;
    QFileInfoList all = dir.entryInfoList(filters);
    foreach(QFileInfo info, all)
    {
        qDebug("Try to load plugin from %s", qPrintable(info.absoluteFilePath()));
        PluginPtr ptr = loadPluginFromSharedLibrary(info.absoluteFilePath());
        if (ptr)
        {
            qDebug("Load plugin %s OK.", qPrintable(info.absoluteFilePath()));
            plugins_.push_back(ptr);
        }
    }
    return all.size();
}

PluginPtr DictionaryManager::loadPluginFromSharedLibrary(const QString &path)
{
    QPluginLoader loader(path, 0);
    QObject *obj  = loader.instance();
    if (obj)
    {
        return qobject_cast<PluginPtr>(obj);
    }
    else
    {
        qDebug("Error in loading dictionary plugin: %s", qPrintable(loader.errorString()));
    }
    return 0;
}


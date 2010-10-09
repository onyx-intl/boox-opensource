
#include "qstardict_plugin_impl.h"

namespace stardict
{

// Upper the first character and lower others.
static QString upFirstChar(const QString & word)
{
    QString result = word.toLower();
    result.at(0).toUpper();
    return result;
}

// Cut s or d.
static QString cutSorD(const QString & word)
{
    if (word.endsWith("s", Qt::CaseInsensitive) ||
        word.endsWith("ed", Qt::CaseInsensitive))
    {
        QString result = word.left(word.size() - 1);
        return result;
    }

    return word;
}


StarDictionaryImpl::StarDictionaryImpl()
: is_loaded_(false)
{
}

StarDictionaryImpl::~StarDictionaryImpl()
{
}

bool StarDictionaryImpl::info(DictionaryInfo& info)
{
    DictInfo & ref = dict_impl_.info();
    info.name = ref.bookname;
    info.description = ref.description;
    info.author = ref.author;
    return true;
}

bool StarDictionaryImpl::load(const QString & working_directory)
{
    // Extract the ifo file.
    QDir dir(working_directory);
    QFileInfoList all = dir.entryInfoList(QDir::Files);
    foreach(QFileInfo info, all)
    {
        if (info.absoluteFilePath().endsWith(".ifo"))
        {
            is_loaded_ = dict_impl_.load(info.absoluteFilePath());
            return is_loaded_;
        }
    }
    return false;
}

bool StarDictionaryImpl::isLoaded()
{
    return is_loaded_;
}

bool StarDictionaryImpl::close()
{
    return true;
}

/// When translate, we try to find the exact word.
bool StarDictionaryImpl::translate(const QString &word, QString& result)
{
    long index = INVALID_INDEX;
    if (find(word, index))
    {
        result = dict_impl_.data(index);
        return true;
    }

    QString str;
    str = cutSorD(word);
    if (dict_impl_.lookup(str, index))
    {
        result = dict_impl_.data(index);
        return true;
    }

    return false;
}

/// Find similar words in this dictionary.
bool StarDictionaryImpl::similarWords(const QString &word,
                                      QStringList & result,
                                      const int offset,
                                      const int count)
{
    long index = INVALID_INDEX;
    if (find(word, index))
    {
        for(int i = 0; i < count; ++i)
        {
            result.append(dict_impl_.key(index + i + offset));
        }
        return true;
    }

    return false;
}

/// Find the index for the word.
bool StarDictionaryImpl::find(const QString & word, long & index)
{
    index = INVALID_INDEX;
    if (dict_impl_.lookup(word, index))
    {
        return true;
    }

    // Try the others.
    QString str = word.toLower();
    if (dict_impl_.lookup(str, index))
    {
        return true;
    }

    // To upper.
    str = word.toUpper();
    if (dict_impl_.lookup(str, index))
    {
        return true;
    }

    str = upFirstChar(word);
    if (dict_impl_.lookup(str, index))
    {
        return true;
    }
    return false;
}

bool StarDictionaryImpl::fuzzyFind(const QString & word, long & index)
{
    return true;
}




/// Implement the dictionary plugin.
StarDictionaryPluginImpl::StarDictionaryPluginImpl(const QStringList &directories)
: DictionaryPlugin(directories)
, directories_(directories)
{
}

StarDictionaryPluginImpl::~StarDictionaryPluginImpl()
{
}

/// Return all dictionaries found.
bool StarDictionaryPluginImpl::dictionaries(QStringList & list)
{
    foreach(DictionaryImplPtr ptr, dict_impls_)
    {
        DictionaryInfo info;
        if (ptr->info(info))
        {
            list.push_back(info.name);
        }
    }
    return true;
}

bool StarDictionaryPluginImpl::load(const QString & name)
{
    return false;
}

int StarDictionaryPluginImpl::loadAll()
{
    // Check if we already load dictionary.
    if (dict_impls_.size() > 0)
    {
        return dict_impls_.size();
    }

    // parent_path
    //  |-- dict a
    //  |-- dict b
    foreach(QString parent_path, directories_)
    {
        QDir dir(parent_path);
        if (!dir.exists())
        {
            continue;
        }

        QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QFileInfo info, all)
        {
            DictionaryImplPtr ptr = create(info.absoluteFilePath());
            if (ptr)
            {
                dict_impls_.push_back(ptr);
            }
        }
    }

    return dict_impls_.size();
}

Dictionary * StarDictionaryPluginImpl::dictionary(const QString & name)
{
    foreach(DictionaryImplPtr ptr, dict_impls_)
    {
        DictionaryInfo info;
        if (ptr->info(info) && name == info.name)
        {
            return ptr;
        }
    }
    return 0;
}

DictionaryImplPtr StarDictionaryPluginImpl::create(const QString &dir)
{
    DictionaryImplPtr ptr = new StarDictionaryImpl;
    if (ptr->load(dir))
    {
        return ptr;
    }
    delete ptr;
    return 0;
}

}   // namespace stardict


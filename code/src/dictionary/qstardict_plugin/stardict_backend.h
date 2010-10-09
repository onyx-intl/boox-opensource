
/// This file implements star dictionary based backend for star dictionary plugin.
#ifndef STARDICT_BACKEND_H__
#define STARDICT_BACKEND_H__

#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <QStringList>

#include "stardict_ziplib.h"
#include "stardict_base.h"

namespace stardict
{


/// Dictionary base class.
class DictBase
{
public:
    DictBase();
    ~DictBase();
    bool wordData(quint32 idxitem_offset, quint32 idxitem_size, QByteArray & data);
    bool containSearchData();
    bool searchData(std::vector<std::string> &SearchWords, quint32 idxitem_offset,
                    quint32 idxitem_size, QByteArray &origin_data);

protected:
    std::string sametypesequence;
    FILE *dictfile;
    std::auto_ptr<DictData> dictdzfile;

private:
    static const int WORDDATA_CACHE_NUM = 10;
    struct CacheItem
    {
        quint32 offset;
        QByteArray data;
    };
    CacheItem cache[WORDDATA_CACHE_NUM];
    qint32 cache_cur;
};

/// Implement dictionary. Internally it uses index file.
class Dict : public DictBase
{
public:
    Dict(){}
    ~Dict() {}

public:
    bool load(const QString& ifofilename);

    inline ulong narticles() { return wordcount;}
    inline const QString& dict_name() { return bookname; }
    inline const QString& ifofilename() { return ifo_file_name; }

    inline QString key(long index) { return idx_file->key(index); }

    inline QString data(const long index)
    {
        idx_file->data(index);
        QByteArray data;
        DictBase::wordData(idx_file->wordentry_offset, idx_file->wordentry_size, data);

        // Start from data() + sizeof(qunit32) + size(char)
        // The size(char) is the sametypesequence.
        return QString::fromUtf8(data.constData() + sizeof(quint32) + sizeof(char));
    }

    void keyAndData(long index,QString &key, quint32 &offset, quint32 &size)
    {
        key = idx_file->keyAndData(index);
        offset = idx_file->wordentry_offset;
        size = idx_file->wordentry_size;
    }

    bool lookup(const QString &str, long &idx)
    {
        return idx_file->lookup(str, idx);
    }

    DictInfo & info() { return dict_info; }

private:
    bool loadFromIfo(const QString& ifofilename, ulong &idxfilesize);

private:
    QString ifo_file_name;
    ulong wordcount;
    QString bookname;
    DictInfo dict_info;
    std::auto_ptr<IndexFile> idx_file;
    // bool LookupWithRule(GPatternSpec *pspec, long *aIndex, int iBuffLen);
};

/// Servers as dictionary container. todo, to be removed or combined with dict plugin.
class Libs
{
public:
    Libs();
    ~Libs();

    void load_all(const QString & base_path);
    void load_dict(const QString& url);

    long narticles(int idict)
    {
        return oLib[idict]->narticles();
    }
    const QString& dict_name(int idict)
    {
        return oLib[idict]->dict_name();
    }
    qint32 ndicts()
    {
        return oLib.size();
    }

    QString poGetWord(long iIndex, int iLib)
    {
        return oLib[iLib]->key(iIndex);
    }
    QString poGetWordData(long iIndex, int iLib);
    QString poGetCurrentWord(long *iCurrent);
    QString poGetNextWord(const QString &word, long *iCurrent);
    QString poGetPreWord(long *iCurrent);
    bool LookupWord(const QString & sWord, long& iWordIndex, int iLib)
    {
        return oLib[iLib]->lookup(sWord, iWordIndex);
    }

    bool LookupSimilarWord(const QString& sWord, long & iWordIndex, int iLib);
    bool SimpleLookupWord(const QString&  sWord, long & iWordIndex, int iLib);


    bool LookupWithFuzzy(const QString& sWord, QStringList &reslist, qint32 iLib);
    qint32 LookupWithRule(const QString& sWord, QStringList &reslist);
    bool LookupData(const QString& sWord, QStringList &reslist);

private:
    std::vector<Dict *> oLib; // word Libs.
    int iMaxFuzzyDistance;
};


enum query_t
{
    qtSIMPLE, qtREGEXP, qtFUZZY, qtDATA
};

extern query_t analyze_query(const char *s, QString& res);

};  // namespace stardict

#endif // STARDICT_BACKEND_H__

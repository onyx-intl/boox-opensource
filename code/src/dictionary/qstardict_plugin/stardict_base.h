
/// This file implements star dictionary based backend for star dictionary plugin.
#ifndef STARDICT_BASE_H__
#define STARDICT_BASE_H__

#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <QStringList>

namespace stardict
{


/// Contains all information about dictionary. Helper class.
struct DictInfo
{
    QString ifo_file_name;
    quint32 wordcount;
    QString bookname;
    QString author;
    QString email;
    QString website;
    QString date;
    QString description;
    quint32 index_file_size;
    QString sametypesequence;
    bool loadFromIfo(const QString& ifofilename, bool istreedict);
};



/// Index File base class. Helper class.
class IndexFile
{
public:
    virtual ~IndexFile()
    {}
    virtual bool load(const QString& url, ulong wc, ulong fsize) = 0;
    virtual QString key(long idx) = 0;
    virtual void data(long idx) = 0;
    virtual QString keyAndData(long idx) = 0;
    virtual bool lookup(const QString &key, long &idx) = 0;

public:
    quint32 wordentry_offset;
    quint32 wordentry_size;
};



/// Offset based index file.
class OffsetIndex : public IndexFile
{
public:
    OffsetIndex() : idxfile(NULL)
    {}
    ~OffsetIndex();
    bool load(const QString& url, ulong wc, ulong fsize);
    QString key(long idx);
    void data(long idx);
    QString keyAndData(long idx);
    bool lookup(const QString &str, long &idx);

private:
    static const int ENTR_PER_PAGE = 32;
    static const char *CACHE_MAGIC;

    std::vector<quint32> wordoffset;
    FILE *idxfile;
    ulong wordcount;
    ulong npages;

    // The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.
    char wordentry_buf[256 + sizeof(quint32)*2];
    struct index_entry
    {
        long idx;
        QString keystr;
        void assign(long i, const QString& str)
        {
            idx = i;
            keystr = str;
        }
    };
    index_entry first, last, middle, real_last;

    struct page_entry
    {
        QString keystr;
        quint32 off, size;
    };
    std::vector<char> page_data;
    struct page_t
    {
        long idx;
        page_entry entries[ENTR_PER_PAGE];

        page_t(): idx( -1)
        {}
        void fill(char *data, int nent, long idx_);
    }
    page;
    ulong load_page(long page_idx);
    QString read_first_on_page_key(long page_idx);
    QString get_first_on_page_key(long page_idx);
    bool load_cache(const QString& url);
    bool save_cache(const QString& url);
    static QStringList get_cache_variant(const QString& url);
};


/// Wordlist based index file.
class WordlistIndex : public IndexFile
{
public:
    WordlistIndex() : idxdatabuf(NULL)
    {}
    ~WordlistIndex();
    bool load(const QString& url, ulong wc, ulong fsize);
    QString key(long idx);
    void data(long idx);
    QString keyAndData(long idx);
    bool lookup(const QString &str, long &idx);

private:
    QByteArray idxdatabuf;
    std::vector<char *> wordlist;
};

static const int INVALID_INDEX = -100;

int stardict_strcmp(const QString &s1, const QString &s2);

int prefix_match (const QString & s1, const QString & s2);

};  // namespace stardict

#endif // STARDICT_BASE_H__

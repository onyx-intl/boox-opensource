
#include <algorithm>
#include <cstring>
#include <cctype>
#include <zlib.h>
#include "stardict_base.h"

#include <QFileInfo>
#include <QDir>
#include <QtEndian>


namespace stardict
{

static const QString TREEDICT_MAGIC_DATA = "StarDict's treedict ifo file\n";
static const QString DICT_MAGIC_DATA = "StarDict's dict ifo file\n";
static const QString MIN_VERSION = "version=2.4.2";

/// Load dictionary information from .ifo file.
bool DictInfo::loadFromIfo(const QString& ifofilename,
                           bool istreedict)
{
    ifo_file_name = ifofilename;
    QFile file(ifo_file_name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Could not open file %s.", qPrintable(ifofilename));
        return false;
    }

    QString str;
    str = file.readLine();

    // magic word
    if (istreedict && str != TREEDICT_MAGIC_DATA)
    {
        qWarning("Incorrect tree dict magic word %s.", qPrintable(str));
        return false;
    }
    else if (str != DICT_MAGIC_DATA)
    {
        qWarning("Incorrect magic word %s.", qPrintable(str));
        return false;
    }

    // min version.
    str = file.readLine();
    if (str < MIN_VERSION)
    {
        qWarning("Version is too low.");
        return false;
    }

    QByteArray data;
    while (!file.atEnd())
    {
        data = file.readLine().trimmed();

        if (data.startsWith("wordcount"))
        {
            // Read wordcount.
            data.remove(0, strlen("wordcount="));
            wordcount = data.toInt();
        }
        else if (data.startsWith("idxfilesize"))
        {
            // Read idxfilesize.
            data.remove(0, strlen("idxfilesize="));
            index_file_size = data.toInt();
        }
        else if (data.startsWith("bookname"))
        {
            // Read bookname.
            bookname = QString::fromUtf8(data.remove(0, strlen("bookname=")).constData());
        }
        else if (data.startsWith("author"))
        {
            // Read author.
            author = QString::fromUtf8(data.remove(0, strlen("author=")).constData());
        }
        else if (data.startsWith("email"))
        {
            // Read email.
            email = QString::fromUtf8(data.remove(0, strlen("email=")).constData());
        }
        else if (data.startsWith("website"))
        {
            // Read website.
            website = QString::fromUtf8(data.remove(0, strlen("website=")).constData());
        }
        else if (data.startsWith("date"))
        {
            // Read date.
            date = QString::fromUtf8(data.remove(0, strlen("date=")).constData());
        }
        else if (data.startsWith("description"))
        {
            // Read description.
            description = QString::fromUtf8(data.remove(0, strlen("description=")).constData());
        }
        else if (data.startsWith("sametypesequence"))
        {
            // Read sametypesequence.
            sametypesequence = QString::fromUtf8(data.remove(0, strlen("sametypesequence=")).constData());
        }
    }

    file.close();
    return true;
}

const char *OffsetIndex::CACHE_MAGIC = "StarDict's Cache, Version: 0.1";
void OffsetIndex::page_t::fill(char *data, int nent, long idx_)
{
    idx = idx_;
    char *p = data;
    long len;
    for (int i = 0; i < nent; ++i)
    {
        entries[i].keystr = QString::fromUtf8(p);
        len = strlen(p);
        p += len + 1;
        entries[i].off = qToBigEndian(*reinterpret_cast<quint32 *>(p));
        p += sizeof(quint32);
        entries[i].size = qToBigEndian(*reinterpret_cast<quint32 *>(p));
        p += sizeof(quint32);
    }
}

OffsetIndex::~OffsetIndex()
{
    if (idxfile)
        fclose(idxfile);
}

inline QString OffsetIndex::read_first_on_page_key(long page_idx)
{
    fseek(idxfile, wordoffset[page_idx], SEEK_SET);
    quint32 page_size = wordoffset[page_idx + 1] - wordoffset[page_idx];

    //TODO: check returned values, deal with word entry that strlen>255.
    fread(wordentry_buf, std::min<quint32>(sizeof(wordentry_buf), page_size), 1, idxfile);
    return QString::fromUtf8(wordentry_buf);
}

inline QString OffsetIndex::get_first_on_page_key(long page_idx)
{
    if (page_idx < middle.idx)
    {
        if (page_idx == first.idx)
            return first.keystr;
        return read_first_on_page_key(page_idx);
    }
    else if (page_idx > middle.idx)
    {
        if (page_idx == last.idx)
            return last.keystr;
        return read_first_on_page_key(page_idx);
    }
    else
    {
        return middle.keystr;
    }
}

bool OffsetIndex::load_cache(const QString& url)
{
    return false;
}

QStringList OffsetIndex::get_cache_variant(const QString& url)
{
    QStringList res(url + ".oft");
    return res;
}

/// No cache support.
bool OffsetIndex::save_cache(const QString& url)
{
    return false;
}

bool OffsetIndex::load(const QString& url, ulong wc, ulong fsize)
{
    wordcount = wc;
    npages = (wc - 1) / ENTR_PER_PAGE + 2;
    wordoffset.resize(npages);
    if (!load_cache(url))
    {
        //map file will close after finish of block
        QFile map_file(url);
        if (!map_file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        uchar * map_address = map_file.map(0, map_file.size());
        const char *idxdatabuffer = reinterpret_cast<const char *>(map_address);
        const char *p1 = idxdatabuffer;
        ulong index_size;
        quint32 j = 0;
        for (quint32 i = 0; i < wc; i++)
        {
            index_size = strlen(p1) + 1 + 2 * sizeof(quint32);
            if (i % ENTR_PER_PAGE == 0)
            {
                wordoffset[j] = p1 - idxdatabuffer;
                ++j;
            }
            p1 += index_size;
        }
        wordoffset[j] = p1 - idxdatabuffer;

        map_file.unmap(map_address);
    }

    if (!(idxfile = fopen(url.toLocal8Bit().data(), "rb")))
    {
        wordoffset.resize(0);
        return false;
    }

    first.assign(0, read_first_on_page_key(0));
    last.assign(npages-2, read_first_on_page_key(npages-2));
    middle.assign((npages-2)/2, read_first_on_page_key((npages-2)/2));
    real_last.assign(wc-1, key(wc-1));

    return true;
}

inline ulong OffsetIndex::load_page(long page_idx)
{
    ulong nentr = ENTR_PER_PAGE;
    if (page_idx == long(npages - 2))
        if ((nentr = wordcount % ENTR_PER_PAGE) == 0)
            nentr = ENTR_PER_PAGE;


    if (page_idx != page.idx)
    {
        page_data.resize(wordoffset[page_idx + 1] - wordoffset[page_idx]);
        fseek(idxfile, wordoffset[page_idx], SEEK_SET);
        fread(&page_data[0], 1, page_data.size(), idxfile);
        page.fill(&page_data[0], nentr, page_idx);
    }

    return nentr;
}

QString OffsetIndex::key(long idx)
{
    load_page(idx / ENTR_PER_PAGE);
    long idx_in_page = idx % ENTR_PER_PAGE;
    wordentry_offset = page.entries[idx_in_page].off;
    wordentry_size = page.entries[idx_in_page].size;

    return page.entries[idx_in_page].keystr;
}

void OffsetIndex::data(long idx)
{
    key(idx);
}

QString OffsetIndex::keyAndData(long idx)
{
    return key(idx);
}


bool OffsetIndex::lookup(const QString &str, long &idx)
{
    long idx_suggest;
    bool bFound=false;
    long iFrom;
    long iTo= npages -2;

    // dump for debug.
    /*
    for(long sss = 0; sss < iTo; ++sss)
    {
    qDebug("%s", qPrintable(get_first_on_page_key(sss)));
    }
    */

    int cmpint;
    long iThisIndex;
    if (stardict_strcmp(str, first.keystr)<0)
    {
        idx = 0;
        idx_suggest = 0;
        return false;
    }
    else if (stardict_strcmp(str, real_last.keystr) >0)
    {
        idx = INVALID_INDEX;
        idx_suggest = iTo;
        return false;
    }
    else
    {
        iFrom=0;
        iThisIndex=0;
        while (iFrom<=iTo)
        {
            iThisIndex=(iFrom+iTo)/2;
            cmpint = stardict_strcmp(str, get_first_on_page_key(iThisIndex));
            if (cmpint>0)
                iFrom=iThisIndex+1;
            else if (cmpint<0)
                iTo=iThisIndex-1;
            else
            {
                bFound=true;
                break;
            }
        }
        if (!bFound) {
            idx = iTo;    //prev
        } else {
            idx = iThisIndex;
        }
    }
    if (!bFound)
    {
        ulong netr=load_page(idx);
        iFrom=1; // Needn't search the first word anymore.
        iTo=netr-1;
        iThisIndex=0;
        while (iFrom<=iTo)
        {
            iThisIndex=(iFrom+iTo)/2;
            cmpint = stardict_strcmp(str, page.entries[iThisIndex].keystr);
            if (cmpint>0)
                iFrom=iThisIndex+1;
            else if (cmpint<0)
                iTo=iThisIndex-1;
            else {
                bFound=true;
                break;
            }
        }
        idx*=ENTR_PER_PAGE;
        if (!bFound)
        {
            idx += iFrom;    //next
            idx_suggest = idx;
            int best, back;
            best = prefix_match (str, page.entries[idx_suggest % ENTR_PER_PAGE].keystr);
            for (;;)
            {
                if ((iTo=idx_suggest-1) < 0)
                    break;
                if (idx_suggest % ENTR_PER_PAGE == 0)
                    load_page(iTo / ENTR_PER_PAGE);
                back = prefix_match (str, page.entries[iTo % ENTR_PER_PAGE].keystr);
                if (!back || back < best)
                    break;
                best = back;
                idx_suggest = iTo;
            }
        }
        else
        {
            idx += iThisIndex;
            idx_suggest = idx;
        }
    }
    else
    {
        idx*=ENTR_PER_PAGE;
        idx_suggest = idx;
    }
    return bFound;
}


/// Word list based index file.
WordlistIndex::~WordlistIndex()
{
}

bool WordlistIndex::load(const QString& url, ulong wc, ulong fsize)
{
    gzFile in = gzopen(url.toLocal8Bit().data(), "rb");
    if (in == NULL)
        return false;

    idxdatabuf.reserve(fsize);

    ulong len = gzread(in, idxdatabuf.data(), fsize);
    gzclose(in);

    if (len != fsize)
        return false;

    wordlist.resize(wc + 1);
    char *p1 = const_cast<char *>(idxdatabuf.constData());
    quint32 i;
    for (i = 0; i < wc; i++)
    {
        wordlist[i] = p1;
        p1 += strlen(p1) + 1 + 2 * sizeof(quint32);
    }
    wordlist[wc] = p1;

    return true;
}

QString WordlistIndex::key(long idx)
{
    return wordlist[idx];
}

void WordlistIndex::data(long idx)
{
    char *p1 = wordlist[idx] + strlen(wordlist[idx]) + sizeof(char);
    wordentry_offset = qToBigEndian(*reinterpret_cast<quint32 *>(p1));
    p1 += sizeof(quint32);
    wordentry_size = qToBigEndian(*reinterpret_cast<quint32 *>(p1));
}

QString WordlistIndex::keyAndData(long idx)
{
    data(idx);
    return key(idx);
}

bool WordlistIndex::lookup(const QString &str, long &idx)
{
    bool bFound = false;
    long iTo = wordlist.size() - 2;

    if (str.compare(key(0)) < 0)
    {
        idx = 0;
    }
    else if (str.compare(key(iTo)) > 0)
    {
        idx = INVALID_INDEX;
    }
    else
    {
        long iThisIndex = 0;
        long iFrom = 0;
        int cmpint;
        while (iFrom <= iTo)
        {
            iThisIndex = (iFrom + iTo) / 2;
            cmpint = str.compare(key(iThisIndex));
            if (cmpint > 0)
                iFrom = iThisIndex + 1;
            else if (cmpint < 0)
                iTo = iThisIndex - 1;
            else
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
            idx = iFrom;    //next
        else
            idx = iThisIndex;
    }
    return bFound;
}

int stardict_strcmp(const QString &s1,
                    const QString &s2)
{

    int a = s1.compare(s2, Qt::CaseInsensitive);
    if (a == 0)
        return s1.compare(s2);
    else
        return a;
}

int prefix_match (const QString & s1,
                  const QString & s2)
{
    int ret = -1;
    int i = 0;
    int min = std::min(s1.size(), s2.size());
    QChar u1, u2;
    do
    {
        u1 = s1.at(i);
        u2 = s2.at(i);
        ret++;
        ++i;
    }
    while (!u1.isNull() && u1.toLower() == u2.toLower() && i < min);
    return ret;
}

}   // namespace stardict


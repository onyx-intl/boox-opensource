
#include <algorithm>
#include <cstring>
#include <cctype>
#include <zlib.h>
#include "distance.h"
#include "stardict_backend.h"

#include <QFileInfo>
#include <QDir>
#include <QtEndian>

namespace stardict
{

static const int MAX_MATCH_ITEM_PER_LIB = 100;
static const int MAX_FUZZY_DISTANCE = 3; // at most MAX_FUZZY_DISTANCE-1 differences allowed when find similar words

// Notice: read src/tools/DICTFILE_FORMAT for the dictionary
// file's format information!

/// Borrow from glib.
static const char * g_strstr_len (const char *haystack,
                                  int       haystack_len,
                                  const char *needle)
{
    if (haystack == NULL) return NULL;
    if (needle == NULL) return  NULL;

    if (haystack_len < 0)
        return strstr (haystack, needle);
    else
    {
        const char *p = haystack;
        size_t needle_len = strlen (needle);
        const char *end;
        size_t i;

        if (needle_len == 0)
            return (char *)haystack;

        if (static_cast<size_t>(haystack_len) < needle_len)
            return NULL;

        end = haystack + haystack_len - needle_len;

        while (*p && p <= end)
        {
            for (i = 0; i < needle_len; i++)
                if (p[i] != needle[i])
                    goto next;

            return (char *)p;

next:
            p++;
        }

        return NULL;
    }
}

static inline bool bIsVowel(QChar inputchar)
{
    char ch = inputchar.toUpper().toAscii();
    return ( ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U' );
}

static bool bIsPureEnglish(const QString & str)
{
    for(int i = 0; i < str.length(); ++i)
    {
        if (!str[i].isLetter())
            return false;
    }
    return true;

    /*
    // i think this should work even when it is UTF8 string :).
    for (int i = 0; str[i] != 0; i++)
    //if(str[i]<0)
    //if(str[i]<32 || str[i]>126) // tab equal 9,so this is not OK.
    // Better use isascii() but not str[i]<0 while char is default unsigned in arm
    if (!isascii(str[i]))
    return false;
    return true;
    */
}


/// Implement dictionary base class.
DictBase::DictBase()
{
    dictfile = NULL;
    cache_cur = 0;
}

DictBase::~DictBase()
{
    if (dictfile)
        fclose(dictfile);
}

bool DictBase::wordData(quint32 idxitem_offset, quint32 idxitem_size, QByteArray & data)
{
    for (int i = 0; i < WORDDATA_CACHE_NUM; i++)
    {
        if (cache[i].offset == idxitem_offset)
        {
            data = cache[i].data;
            return true;
        }
    }

    if (dictfile)
    {
        fseek(dictfile, idxitem_offset, SEEK_SET);
    }

    if (!sametypesequence.empty())
    {
        QByteArray origin_data;
        origin_data.reserve(idxitem_size);

        if (dictfile)
            fread(origin_data.data(), idxitem_size, 1, dictfile);
        else
            dictdzfile->read(origin_data.data(), idxitem_offset, idxitem_size);

        quint32 data_size;
        int sametypesequence_len = sametypesequence.length();

        //there have sametypesequence_len char being omitted.
        // Here is a bug fix of 2.4.8, which don't add sizeof(quint32) anymore.
        data_size = idxitem_size +  sametypesequence_len;

        //if the last item's size is determined by the end up '\0',then +=sizeof(char);
        //if the last item's size is determined by the head quint32 type data,then +=sizeof(quint32);
        switch (sametypesequence[sametypesequence_len - 1])
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
        case 'k':
        case 'w':
            data_size += sizeof(char);
            break;
        case 'W':
        case 'P':
            data_size += sizeof(quint32);
            break;
        default:
            if (isupper(sametypesequence[sametypesequence_len - 1]))
                data_size += sizeof(quint32);
            else
                data_size += sizeof(char);
            break;
        }

        // Actually, when using QByteArray, we don't need to record the size any more.
        data.reserve(data_size + sizeof(quint32));
        char *p1, *p2;
        p1 = data.data() + sizeof(quint32);
        p2 = origin_data.data();
        quint32 sec_size;
        //copy the head items.
        for (int i = 0; i < sametypesequence_len - 1; i++)
        {
            *p1 = sametypesequence[i];
            p1 += sizeof(char);
            switch (sametypesequence[i])
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
            case 'k':
            case 'w':
                sec_size = strlen(p2) + 1;
                memcpy(p1, p2, sec_size);
                p1 += sec_size;
                p2 += sec_size;
                break;
            case 'W':
            case 'P':
                sec_size = *reinterpret_cast<quint32 *>(p2);
                sec_size += sizeof(quint32);
                memcpy(p1, p2, sec_size);
                p1 += sec_size;
                p2 += sec_size;
                break;
            default:
                if (isupper(sametypesequence[i]))
                {
                    sec_size = *reinterpret_cast<quint32 *>(p2);
                    sec_size += sizeof(quint32);
                }
                else
                {
                    sec_size = strlen(p2) + 1;
                }
                memcpy(p1, p2, sec_size);
                p1 += sec_size;
                p2 += sec_size;
                break;
            }
        }
        //calculate the last item 's size.
        sec_size = p2 - origin_data.data();
        sec_size = idxitem_size - sec_size; // (p2 - origin_data.data());

        // Not sure, we need the sametypesequence.
        *p1 = sametypesequence[sametypesequence_len - 1];
        p1 += sizeof(char);
        switch (sametypesequence[sametypesequence_len - 1])
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
        case 'k':
        case 'w':
            memcpy(p1, p2, sec_size);
            p1 += sec_size;
            *p1 = '\0'; //add the end up '\0';
            break;
        case 'W':
        case 'P':
            *reinterpret_cast<quint32 *>(p1) = sec_size;
            p1 += sizeof(quint32);
            memcpy(p1, p2, sec_size);
            break;
        default:
            if (isupper(sametypesequence[sametypesequence_len - 1]))
            {
                *reinterpret_cast<quint32 *>(p1) = sec_size;
                p1 += sizeof(quint32);
                memcpy(p1, p2, sec_size);
            }
            else
            {
                memcpy(p1, p2, sec_size);
                p1 += sec_size;
                *p1 = '\0';
            }
            break;
        }
        *reinterpret_cast<quint32 *>(data.data()) = data_size;
    }
    else
    {
        data.reserve(idxitem_size + sizeof(quint32));
        if (dictfile)
        {
            fread(data.data() + sizeof(quint32), idxitem_size, 1, dictfile);
        }
        else
        {
            dictdzfile->read(data.data() + sizeof(quint32), idxitem_offset, idxitem_size);
        }

        // copied from stardict, seems a bug fix.
        *reinterpret_cast<quint32 *>(data.data()) = idxitem_size; // + sizeof(quint32);
    }

    cache[cache_cur].data = data;
    cache[cache_cur].offset = idxitem_offset;
    cache_cur++;
    if (cache_cur == WORDDATA_CACHE_NUM)
        cache_cur = 0;
    return true;
}

inline bool DictBase::containSearchData()
{
    if (sametypesequence.empty())
        return true;

    return (sametypesequence.find_first_of("mlgxtykwh") != std::string::npos);
}

bool DictBase::searchData(std::vector<std::string> &SearchWords,
                          quint32 idxitem_offset,
                          quint32 idxitem_size,
                          QByteArray &origin_data)
{
    int nWord = SearchWords.size();
    std::vector<bool> WordFind(nWord, false);
    int nfound = 0;

    if (dictfile)
    {
        fseek(dictfile, idxitem_offset, SEEK_SET);
    }
    if (dictfile)
    {
        fread(origin_data.data(), idxitem_size, 1, dictfile);
    }
    else
    {
        dictdzfile->read(origin_data.data(), idxitem_offset, idxitem_size);
    }

    char *p = origin_data.data();
    quint32 sec_size;
    int j;
    if (!sametypesequence.empty())
    {
        int sametypesequence_len = sametypesequence.length();
        for (int i = 0; i < sametypesequence_len - 1; i++)
        {
            switch (sametypesequence[i])
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
            case 'k':
            case 'w':
            case 'h':
                for (j = 0; j < nWord; j++)
                    if (!WordFind[j] && strstr(p, SearchWords[j].c_str()))
                    {
                        WordFind[j] = true;
                        ++nfound;
                    }

                    if (nfound == nWord)
                        return true;
                    sec_size = strlen(p) + 1;
                    p += sec_size;
                    break;
            default:
                if (isupper(sametypesequence[i]))
                {
                    sec_size = *reinterpret_cast<quint32 *>(p);
                    sec_size += sizeof(quint32);
                }
                else
                {
                    sec_size = strlen(p) + 1;
                }
                p += sec_size;
            }
        }
        switch (sametypesequence[sametypesequence_len - 1])
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
        case 'k':
        case 'w':
        case 'h':
            sec_size = idxitem_size - (p - origin_data.data());
            for (j = 0; j < nWord; j++)
                if (!WordFind[j] &&
                    g_strstr_len(p, sec_size, SearchWords[j].c_str()))
                {
                    WordFind[j] = true;
                    ++nfound;
                }


                if (nfound == nWord)
                    return true;
                break;
        }
    }
    else
    {
        while (quint32(p - origin_data.data()) < idxitem_size)
        {
            switch (*p)
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
            case 'k':
            case 'w':
            case 'h':
                for (j = 0; j < nWord; j++)
                    if (!WordFind[j] && strstr(p, SearchWords[j].c_str()))
                    {
                        WordFind[j] = true;
                        ++nfound;
                    }

                    if (nfound == nWord)
                        return true;
                    sec_size = strlen(p) + 1;
                    p += sec_size;
                    break;
            default:
                if (isupper(*p))
                {
                    sec_size = *reinterpret_cast<quint32 *>(p);
                    sec_size += sizeof(quint32);
                }
                else
                {
                    sec_size = strlen(p) + 1;
                }
                p += sec_size;
            }
        }
    }
    return false;
}

/// Load dictionary from specified file.
bool Dict::load(const QString& ifofilename)
{
    ulong idxfilesize;
    if (!loadFromIfo(ifofilename, idxfilesize))
    {
        return false;
    }

    QString base(ifofilename);
    int pos = base.lastIndexOf(".ifo");
    base = base.mid(0, pos);

    QString fullfilename = base + ".dict.dz";
    QFileInfo info(fullfilename);
    if (info.exists())
    {
        dictdzfile.reset(new DictData);
        if (!dictdzfile->open(fullfilename, 0))
        {
            return false;
        }
    }
    else
    {
        /*
        // TODO
        fullfilename.remove(fullfilename.length() - sizeof(".dz") + 1, sizeof(".dz") - 1);
        dictfile = fopen(fullfilename.toStd(), "rb");
        if (!dictfile)
        {
        //g_print("open file %s failed!\n",fullfilename);
        return false;
        }
        */
    }

    fullfilename = base + ".idx.gz";;
    info.setFile(fullfilename);
    if (info.exists())
    {
        idx_file.reset(new WordlistIndex);
    }
    else
    {
        fullfilename = base + ".idx";
        idx_file.reset(new OffsetIndex);
    }


    if (!idx_file->load(fullfilename, wordcount, idxfilesize))
        return false;

    return true;
}

/// Load information from .ifo file.
bool Dict::loadFromIfo(const QString& ifofilename, ulong &idxfilesize)
{
    if (!dict_info.loadFromIfo(ifofilename, false))
    {
        return false;
    }

    if (dict_info.wordcount == 0)
    {
        return false;
    }

    ifo_file_name = dict_info.ifo_file_name;
    wordcount = dict_info.wordcount;
    bookname = dict_info.bookname;
    idxfilesize = dict_info.index_file_size;
    sametypesequence = dict_info.sametypesequence.toStdString();

    return true;
}


Libs::Libs()
{
    iMaxFuzzyDistance = MAX_FUZZY_DISTANCE; //need to read from cfg.
}

Libs::~Libs()
{
    for (std::vector<Dict *>::iterator p = oLib.begin(); p != oLib.end(); ++p)
        delete *p;
}

void Libs::load_all(const QString & base_path)
{
    QDir dir(base_path);

    if (dir.exists())
    {
        QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QFileInfo info, all)
        {
            QDir sub_dir(info.filePath(), "*.ifo", QDir::Unsorted, QDir::Files);
            QFileInfoList ifo_file_list = sub_dir.entryInfoList();

            // Only care the first ifo file.
            if (ifo_file_list.size() > 0)
            {
                load_dict(ifo_file_list.at(0).absoluteFilePath());
            }
        }
    }
}

void Libs::load_dict(const QString& url)
{
    Dict *lib = new Dict;
    if (lib->load(url))
        oLib.push_back(lib);
    else
        delete lib;
}

class DictLoader
{
public:
    DictLoader(Libs& lib_): lib(lib_)
    {}
    void operator()(const QString& url, bool disable)
    {
        if (!disable)
            lib.load_dict(url);
    }
private:
    Libs& lib;
};

class DictReLoader
{
public:
    DictReLoader(std::vector<Dict *> &p, std::vector<Dict *> &f,
        Libs& lib_) : prev(p), future(f), lib(lib_)
    {}
    void operator()(const QString& url, bool disable)
    {
        if (!disable)
        {
            Dict *dict = find(url);
            if (dict)
                future.push_back(dict);
            else
                lib.load_dict(url);
        }
    }
private:
    std::vector<Dict *> &prev;
    std::vector<Dict *> &future;
    Libs& lib;

    Dict *find(const QString& url)
    {
        std::vector<Dict *>::iterator it;
        for (it = prev.begin(); it != prev.end(); ++it)
            if ((*it)->ifofilename() == url)
                break;
        if (it != prev.end())
        {
            Dict *res = *it;
            prev.erase(it);
            return res;
        }
        return NULL;
    }
};

QString Libs::poGetWordData(long iIndex, int iLib)
{
    if (iIndex == INVALID_INDEX)
        return QString();
    return oLib[iLib]->data(iIndex);
}


QString Libs::poGetCurrentWord(long * iCurrent)
{
    QString poCurrentWord;
    QString word;
    for (std::vector<Dict *>::size_type iLib = 0; iLib<oLib.size(); iLib++)
    {
        if (iCurrent[iLib] == INVALID_INDEX)
            continue;
        if ( iCurrent[iLib] >= narticles(iLib) || iCurrent[iLib] < 0)
            continue;
        if ( poCurrentWord == NULL )
        {
            poCurrentWord = poGetWord(iCurrent[iLib], iLib);
        }
        else
        {
            word = poGetWord(iCurrent[iLib], iLib);

            if (poCurrentWord.compare(word) > 0 )
                poCurrentWord = word;
        }
    }
    return poCurrentWord;
}

QString
Libs::poGetNextWord(const QString &sWord, long *iCurrent)
{
    // the input can be:
    // (word,iCurrent),read word,write iNext to iCurrent,and return next word. used by TopWin::NextCallback();
    // (NULL,iCurrent),read iCurrent,write iNext to iCurrent,and return next word. used by AppCore::ListWords();
    QString poCurrentWord;
    std::vector<Dict *>::size_type iCurrentLib = 0;
    QString word;

    for (std::vector<Dict *>::size_type iLib = 0;iLib<oLib.size();iLib++)
    {
        if (!sWord.isEmpty())
            oLib[iLib]->lookup(sWord, iCurrent[iLib]);
        if (iCurrent[iLib] == INVALID_INDEX)
            continue;
        if (iCurrent[iLib] >= narticles(iLib) || iCurrent[iLib] < 0)
            continue;
        if (poCurrentWord == NULL )
        {
            poCurrentWord = poGetWord(iCurrent[iLib], iLib);
            iCurrentLib = iLib;
        }
        else
        {
            word = poGetWord(iCurrent[iLib], iLib);

            if (poCurrentWord.compare(word) > 0 )
            {
                poCurrentWord = word;
                iCurrentLib = iLib;
            }
        }
    }
    if (!poCurrentWord.isEmpty())
    {
        iCurrent[iCurrentLib]++;
        for (std::vector<Dict *>::size_type iLib = 0;iLib<oLib.size();iLib++)
        {
            if (iLib == iCurrentLib)
                continue;
            if (iCurrent[iLib] == INVALID_INDEX)
                continue;
            if ( iCurrent[iLib] >= narticles(iLib) || iCurrent[iLib] < 0)
                continue;
            if (poCurrentWord == poGetWord(iCurrent[iLib], iLib))
                iCurrent[iLib]++;
        }
        poCurrentWord = poGetCurrentWord(iCurrent);
    }
    return poCurrentWord;
}


QString
Libs::poGetPreWord(long * iCurrent)
{
    // used by TopWin::PreviousCallback(); the iCurrent is cached by AppCore::TopWinWordChange();
    QString poCurrentWord;
    std::vector<Dict *>::size_type iCurrentLib = 0;
    QString word;

    for (std::vector<Dict *>::size_type iLib = 0;iLib<oLib.size();iLib++)
    {
        if (iCurrent[iLib] == INVALID_INDEX)
            iCurrent[iLib] = narticles(iLib);
        else
        {
            if ( iCurrent[iLib] > narticles(iLib) || iCurrent[iLib] <= 0)
                continue;
        }
        if ( poCurrentWord == NULL )
        {
            poCurrentWord = poGetWord(iCurrent[iLib] - 1, iLib);
            iCurrentLib = iLib;
        }
        else
        {
            word = poGetWord(iCurrent[iLib] - 1, iLib);
            if (poCurrentWord.compare(word) < 0 )
            {
                poCurrentWord = word;
                iCurrentLib = iLib;
            }
        }
    }

    if (!poCurrentWord.isEmpty())
    {
        iCurrent[iCurrentLib]--;
        for (std::vector<Dict *>::size_type iLib = 0;iLib<oLib.size();iLib++)
        {
            if (iLib == iCurrentLib)
                continue;
            if (iCurrent[iLib] > narticles(iLib) || iCurrent[iLib] <= 0)
                continue;
            if (poCurrentWord ==  poGetWord(iCurrent[iLib] - 1, iLib))
            {
                iCurrent[iLib]--;
            }
            else
            {
                if (iCurrent[iLib] == narticles(iLib))
                    iCurrent[iLib] = INVALID_INDEX;
            }
        }
    }
    return poCurrentWord;
}

bool Libs::LookupSimilarWord(const QString & sWord, long & iWordIndex, int iLib)
{
    long iIndex;
    bool bFound = false;
    QString casestr;

    if (!bFound)
    {
        // to lower case.
        casestr = sWord.toLower();
        if (casestr != sWord)
        {
            if (oLib[iLib]->lookup(casestr, iIndex))
                bFound = true;
        }

        // to upper case.
        if (!bFound)
        {
            casestr = sWord.toUpper();
            if (casestr !=  sWord)
            {
                if (oLib[iLib]->lookup(casestr, iIndex))
                    bFound = true;
            }

        }

        // Upper the first character and lower others.
        if (!bFound)
        {
            casestr = sWord;
            casestr = casestr.toLower();
            casestr[0].toUpper();
            if (casestr != sWord)
            {
                if (oLib[iLib]->lookup(casestr, iIndex))
                    bFound = true;
            }

        }
    }

    /*
    TODO
    if (bIsPureEnglish(sWord))
    {
    // If not Found , try other status of sWord.
    int iWordLen = strlen(sWord);
    bool isupcase;

    char *sNewWord = (char *)g_malloc(iWordLen + 1);

    //cut one char "s" or "d"
    if (!bFound && iWordLen > 1)
    {
    isupcase = sWord[iWordLen - 1] == 'S' || !strncmp(&sWord[iWordLen - 2], "ED", 2);
    if (isupcase || sWord[iWordLen - 1] == 's' || !strncmp(&sWord[iWordLen - 2], "ed", 2))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 1] = '\0'; // cut "s" or "d"
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }

    //cut "ly"
    if (!bFound && iWordLen > 2)
    {
    isupcase = !strncmp(&sWord[iWordLen - 2], "LY", 2);
    if (isupcase || (!strncmp(&sWord[iWordLen - 2], "ly", 2)))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 2] = '\0';  // cut "ly"
    if (iWordLen > 5 && sNewWord[iWordLen - 3] == sNewWord[iWordLen - 4]
    && !bIsVowel(sNewWord[iWordLen - 4]) &&
    bIsVowel(sNewWord[iWordLen - 5]))
    { //doubled

    sNewWord[iWordLen - 3] = '\0';
    if ( oLib[iLib]->Lookup(sNewWord, iIndex) )
    bFound = true;
    else
    {
    if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    if (!bFound)
    sNewWord[iWordLen - 3] = sNewWord[iWordLen - 4];  //restore
    }
    }
    if (!bFound)
    {
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }
    }

    //cut "ing"
    if (!bFound && iWordLen > 3)
    {
    isupcase = !strncmp(&sWord[iWordLen - 3], "ING", 3);
    if (isupcase || !strncmp(&sWord[iWordLen - 3], "ing", 3) )
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 3] = '\0';
    if ( iWordLen > 6 && (sNewWord[iWordLen - 4] == sNewWord[iWordLen - 5])
    && !bIsVowel(sNewWord[iWordLen - 5]) &&
    bIsVowel(sNewWord[iWordLen - 6]))
    {  //doubled
    sNewWord[iWordLen - 4] = '\0';
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else
    {
    if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    if (!bFound)
    sNewWord[iWordLen - 4] = sNewWord[iWordLen - 5];  //restore
    }
    }
    if ( !bFound )
    {
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    if (!bFound)
    {
    if (isupcase)
    strcat(sNewWord, "E"); // add a char "E"
    else
    strcat(sNewWord, "e"); // add a char "e"
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }
    }

    //cut two char "es"
    if (!bFound && iWordLen > 3)
    {
    isupcase = (!strncmp(&sWord[iWordLen - 2], "ES", 2) &&
    (sWord[iWordLen - 3] == 'S' ||
    sWord[iWordLen - 3] == 'X' ||
    sWord[iWordLen - 3] == 'O' ||
    (iWordLen > 4 && sWord[iWordLen - 3] == 'H' &&
    (sWord[iWordLen - 4] == 'C' ||
    sWord[iWordLen - 4] == 'S'))));
    if (isupcase ||
    (!strncmp(&sWord[iWordLen - 2], "es", 2) &&
    (sWord[iWordLen - 3] == 's' || sWord[iWordLen - 3] == 'x' ||
    sWord[iWordLen - 3] == 'o' ||
    (iWordLen > 4 && sWord[iWordLen - 3] == 'h' &&
    (sWord[iWordLen - 4] == 'c' || sWord[iWordLen - 4] == 's')))))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 2] = '\0';
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }

    //cut "ed"
    if (!bFound && iWordLen > 3)
    {
    isupcase = !strncmp(&sWord[iWordLen - 2], "ED", 2);
    if (isupcase || !strncmp(&sWord[iWordLen - 2], "ed", 2))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 2] = '\0';
    if (iWordLen > 5 && (sNewWord[iWordLen - 3] == sNewWord[iWordLen - 4])
    && !bIsVowel(sNewWord[iWordLen - 4]) &&
    bIsVowel(sNewWord[iWordLen - 5]))
    { //doubled
    sNewWord[iWordLen - 3] = '\0';
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else
    {
    if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    if (!bFound)
    sNewWord[iWordLen - 3] = sNewWord[iWordLen - 4];  //restore
    }
    }
    if (!bFound)
    {
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }
    }

    // cut "ied" , add "y".
    if (!bFound && iWordLen > 3)
    {
    isupcase = !strncmp(&sWord[iWordLen - 3], "IED", 3);
    if (isupcase || (!strncmp(&sWord[iWordLen - 3], "ied", 3)))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 3] = '\0';
    if (isupcase)
    strcat(sNewWord, "Y"); // add a char "Y"
    else
    strcat(sNewWord, "y"); // add a char "y"
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }

    // cut "ies" , add "y".
    if (!bFound && iWordLen > 3)
    {
    isupcase = !strncmp(&sWord[iWordLen - 3], "IES", 3);
    if (isupcase || (!strncmp(&sWord[iWordLen - 3], "ies", 3)))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 3] = '\0';
    if (isupcase)
    strcat(sNewWord, "Y"); // add a char "Y"
    else
    strcat(sNewWord, "y"); // add a char "y"
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }

    // cut "er".
    if (!bFound && iWordLen > 2)
    {
    isupcase = !strncmp(&sWord[iWordLen - 2], "ER", 2);
    if (isupcase || (!strncmp(&sWord[iWordLen - 2], "er", 2)))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 2] = '\0';
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }

    // cut "est".
    if (!bFound && iWordLen > 3)
    {
    isupcase = !strncmp(&sWord[iWordLen - 3], "EST", 3);
    if (isupcase || (!strncmp(&sWord[iWordLen - 3], "est", 3)))
    {
    strcpy(sNewWord, sWord);
    sNewWord[iWordLen - 3] = '\0';
    if (oLib[iLib]->Lookup(sNewWord, iIndex))
    bFound = true;
    else if (isupcase || g_ascii_isupper(sWord[0]))
    {
    casestr = g_ascii_strdown(sNewWord, -1);
    if (strcmp(casestr, sNewWord))
    {
    if (oLib[iLib]->Lookup(casestr, iIndex))
    bFound = true;
    }
    g_free(casestr);
    }
    }
    }

    g_free(sNewWord);
    }
    */

    if (bFound)
        iWordIndex = iIndex;
#if 0

    else
    {
        //don't change iWordIndex here.
        //when LookupSimilarWord all failed too, we want to use the old LookupWord index to list words.
        //iWordIndex = INVALID_INDEX;
    }
#endif
    return bFound;
}

bool Libs::SimpleLookupWord(const QString & sWord, long & iWordIndex, int iLib)
{
    bool bFound = oLib[iLib]->lookup(sWord, iWordIndex);
    if (!bFound)
        bFound = LookupSimilarWord(sWord, iWordIndex, iLib);
    return bFound;
}

struct Fuzzystruct
{
    char * pMatchWord;
    int iMatchWordDistance;
};

inline bool operator<(const Fuzzystruct & lh, const Fuzzystruct & rh)
{
    if (lh.iMatchWordDistance != rh.iMatchWordDistance)
        return lh.iMatchWordDistance < rh.iMatchWordDistance;

    if (lh.pMatchWord && rh.pMatchWord)
        return stardict_strcmp(lh.pMatchWord, rh.pMatchWord) < 0;

    return false;
}


bool Libs::LookupWithFuzzy(const QString &sWord, QStringList &reslist, int iLib)
{
    if (sWord.isEmpty())
    {
        return false;
    }
    return false;

    /*
    Fuzzystruct *oFuzzystruct = new Fuzzystruct[reslist_size];

    for (int i = 0; i < reslist_size; i++)
    {
    oFuzzystruct[i].pMatchWord = NULL;
    oFuzzystruct[i].iMatchWordDistance = iMaxFuzzyDistance;
    }
    int iMaxDistance = iMaxFuzzyDistance;
    int iDistance;
    bool Found = false;
    EditDistance oEditDistance;

    long iCheckWordLen;
    const char *sCheck;
    gunichar *ucs4_str1, *ucs4_str2;
    long ucs4_str2_len;

    ucs4_str2 = g_utf8_to_ucs4_fast(sWord, -1, &ucs4_str2_len);
    unicode_strdown(ucs4_str2);

    //    for (std::vector<Dict *>::size_type iLib = 0; iLib<oLib.size(); iLib++)
    //    {
    if (progress_func)
    progress_func();

    //if (stardict_strcmp(sWord, poGetWord(0,iLib))>=0 &&
    stardict_strcmp(sWord, poGetWord(narticles(iLib)-1,iLib))<=0) {
    //there are Chinese dicts and English dicts...
    if (TRUE)
    {
    const int iwords = narticles(iLib);
    for (int index = 0; index < iwords; index++)
    {
    sCheck = poGetWord(index, iLib);
    // tolower and skip too long or too short words
    iCheckWordLen = g_utf8_strlen(sCheck, -1);
    if (iCheckWordLen - ucs4_str2_len >= iMaxDistance ||
    ucs4_str2_len - iCheckWordLen >= iMaxDistance)
    continue;
    ucs4_str1 = g_utf8_to_ucs4_fast(sCheck, -1, NULL);
    if (iCheckWordLen > ucs4_str2_len)
    ucs4_str1[ucs4_str2_len] = 0;
    unicode_strdown(ucs4_str1);

    iDistance = oEditDistance.CalEditDistance(ucs4_str1, ucs4_str2, iMaxDistance);
    g_free(ucs4_str1);
    if (iDistance < iMaxDistance && iDistance < ucs4_str2_len)
    {
    // when ucs4_str2_len=1,2 we need less fuzzy.
    Found = true;
    bool bAlreadyInList = false;
    int iMaxDistanceAt = 0;
    for (int j = 0; j < reslist_size; j++)
    {
    if (oFuzzystruct[j].pMatchWord &&
    strcmp(oFuzzystruct[j].pMatchWord, sCheck) == 0 )
    { //already in list
    bAlreadyInList = true;
    break;
    }
    //find the position,it will certainly be found (include the first time)
    .. as iMaxDistance is set by last time.
    if (oFuzzystruct[j].iMatchWordDistance == iMaxDistance )
    {
    iMaxDistanceAt = j;
    }
    }
    if (!bAlreadyInList)
    {
    if (oFuzzystruct[iMaxDistanceAt].pMatchWord)
    g_free(oFuzzystruct[iMaxDistanceAt].pMatchWord);
    oFuzzystruct[iMaxDistanceAt].pMatchWord = g_strdup(sCheck);
    oFuzzystruct[iMaxDistanceAt].iMatchWordDistance = iDistance;
    // calc new iMaxDistance
    iMaxDistance = iDistance;
    for (int j = 0; j < reslist_size; j++)
    {
    if (oFuzzystruct[j].iMatchWordDistance > iMaxDistance)
    iMaxDistance = oFuzzystruct[j].iMatchWordDistance;
    } // calc new iMaxDistance
    }   // add to list
    }   // find one
    }   // each word
    }   // ok for search
    //    }   // each lib
    g_free(ucs4_str2);

    if (Found) // sort with distance
    std::sort(oFuzzystruct, oFuzzystruct + reslist_size);

    for (int i = 0; i < reslist_size; ++i)
    reslist[i] = oFuzzystruct[i].pMatchWord;

    delete[] oFuzzystruct;

    return Found;
    */
}

inline bool less_for_compare(const char *lh, const char *rh)
{
    return stardict_strcmp(lh, rh) < 0;
}


int Libs::LookupWithRule(const QString & word, QStringList & ppMatchWord)
{
    /*
    long aiIndex[MAX_MATCH_ITEM_PER_LIB + 1];
    int iMatchCount = 0;
    GPatternSpec *pspec = g_pattern_spec_new(word);

    for (std::vector<Dict *>::size_type iLib = 0; iLib<oLib.size(); iLib++)
    {
    //if(oLibs.LookdupWordsWithRule(pspec,aiIndex,MAX_MATCH_ITEM_PER_LIB+1-iMatchCount,iLib))
    // -iMatchCount,so save time,but may got less result and the word may repeat.

    if (oLib[iLib]->
    LookupWithRule(pspec, aiIndex, MAX_MATCH_ITEM_PER_LIB + 1))
    {
    if (progress_func)
    progress_func();
    for (int i = 0; aiIndex[i] != -1; i++)
    {
    const char * sMatchWord = poGetWord(aiIndex[i], iLib);
    bool bAlreadyInList = false;
    for (int j = 0; j < iMatchCount; j++)
    {
    if (strcmp(ppMatchWord[j], sMatchWord) == 0)
    { //already in list
    bAlreadyInList = true;
    break;
    }
    }
    if (!bAlreadyInList)
    ppMatchWord[iMatchCount++] = g_strdup(sMatchWord);
    }
    }
    }
    g_pattern_spec_free(pspec);

    if (iMatchCount) // sort it.
    std::sort(ppMatchWord, ppMatchWord + iMatchCount, less_for_compare);

    return iMatchCount;
    */
    return 0;
}

bool Libs::LookupData(const QString &sWord, QStringList &reslist)
{
    if (sWord.isEmpty())
    {
        return false;
    }

    std::vector<std::string> SearchWords;
    SearchWords.push_back(sWord.toUtf8().data());

    quint32 max_size = 0;
    QByteArray origin_data;
    for (std::vector<Dict *>::size_type i = 0; i<oLib.size(); ++i)
    {
        if (!oLib[i]->containSearchData())
        {
            continue;
        }

        const ulong iwords = narticles(i);
        QString key;
        quint32 offset, size;
        for (ulong j = 0; j < iwords; ++j)
        {
            oLib[i]->keyAndData(j, key, offset, size);

            if (size > max_size)
            {
                origin_data.reserve(size);
                max_size = size;
            }

            if (oLib[i]->searchData(SearchWords, offset, size, origin_data))
            {
                reslist.append(key);
            }
        }
    }

    /*
    std::vector<Dict *>::size_type i;
    for (i = 0; i<oLib.size(); ++i)
    {

    if (!reslist.at(i).isEmpty())
    {
    break;
    }
    }
    return i != oLib.size();
    */
    return false;
}

/**************************************************/
query_t analyze_query(const char *s, QString& res)
{
    if (!s || !*s)
    {
        res = "";
        return qtSIMPLE;
    }
    if (*s == '/')
    {
        res = s + 1;
        return qtFUZZY;
    }

    if (*s == '|')
    {
        res = s + 1;
        return qtDATA;
    }

    bool regexp = false;
    const char *p = s;
    res = "";
    for (; *p; res += *p, ++p)
    {
        if (*p == '\\')
        {
            ++p;
            if (!*p)
                break;
            continue;
        }
        if (*p == '*' || *p == '?')
            regexp = true;
    }
    if (regexp)
        return qtREGEXP;

    return qtSIMPLE;
}

}   // namespace stardict


#ifndef RSS_READER_CONFI_H_
#define RSS_READER_CONFI_H_


//-- Define some global settings
#define CONFIGFILE "NewsFlash.ini"
#define DATASTOREDIR "Cache"
//-- The size limit for data that is being cached in kByte
#define CACHELIMIT 20
//-- Maximum number of elements of CACHELIMIT size within the cache
#define CACHEBUCKETS 30
#define VERSION "0.92"

class MyConf
{
public:
    static QString getConfDir() 
    {
        return QDir::homePath ().append("/rss_reader_dir");
    }

    static QString getStoreDir() 
    {
        return "/media/flash/.rss_reader_dir";
    }
};

#endif

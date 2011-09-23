#ifndef PAUTIL_H
#define PAUTIL_H

#include <cstdlib>
#include <ctime>

#include <string>
#include <sstream>

namespace pdfanno {
class PAUtil {
public:
    static std::string getTimeStamp()
    {
        time_t t = time(0);
        tm *lt = localtime(&t);

        std::ostringstream ostr;
        ostr<<(lt->tm_year + 1900)<<"-"<<(lt->tm_mon+1)<<"-"<<lt->tm_mday<<"_"<<lt->tm_hour<<"-"<<lt->tm_min<<"-"<<lt->tm_sec;

        return ostr.str();
    }

    template<class T> static void internalSwap(T *a, T *b)
    {
        T tmp = *a;
        *a = *b;
        *b = tmp;
    }

    static std::string getSaveAsPath(const std::string &filePath)
    {
        std::string::size_type idx = filePath.find_last_of('.');
        if (idx == std::string::npos) {
            return filePath + "_" + PAUtil::getTimeStamp();
        }
        else {
            return filePath.substr(0, idx) + "_" + PAUtil::getTimeStamp() +
                    "." + filePath.substr(idx + 1, filePath.length() - idx - 1);
        }
    }
};

} //namespace

#endif // PAUTIL_H

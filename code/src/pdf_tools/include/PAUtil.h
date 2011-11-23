#ifndef PAUTIL_H
#define PAUTIL_H

#include <string>

namespace pdfanno {
class PAUtil {
public:
    // postfix will be added to pdf's title
    static std::string getMergeMarkAsPostfix();
    static std::string getTimeStamp();
    static std::string getSaveAsPath(const std::string &filePath);

    template<class T> static void internalSwap(T *a, T *b)
    {
        T tmp = *a;
        *a = *b;
        *b = tmp;
    }
};

} //namespace

#endif // PAUTIL_H

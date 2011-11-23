/*
 * PAUtil.cpp
 *
 *  Created on: 26 Sep, 2011
 *      Author: joy
 */

#include <cstdlib>
#include <ctime>

#include <string>
#include <sstream>

#include "../include/PAUtil.h"

using namespace pdfanno;

std::string PAUtil::getMergeMarkAsPostfix()
{
    return "Merged";
}

std::string PAUtil::getTimeStamp()
{
    time_t t = time(0);
    tm *lt = localtime(&t);

    std::ostringstream ostr;
    ostr<<(lt->tm_year + 1900)<<"-"<<(lt->tm_mon+1)<<"-"<<lt->tm_mday<<"_"<<lt->tm_hour<<"-"<<lt->tm_min<<"-"<<lt->tm_sec;

    return ostr.str();
}

std::string PAUtil::getSaveAsPath(const std::string &filePath)
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

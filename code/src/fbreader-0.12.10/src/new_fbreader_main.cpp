#include <QtGui/QtGui>
#include <ZLibrary.h>
#include "FBReader.h"

int main(int argc, char* argv[])
{
    if (!ZLibrary::init(argc, argv))
    {
        return 1;
    }
    ZLibrary::run(new FBReader(argc == 1 ? std::string() : argv[1]));
    ZLibrary::shutdown();
    return 0;
}

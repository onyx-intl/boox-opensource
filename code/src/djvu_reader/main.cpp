#include "djvu_application.h"

using namespace djvu_reader;

int main(int argc, char* argv[])
{
    DjvuApplication app(argc,argv);
    DjvuApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(dictionary_images);

    if (app.open(app.currentPath()))
    {
        return app.exec();
    }
    return 0;
}

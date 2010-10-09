#include "onyx/base/base.h"
#include "dict_application.h"

using namespace ui;
using namespace dict_tool;

int main(int argc, char** argv)
{
    DictApplication app(argc,argv);
    DictApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(tts_images);

    if (app.open())
    {
        return app.exec();
    }
    return 0;
}

#include "player_application.h"

using namespace player;

int main(int argc, char** argv)
{
    PlayerApplication app(argc, argv);
    PlayerApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(music_player);
    Q_INIT_RESOURCE(onyx_ui_images);
    return app.exec();
}

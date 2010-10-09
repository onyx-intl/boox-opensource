#include <QtGui/QtGui>
#include <QString>
#include "dictionary_manager.h"
#include "dict_widget.h"

static const QString translationCSS =
    "body {\n"
        "font-size: 10pt; }\n"
    "font.dict_name {\n"
        "color: blue;\n"
        "font-style: italic; }\n"
    "font.title {\n"
        "font-size: 16pt;\n"
        "font-weight: bold; }\n"
    "font.explanation {\n"
        "color: #7f7f7f;\n"
        "font-style: italic; }\n"
    "font.abbreviature {\n"
        "font-style: italic; }\n"
    "font.example {\n"
        "font-style: italic; }\n"
    "font.transcription {\n"
        "font-weight: bold; }\n";

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        qCritical("Usage: %s <dictionary root dir> <word>", argv[0]);
        return -1;
    }

    QApplication app(argc, argv);

    DictionaryManager dicts(argv[1]);

    int num_dicts = dicts.loadDictionaries();
    qDebug("num_dicts = %d", num_dicts);

    ui::DictWidget dict_wnd(NULL, dicts);
    //dict_wnd.Lookup(argv[2]);
    dict_wnd.show();

    app.exec();
    return 0;
}

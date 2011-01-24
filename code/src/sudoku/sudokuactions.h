#ifndef SUDOKUACTIONS_H
#define SUDOKUACTIONS_H
#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"
using namespace ui;
namespace simsu{
enum SudokuActionsType{
    INVALID=-1,
    NEW=0,
    CHECK,
    ABOUT,
//    QUIT
};
class SudokuActions :public BaseActions
{

public:
    SudokuActions();
    ~SudokuActions(){}
    void generateActions();
    QAction * action(const SudokuActionsType action);
    SudokuActionsType selected();
};
}
#endif // SUDOKUACTIONS_H

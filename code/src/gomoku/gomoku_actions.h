#ifndef GOMOKU_ACTIONS_H
#define GOMOKU_ACTIONS_H

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

using namespace ui;

enum GomokuActionsType{INVALID=-1, NEW=0, ABOUT=1};

class GomokuActions :public BaseActions
{

public:
    GomokuActions();
    ~GomokuActions(){}
    void generateActions();
    QAction * action(const GomokuActionsType action);
    GomokuActionsType selected();
};

#endif // GOMOKU_ACTIONS_H

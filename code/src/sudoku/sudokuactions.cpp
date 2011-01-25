#include "sudokuactions.h"
namespace onyx{
namespace simsu{
SudokuActions::SudokuActions() : BaseActions()
{
    category()->setText(QCoreApplication::tr("Game"));
    category()->setIcon(QIcon(QPixmap(":/simsu.png")));
}

QAction* SudokuActions::action(const SudokuActionsType action)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_.at(i)->data().toInt() == action)
        {
            return actions_.at(i).get();
        }
    }
    return 0;
}

void SudokuActions::generateActions()
{

    actions_.clear();
    shared_ptr<QAction> newgame(new QAction(exclusiveGroup()));
    newgame->setCheckable(true);
    newgame->setText(QCoreApplication::tr("New"));
    newgame->setIcon(QIcon(QPixmap(":/none.png")));
    newgame->setData(NEW);
    actions_.push_back(newgame);

    shared_ptr<QAction> checkgame(new QAction(exclusiveGroup()));
    checkgame->setCheckable(true);
    checkgame->setText(QCoreApplication::tr("Check"));
    checkgame->setIcon(QIcon(QPixmap(":/random.png")));
    checkgame->setData(CHECK);
    actions_.push_back(checkgame);

    shared_ptr<QAction> aboutgame(new QAction(exclusiveGroup()));
    aboutgame->setCheckable(true);
    aboutgame->setText(QCoreApplication::tr("About"));
    aboutgame->setIcon(QIcon(QPixmap(":/images/about.png")));
    aboutgame->setData(ABOUT);
    actions_.push_back(aboutgame);
    //return_to_library for quit
}

SudokuActionsType SudokuActions::selected()
{
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<SudokuActionsType>(act->data().toInt());
    }
    return INVALID;
}
}
}
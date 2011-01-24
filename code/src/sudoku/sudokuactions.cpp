#include "sudokuactions.h"
namespace simsu{
SudokuActions::SudokuActions() : BaseActions()
{
    //TODO add icon
    category()->setIcon(QIcon(QPixmap(":/images/sudoku.png")));
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
    category()->setText(QCoreApplication::tr("Edit"));
    actions_.clear();
    shared_ptr<QAction> newgame(new QAction(exclusiveGroup()));
    newgame->setCheckable(true);
    newgame->setText(QCoreApplication::tr("New"));
    newgame->setIcon(QIcon(QPixmap(":/images/newgame.png")));
    newgame->setData(NEW);
    actions_.push_back(newgame);

    shared_ptr<QAction> checkgame(new QAction(exclusiveGroup()));
    checkgame->setCheckable(true);
    checkgame->setText(QCoreApplication::tr("Check"));
    checkgame->setIcon(QIcon(QPixmap(":/images/checkgame.png")));
    checkgame->setData(CHECK);
    actions_.push_back(checkgame);

    shared_ptr<QAction> aboutgame(new QAction(exclusiveGroup()));
    aboutgame->setCheckable(true);
    aboutgame->setText(QCoreApplication::tr("About"));
    aboutgame->setIcon(QIcon(QPixmap(":/images/checkgame.png")));
    aboutgame->setData(ABOUT);
    actions_.push_back(aboutgame);
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
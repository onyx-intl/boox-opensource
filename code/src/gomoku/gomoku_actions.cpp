#include "gomoku_actions.h"

GomokuActions::GomokuActions() : BaseActions()
{
    category()->setText(QCoreApplication::tr("Game"));
    category()->setIcon(QIcon(QPixmap(":/images/gomoku.png")));
}

QAction* GomokuActions::action(const GomokuActionsType action)
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

void GomokuActions::generateActions()
{

    actions_.clear();
    shared_ptr<QAction> newgame(new QAction(exclusiveGroup()));
    newgame->setCheckable(true);
    newgame->setText(QCoreApplication::tr("New"));
    newgame->setIcon(QIcon(QPixmap(":/images/newgame.png")));
    newgame->setData(NEW);
    actions_.push_back(newgame);

    shared_ptr<QAction> aboutgame(new QAction(exclusiveGroup()));
    aboutgame->setCheckable(true);
    aboutgame->setText(QCoreApplication::tr("About"));
    aboutgame->setIcon(QIcon(QPixmap(":/images/about.png")));
    aboutgame->setData(ABOUT);
    actions_.push_back(aboutgame);
}

GomokuActionsType GomokuActions::selected()
{
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<GomokuActionsType>(act->data().toInt());
    }
    return INVALID;
}

#include <stdio.h>
#include "advanced_actions.h"

namespace ui
{

AdvancedActions::AdvancedActions(void)
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/system.png")));
}

AdvancedActions::~AdvancedActions(void)
{
}

void AdvancedActions::generateActions(const vector<AdvancedType> & values, bool append)
{
    category()->setText(QCoreApplication::tr("Advanced"));
    if ( !append )
    {
        actions_.clear();
    }
    else
    {
        // add separator
        shared_ptr<QAction> separator(new QAction(exclusiveGroup()));
        separator->setSeparator(true);
        actions_.push_back(separator);
    }

    vector<AdvancedType>::const_iterator begin = values.begin();
    vector<AdvancedType>::const_iterator end   = values.end();
    for(vector<AdvancedType>::const_iterator iter = begin;
        iter != end; ++iter)
    {
        // Add to category automatically.
        shared_ptr<QAction> act(new QAction(exclusiveGroup()));

        // Change font and make it as checkable.
        act->setCheckable(true);
        act->setData(*iter);
        switch (*iter)
        {
        case SETTINGS:
            {
                act->setText(QCoreApplication::tr("Settings"));
                act->setIcon(QIcon(QPixmap(":/images/system.png")));
            }
            break;
        case INFO:
            {
                act->setText(QCoreApplication::tr("Information"));
                act->setIcon(QIcon(QPixmap(":/images/system.png")));
            }
            break;
        case RECENT_BOOKS:
            {
                act->setText(QCoreApplication::tr("Recent Books"));
                act->setIcon(QIcon(QPixmap(":/images/system.png")));
            }
            break;
        case ADD_CITATION:
            {
                act->setText(QCoreApplication::tr("Add Citation"));
                act->setIcon(QIcon(QPixmap(":/images/add_bookmark.png")));
            }
            break;
        case DELETE_CITE:
            {
                act->setText(QCoreApplication::tr("Delete Citation"));
                act->setIcon(QIcon(QPixmap(":/images/delete_bookmark.png")));
            }
            break;
        case SHOW_ALL_CITES:
            {
                act->setText(QCoreApplication::tr("Show Citations"));
                act->setIcon(QIcon(QPixmap(":/images/show_all_bookmarks.png")));
            }
            break;
        default:
            break;
        }
        actions_.push_back(act);
    }
}

void AdvancedActions::setActionStatus(const AdvancedType type,
                                          bool selected)
{
    assert(type > INVALID_TOOL && type < UNDEFINED_TOOL);

    QAction *act = 0;
    for (int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (type == actions_[i]->data().toInt())
        {
            act = actions_[i].get();
            break;
        }
    }

    if (act == 0)
    {
        return;
    }

    // The text.
    QString text;
    switch (type)
    {
    case SETTINGS:
        {
        }
        break;
    case INFO:
        {
        }
        break;
    case RECENT_BOOKS:
        {
        }
        break;
    default:
        break;
    }

    act->setText(text);
    act->setChecked(selected);
}

QAction * AdvancedActions::action(const AdvancedType type)
{
    for (int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (type == actions_[i]->data().toInt())
        {
            return actions_[i].get();
        }
    }
    return 0;
}


AdvancedType AdvancedActions::selectedTool()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<AdvancedType>(act->data().toInt());
    }
    return INVALID_SETTING;
}

}   // namespace ui

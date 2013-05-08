#ifndef ADVANCED_ACTIONS_H_
#define ADVANCED_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

namespace ui
{

enum AdvancedType
{
    INVALID_SETTING = -1,
    SETTINGS,
    RECENT_BOOKS,
    DELETE_CITE,
    SHOW_ALL_CITES,
    INFO,
    ADD_CITATION,
};


class AdvancedActions : public BaseActions
{
public:
    AdvancedActions(void);
    ~AdvancedActions(void);

public:
    /// Generate or re-generate the setting actions group.
    void generateActions(const vector<AdvancedType> & values, bool append = false);

    /// Set the status of reading tool
    void setActionStatus(const AdvancedType type, bool selected);

    /// Retrieve the action according to the type.
    QAction * action(const AdvancedType type);

    /// Retrieve the selected font size.
    AdvancedType selectedTool();

};  // AdvancedActions

}  // namespace ui

#endif

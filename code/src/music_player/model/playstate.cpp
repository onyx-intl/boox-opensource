#include "playstate.h"

namespace player
{

ShufflePlayState::ShufflePlayState(PlayListModel * model)
    : PlayState(model)
{
    prepare();
}

bool ShufflePlayState::next()
{
    int itm_count = model_->items().count();

    if (itm_count > 0)
    {
        if (shuffled_current_ >= shuffled_indexes_.count() -1 )
        {
            if (!model_->isRepeatableList())
            {
                return FALSE;
            }
            else
            {
                prepare();
            }
        }

        if (shuffled_current_ < shuffled_indexes_.count() - 1)
        {
            shuffled_current_++;
        }
        return model_->setCurrent(shuffled_indexes_.at(shuffled_current_));
    }
    return FALSE;
}

bool ShufflePlayState::previous()
{
    int itm_count = model_->items().count();

    if (itm_count > 0)
    {
        if (shuffled_current_ <= 0)
        {
            if (!model_->isRepeatableList())
            {
                return FALSE;
            }
            else
            {
                prepare();
                shuffled_current_ = shuffled_indexes_.count() - 1;
            }
        }

        if (itm_count > 1)
        {
            shuffled_current_--;
        }

        model_->setCurrent(shuffled_indexes_.at(shuffled_current_));
        return TRUE;
    }
    return FALSE;
}

void ShufflePlayState::prepare()
{
    resetState();
    for (int i = 0;i < model_->items().count();i++)
    {
        if (i != model_->currentRow())
        {
            shuffled_indexes_ << i;
        }
    }

    for (int i = 0;i < shuffled_indexes_.count();i++)
    {
        shuffled_indexes_.swap(qrand()%shuffled_indexes_.size(),qrand()%shuffled_indexes_.size());
    }

    shuffled_indexes_.prepend(model_->currentRow());
}

void ShufflePlayState::resetState()
{
    shuffled_indexes_.clear();
    shuffled_current_ = 0;
}

NormalPlayState::NormalPlayState(PlayListModel * model)
    : PlayState(model)
{
}

bool NormalPlayState::next()
{
    int itm_count = model_->items().count();

    if (itm_count > 0)
    {
        if ( model_->currentRow() == itm_count - 1)
        {
            if (model_->isRepeatableList())
            {
                return model_->setCurrent(0);
            }
            else
            {
                return FALSE;
            }
        }
        return model_->setCurrent(model_->currentRow() + 1);
    }

    return FALSE;
}

bool NormalPlayState::previous()
{
    int itm_count = model_->items().count();

    if (itm_count > 0)
    {
        if ( model_->currentRow() < 1 && !model_->isRepeatableList())
            return FALSE;
        else if (model_->setCurrent(model_->currentRow() - 1))
            return TRUE;
        else if (model_->isRepeatableList())
            return model_->setCurrent(model_->items().count() - 1);
    }

    return FALSE;
}

}

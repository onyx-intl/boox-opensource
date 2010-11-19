#ifndef PLAYER_PLAYSTATE_H_
#define PLAYER_PLAYSTATE_H_

#include "playlistmodel.h"

namespace player
{

/// Abstract class that represents data model playing states.
class PlayState
{
public:
    virtual bool next() = 0;
    virtual bool previous() = 0;
    virtual void resetState() {}
    virtual void prepare() {}
    virtual ~PlayState() {}

    PlayState(PlayListModel* model)
        : model_(model) {}

protected:
    /// Data model
    PlayListModel* model_;
};

class NormalPlayState : public PlayState
{
public:
    virtual bool next();
    virtual bool previous();
    NormalPlayState(PlayListModel* model);
};

class ShufflePlayState : public PlayState
{
public:
    virtual bool next();
    virtual bool previous();
    virtual void prepare();
    ShufflePlayState(PlayListModel* model);
    virtual void resetState();
private:
    /// Current shuffled index.
    int shuffled_current_;

    /// List of indexes used for shuffled playing.
    QList<int> shuffled_indexes_;
};

};

#endif

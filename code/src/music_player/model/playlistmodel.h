#ifndef PLAYER_PLAYLISTMODEL_H_
#define PLAYER_PLAYLISTMODEL_H_

#include <utils/player_utils.h>

namespace player
{

using namespace vbf;

class FileLoader;
class PlayListItem;
class PlayState;
class PlaylistFormat;
class PlayListModel;

/// Helper class that keeps track of a view's selected items.
struct SimpleSelection
{
    SimpleSelection()
    {
    }

    inline bool isValid()const
    {
        return (bottom_ != -1) && (anchor_ != -1) && (top_ != -1);
    }
    inline void dump()const
    {
        qWarning("top: %d\tbotom: %d\tanchor: %d",top_,bottom_,anchor_);
    }
    inline int count()const
    {
        return bottom_ - top_ + 1;
    }

    int        bottom_;         /*!<    */
    int        top_;            /*!<    */
    int        anchor_;         /*!<    */
    QList<int> selected_rows_;  /*!< Selected rows numbers */
};

/// Helper class used for tags update after details dialog closing.
class TagUpdater : public QObject
{
    Q_OBJECT
public:
    TagUpdater(QObject* o, PlayListItem* item);

protected Q_SLOTS:
    void updateTag();

private:
    QObject*      observable_;
    PlayListItem* item_;
};

/// The PlayListModel class provides a data model for the playlist.
class PlayListModel : public QObject
{
    Q_OBJECT
public:
    PlayListModel(QObject *parent = 0);
    ~PlayListModel();

    int count();
    PlayListItem* currentItem();
    int row(PlayListItem* item) const{ return items_.indexOf(item); }
    int queuedIndex(PlayListItem* item) const { return queued_songs_.indexOf(item); }
    QList<PlayListItem*> items()const { return items_; }
    int totalLength()const { return total_length_; }

    PlayListItem* item(int row) const;
    int currentRow();
    bool setCurrent (int row);
    bool isSelected(int row);
    void setSelected(int row, bool select);
    bool next();
    bool previous();
    QList <QString> getTitles(int first,int last);
    QList <QString> getTimes(int first,int last);
    void moveItems(int from, int to);
    bool isQueued(PlayListItem* item) const;
    void setCurrentToQueued();
    bool isEmptyQueue()const;

    const SimpleSelection& getSelection(int row);
    QList<int> getSelectedRows()const;
    QList<PlayListItem*> getSelectedItems()const;

    int firstSelectedUpper(int row);
    int firstSelectedLower(int row);
    bool isRepeatableList() const;
    bool isShuffle() const;

    QStandardItemModel* standardItemModel();

public:
    enum SortMode
    {
        TITLE,             /*!< by title */
        FILENAME,          /*!< by file name */
        PATH_AND_FILENAME, /*!< by path and file name */
        DATE,              /*!< by date */
        TRACK              /*!< by track */
    };

Q_SIGNALS:
    void listChanged();
    void currentChanged();
    void firstAdded();
    void repeatableListChanged(bool state);
    void shuffleChanged(bool state);
    void settingsChanged();
    void loadingFinished();

public Q_SLOTS:
    void load(PlayListItem *item);
    void clear();
    void clearSelection();
    void removeSelected();
    void removeUnselected();
    void removeAt (int i);
    void invertSelection();
    void selectAll();
    void showDetails();
    void doCurrentVisibleRequest();
    void addFile(const QString &path);
    void addFiles(const QStringList& l);
    void addDirectory(const QString& file_path);
    bool setFileList(const QStringList &l);
    void addFileList(const QStringList &l);
    void randomizeList();
    void reverseList();
    void prepareForShufflePlaying(bool yes);
    void prepareForRepeatablePlaying(bool yes);
    void sortSelection(int mode);
    void sort(int mode);
    void addToQueue();
    void setQueued(PlayListItem* f);
    bool convertUnderscore();
    bool convertTwenty();
    bool useMetadata();
    const QString format() const;
    void setConvertUnderscore(bool enabled);
    void setConvertTwenty(bool enabled);
    void setUseMetadata(bool enabled);
    void setFormat(const QString &format);

private:
    void doSort(int mode,QList<PlayListItem*>& list_to_sort);
    int topmostInSelection(int);
    int bottommostInSelection(int);
    FileLoader* createFileLoader();
    bool isFileLoaderRunning()const;
    void removeSelection(bool inverted = false);
    void readSettings();
    void writeSettings();
    void setUpdatesEnabled(bool);
    bool updatesEnabled()const{ return !block_update_signals_; }

private Q_SLOTS:
    void preparePlayState();

private:
    typedef QPointer<FileLoader> GuardedFileLoader;

private:
    QList<PlayListItem*>  items_;
    QList<PlayListItem*>  editing_items_;
    PlayListItem*         current_item_;
    int                   current_;
    QString               init_file_path_;

    SimpleSelection       selection_;
    QList<PlayListItem*>  queued_songs_;
    bool                  is_repeatable_list_;
    scoped_ptr<PlayState> play_state_;
    bool                  block_update_signals_;
    int                   total_length_;

    QVector<GuardedFileLoader> running_loaders_;
    bool                       shuffle_;

    scoped_ptr<QStandardItemModel> play_list_model_;
};

};

#endif

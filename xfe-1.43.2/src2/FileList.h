#ifndef FILELIST_H
#define FILELIST_H

#include "StringList.h"
#include "IconList.h"


struct FileAssoc;
class FileDict;
class FileList;

// File List options (prefixed by underscore to avoid conflict with the FOX library)
enum
{
    _FILELIST_SHOWHIDDEN   = 0x04000000, // Show hidden files or directories
    _FILELIST_SHOWDIRS     = 0x08000000, // Show only directories
    _FILELIST_SEARCH       = 0x10000000, // File list is a search list (must be the same value as in IconList)
};

// File item
class FXAPI FileItem : public IconItem
{
    FXDECLARE(FileItem)
    friend class FileList;
    friend class SearchPanel;
protected:
    FileAssoc* assoc;                       // File association record
    FileItem*  link;                        // Link to next item
    FXulong    size;                        // File size
    FXTime     date;                        // File date (mtime)
    FXTime     cdate;                       // Changed date (ctime)
    FXTime     deldate;                     // Deletion date
protected:
    FileItem() : assoc(NULL), link(NULL), size(0), date(0), cdate(0), deldate(0)
    {}
protected:
    enum
    {
        FOLDER     = 64,                        // Directory item
        EXECUTABLE = 128,                       // Executable item
        SYMLINK    = 256,                       // Symbolic linked item
        CHARDEV    = 512,                       // Character special item
        BLOCKDEV   = 1024,                      // Block special item
        FIFO       = 2048,                      // FIFO item
        SOCK       = 4096                       // Socket item
    };
public:
    // Constructor
    FileItem(const FXString& text, FXIcon* bi = NULL, FXIcon* mi = NULL, void* ptr = NULL) : IconItem(text, bi, mi, ptr), assoc(NULL),
                                                                                             link(NULL), size(0), date(0), cdate(0), deldate(0)
    {}

    // Return true if this is a file item
    FXbool isFile() const
    {
        return((state&(FOLDER|BLOCKDEV|CHARDEV|FIFO|SOCK)) == 0);
    }

    // Return true if this is a directory item
    FXbool isDirectory() const
    {
        return((state&FOLDER) != 0);
    }

    // Return true if this is an executable item
    FXbool isExecutable() const
    {
        return((state&EXECUTABLE) != 0);
    }

    // Return true if this is a symbolic link item
    FXbool isSymlink() const
    {
        return((state&SYMLINK) != 0);
    }

    // Return true if this is a character device item
    FXbool isChardev() const
    {
        return((state&CHARDEV) != 0);
    }

    // Return true if this is a block device item
    FXbool isBlockdev() const
    {
        return((state&BLOCKDEV) != 0);
    }

    // Return true if this is an FIFO item
    FXbool isFifo() const
    {
        return((state&FIFO) != 0);
    }

    // Return true if this is a socket
    FXbool isSocket() const
    {
        return((state&SOCK) != 0);
    }

    // Return the file-association object for this item
    FileAssoc* getAssoc() const
    {
        return(assoc);
    }

    // Return the file size for this item
    FXulong getSize() const
    {
        return(size);
    }

    // Return the date for this item
    FXTime getDate() const
    {
        return(date);
    }
};


// File List object
class FXAPI FileList : public IconList
{
    FXDECLARE(FileList)
protected:
    FileItem*    list;               // File item list
    int          prevIndex;
    FXString     directory;          // Current directory
    FXString     orgdirectory;       // Original directory
    FXString     dropdirectory;      // Drop directory
    FXDragAction dropaction;         // Drop action
    FXString     dragfiles;          // Dragged files
    FileDict*    associations;       // Association table
    FXString     pattern;            // Pattern of file names
    FXuint       matchmode;          // File wildcard match mode
    FXTime       timestamp;          // Time when last refreshed
    FXuint       counter;            // Refresh counter
    FXbool       allowrefresh;       // Allow or disallow periodic refresh
    FXbool       displaythumbnails;  // Display thumbnails
    FXString     trashfileslocation; // Location of the trash files directory
    FXString     trashinfolocation;  // Location of the trash info directory
    FXbool       dirsfirst;          // Sort directories first
    int          deldatesize;
    int          origpathsize;
    FXWindow*    focuswindow;          // Window used to test focus
public:
    StringList* backhist;              // Back history
    StringList* forwardhist;           // Forward history

protected:
    FileList() : list(NULL), prevIndex(0), dropaction(DRAG_MOVE), associations(NULL),
                 matchmode(0), timestamp(0), counter(0), allowrefresh(false), displaythumbnails(false), dirsfirst(false),
                 deldatesize(0), origpathsize(0), focuswindow(NULL), backhist(NULL), forwardhist(NULL)
    {}
    virtual IconItem* createItem(const FXString& text, FXIcon* big, FXIcon* mini, void* ptr);

    FXbool updateItems(FXbool);
    void   listItems(FXbool);

private:
    FileList(const FileList&);
    FileList& operator=(const FileList&);

public:
    long onCmdRefresh(FXObject*, FXSelector, void*);
    long onCmdRefreshTimer(FXObject*, FXSelector, void*);
    long onOpenTimer(FXObject*, FXSelector, void*);
    long onDNDEnter(FXObject*, FXSelector, void*);
    long onDNDLeave(FXObject*, FXSelector, void*);
    long onDNDMotion(FXObject*, FXSelector, void*);
    long onDNDDrop(FXObject*, FXSelector, void*);
    long onDNDRequest(FXObject*, FXSelector, void*);
    long onBeginDrag(FXObject*, FXSelector, void*);
    long onEndDrag(FXObject*, FXSelector, void*);
    long onDragged(FXObject*, FXSelector, void*);
    long onCmdDirectoryUp(FXObject*, FXSelector, void*);
    long onUpdDirectoryUp(FXObject*, FXSelector, void*);
    long onCmdSortByName(FXObject*, FXSelector, void*);
    long onCmdSortByDirName(FXObject*, FXSelector, void*);
    long onUpdSortByName(FXObject*, FXSelector, void*);
    long onUpdSortByDirName(FXObject*, FXSelector, void*);
    long onCmdSortByType(FXObject*, FXSelector, void*);
    long onUpdSortByType(FXObject*, FXSelector, void*);
    long onCmdSortBySize(FXObject*, FXSelector, void*);
    long onUpdSortBySize(FXObject*, FXSelector, void*);
    long onCmdSortByExt(FXObject*, FXSelector, void*);
    long onUpdSortByExt(FXObject*, FXSelector, void*);
    long onCmdSortByTime(FXObject*, FXSelector, void*);
    long onUpdSortByTime(FXObject*, FXSelector, void*);
    long onCmdSortByUser(FXObject*, FXSelector, void*);
    long onUpdSortByUser(FXObject*, FXSelector, void*);
    long onCmdSortByGroup(FXObject*, FXSelector, void*);
    long onUpdSortByGroup(FXObject*, FXSelector, void*);
    long onCmdSortByPerm(FXObject*, FXSelector, void*);
    long onUpdSortByPerm(FXObject*, FXSelector, void*);
    long onCmdSortByDeltime(FXObject*, FXSelector, void*);
    long onUpdSortByDeltime(FXObject*, FXSelector, void*);
    long onCmdSortByOrigpath(FXObject*, FXSelector, void*);
    long onUpdSortByOrigpath(FXObject*, FXSelector, void*);
    long onCmdSortReverse(FXObject*, FXSelector, void*);
    long onUpdSortReverse(FXObject*, FXSelector, void*);
    long onCmdSortCase(FXObject*, FXSelector, void*);
    long onUpdSortCase(FXObject*, FXSelector, void*);
    long onCmdSetPattern(FXObject*, FXSelector, void*);
    long onUpdSetPattern(FXObject*, FXSelector, void*);
    long onCmdToggleHidden(FXObject*, FXSelector, void*);
    long onUpdToggleHidden(FXObject*, FXSelector, void*);
    long onCmdShowHidden(FXObject*, FXSelector, void*);
    long onUpdShowHidden(FXObject*, FXSelector, void*);
    long onCmdHideHidden(FXObject*, FXSelector, void*);
    long onUpdHideHidden(FXObject*, FXSelector, void*);
    long onCmdHeader(FXObject*, FXSelector, void*);
    long onUpdHeader(FXObject*, FXSelector, void*);
    long onCmdToggleThumbnails(FXObject*, FXSelector, void*);
    long onUpdToggleThumbnails(FXObject* sender, FXSelector, void*);
    long onCmdDirsFirst(FXObject*, FXSelector, void*);
    long onUpdDirsFirst(FXObject*, FXSelector, void*);
    long onCmdDragCopy(FXObject* sender, FXSelector, void*);
    long onCmdDragMove(FXObject* sender, FXSelector, void*);
    long onCmdDragLink(FXObject* sender, FXSelector, void*);
    long onCmdDragReject(FXObject* sender, FXSelector, void*);
    long onUpdRefreshTimer(FXObject* sender, FXSelector, void*);
public:
    static int compare(const IconItem*, const IconItem*, FXbool, FXbool, FXbool, FXuint);
    static int ascending(const IconItem*, const IconItem*);
    static int descending(const IconItem*, const IconItem*);
    static int ascendingCase(const IconItem*, const IconItem*);
    static int descendingCase(const IconItem*, const IconItem*);
    static int ascendingDir(const IconItem*, const IconItem*);
    static int descendingDir(const IconItem*, const IconItem*);
    static int ascendingDirCase(const IconItem*, const IconItem*);
    static int descendingDirCase(const IconItem*, const IconItem*);
    static int ascendingType(const IconItem*, const IconItem*);
    static int descendingType(const IconItem*, const IconItem*);
    static int ascendingSize(const IconItem*, const IconItem*);
    static int descendingSize(const IconItem*, const IconItem*);
    static int ascendingExt(const IconItem*, const IconItem*);
    static int descendingExt(const IconItem*, const IconItem*);
    static int ascendingTime(const IconItem*, const IconItem*);
    static int descendingTime(const IconItem*, const IconItem*);
    static int ascendingUser(const IconItem*, const IconItem*);
    static int descendingUser(const IconItem*, const IconItem*);
    static int ascendingGroup(const IconItem*, const IconItem*);
    static int descendingGroup(const IconItem*, const IconItem*);
    static int ascendingPerm(const IconItem*, const IconItem*);
    static int descendingPerm(const IconItem*, const IconItem*);
    static int ascendingDeltime(const IconItem*, const IconItem*);
    static int descendingDeltime(const IconItem*, const IconItem*);
    static int ascendingOrigpath(const IconItem*, const IconItem*);
    static int descendingOrigpath(const IconItem*, const IconItem*);
    static int ascendingMix(const IconItem*, const IconItem*);
    static int descendingMix(const IconItem*, const IconItem*);
    static int ascendingCaseMix(const IconItem*, const IconItem*);
    static int descendingCaseMix(const IconItem*, const IconItem*);
    static int ascendingDirMix(const IconItem*, const IconItem*);
    static int descendingDirMix(const IconItem*, const IconItem*);
    static int ascendingDirCaseMix(const IconItem*, const IconItem*);
    static int descendingDirCaseMix(const IconItem*, const IconItem*);
    static int ascendingTypeMix(const IconItem*, const IconItem*);
    static int descendingTypeMix(const IconItem*, const IconItem*);
    static int ascendingSizeMix(const IconItem*, const IconItem*);
    static int descendingSizeMix(const IconItem*, const IconItem*);
    static int ascendingExtMix(const IconItem*, const IconItem*);
    static int descendingExtMix(const IconItem*, const IconItem*);
    static int ascendingTimeMix(const IconItem*, const IconItem*);
    static int descendingTimeMix(const IconItem*, const IconItem*);
    static int ascendingUserMix(const IconItem*, const IconItem*);
    static int descendingUserMix(const IconItem*, const IconItem*);
    static int ascendingGroupMix(const IconItem*, const IconItem*);
    static int descendingGroupMix(const IconItem*, const IconItem*);
    static int ascendingPermMix(const IconItem*, const IconItem*);
    static int descendingPermMix(const IconItem*, const IconItem*);
    static int ascendingDeltimeMix(const IconItem*, const IconItem*);
    static int descendingDeltimeMix(const IconItem*, const IconItem*);
    static int ascendingOrigpathMix(const IconItem*, const IconItem*);
    static int descendingOrigpathMix(const IconItem*, const IconItem*);

public:
    enum
    {
        // Note : the order of the 10 following sort IDs must be kept
        ID_SORT_BY_NAME=IconList::ID_LAST,
        ID_SORT_BY_SIZE,
        ID_SORT_BY_TYPE,
        ID_SORT_BY_EXT,
        ID_SORT_BY_TIME,
        ID_SORT_BY_USER,
        ID_SORT_BY_GROUP,
        ID_SORT_BY_PERM,
        ID_SORT_BY_ORIGPATH,
        ID_SORT_BY_DELTIME,
        ID_SORT_BY_DIRNAME,
        ID_SORT_REVERSE,
        ID_SORT_CASE,
        ID_DIRS_FIRST,
        ID_DIRECTORY_UP,
        ID_SET_PATTERN,
        ID_SET_DIRECTORY,
        ID_SHOW_HIDDEN,
        ID_HIDE_HIDDEN,
        ID_TOGGLE_HIDDEN,
        ID_TOGGLE_THUMBNAILS,
        ID_REFRESH_TIMER,
        ID_REFRESH,
        ID_OPEN_TIMER,
        ID_DRAG_COPY,
        ID_DRAG_MOVE,
        ID_DRAG_LINK,
        ID_DRAG_REJECT,
        ID_LAST
    };
public:

    // Construct a file list
    FileList(FXWindow* focuswin, FXComposite* p, FXObject* tgt = NULL, FXSelector sel = 0, FXbool showthumbs = false, FXuint opts = 0, int x = 0, int y = 0, int w = 0, int h = 0);

    // Create server-side resources
    virtual void create();

    // Scan the current directory and update the items if needed, or if force is true
    void scan(FXbool force = true);

    // Set current file
    void setCurrentFile(const FXString& file);

    // Return current file
    FXString getCurrentFile() const;

    // Set current directory
    void setDirectory(const FXString& path, const FXbool histupdate = true, FXString prevpath = "");

    // Return current directory
    FXString getDirectory() const
    {
        return(directory);
    }

    // Change wildcard matching pattern
    void setPattern(const FXString& ptrn);

    // Return wildcard pattern
    FXString getPattern() const
    {
        return(pattern);
    }

    // Return true if item is a directory
    FXbool isItemDirectory(int index) const;

    // Return true if item is a file
    FXbool isItemFile(int index) const;

    // Return true if item is executable
    FXbool isItemExecutable(int index) const;

    // Return true if item is a symbolic link
    FXbool isItemLink(int index) const;

    // Get number of selected items
    int getNumSelectedItems(void) const
    {
        int num = 0;

        for (int u = 0; u < getNumItems(); u++)
        {
            if (isItemSelected(u))
            {
                num++;
            }
        }
        return(num);
    }

    // Get number of selected items and index of first selected item
    int getNumSelectedItems(int* index) const
    {
        int num = 0, itm = -1;

        for (int u = 0; u < getNumItems(); u++)
        {
            if (isItemSelected(u))
            {
                if (itm == -1)
                {
                    itm = u;
                }
                num++;
            }
        }
        (*index) = itm;
        return(num);
    }

    // Return name of item at index
    FXString getItemFilename(int index) const;

    // Get pathname from item at index, relatively to the current directory
    FXString getItemPathname(int index) const;

    // Get full pathname from item at index, as obtained from the label string
    FXString getItemFullPathname(int index) const;

    // Return file association of item
    FileAssoc* getItemAssoc(int index) const;

    // Return file size of the item
    FXulong getItemFileSize(int index) const;

    // Return wildcard matching mode
    FXuint getMatchMode() const
    {
        return(matchmode);
    }

    // Return directory first state for file name sorting
    FXbool getDirsFirst() const
    {
        return(dirsfirst);
    }

    // Set directory first state for file name sorting
    void setDirsFirst(const FXbool dfirst)
    {
        dirsfirst = dfirst;
    }

    int getHeaderSize(int index) const;

    void setHeaderSize(int index, int size);

    // Allow or disallow periodic refresh
    void setAllowRefresh(const FXbool allow);

    // Change wildcard matching mode
    void setMatchMode(FXuint mode);

    // Return true if showing hidden files
    FXbool shownHiddenFiles() const;

    // Show or hide hidden files
    void showHiddenFiles(FXbool showing);

    // Return true if displaying thumbnails
    FXbool shownThumbnails() const;

    // Display or not thumbnails
    void showThumbnails(FXbool display);

    // Return true if showing directories only
    FXbool showOnlyDirectories() const;

    // Show directories only
    void showOnlyDirectories(FXbool shown);

    // Change file associations
    void setAssociations(FileDict* assoc);

    // Return file associations
    FileDict* getAssociations() const
    {
        return(associations);
    }

#if defined(linux)
    // Force mtdevices list refresh
    void refreshMtdevices(void);

#endif

    // Destructor
    virtual ~FileList();
};

#endif

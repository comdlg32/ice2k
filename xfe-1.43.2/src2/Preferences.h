#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <string.h>

#include "DialogBox.h"
#include "Keybindings.h"


// Number of modifiable colors
#define NUM_COLORS    12

// Number of themes
#define NUM_THEMES    10


struct Theme
{
    //const char *name;
    FXString name;
    FXColor  color[NUM_COLORS];
    Theme()
    {
        name = "";
    }

    Theme(const char* n, FXColor base = 0, FXColor bdr = 0, FXColor bg = 0, FXColor fg = 0,
          FXColor selbg = 0, FXColor selfg = 0, FXColor listbg = 0, FXColor listfg = 0, FXColor listhl = 0, FXColor pbarfg = 0, FXColor attenfg = 0, FXColor scrollfg = 0)
    {
        name = FXString(n);
        color[0] = base;
        color[1] = bdr;
        color[2] = bg;
        color[3] = fg;
        color[4] = selbg;
        color[5] = selfg;
        color[6] = listbg;
        color[7] = listfg;
        color[8] = listhl;
        color[9] = pbarfg;
        color[10] = attenfg;
        color[11] = scrollfg;
    }

    FXbool operator !=(const Theme&);
};


class PreferencesBox : public DialogBox
{
    FXDECLARE(PreferencesBox)
private:
    FXComboBox*    colorsBox;
    FXComboBox*    themesBox;
    FXList*        themesList;
    FXTextField*   iconpath;
    FXTextField*   txtviewer;
    FXTextField*   txteditor;
    FXTextField*   filecomparator;
    FXTextField*   timeformat;
    FXTextField*   imgviewer;
    FXTextField*   xterm;
    FXTextField*   imgeditor;
    FXTextField*   archiver;
    FXTextField*   pdfviewer;
    FXTextField*   videoplayer;
    FXTextField*   audioplayer;
    FXTextField*   normalfont;
    FXTextField*   textfont;
    FXTextField*   mountcmd;
    FXTextField*   umountcmd;
    FXString       oldiconpath;
    FXString       oldtxtviewer;
    FXString       oldtxteditor;
    FXString       oldfilecomparator;
    FXString       oldtimeformat;
    FXString       oldimgviewer;
    FXString       oldxterm;
    FXString       oldnormalfont;
    FXString       oldtextfont;
    FXString       oldimgeditor;
    FXString       oldarchiver;
    FXString       oldpdfviewer;
    FXString       oldaudioplayer;
    FXString       oldvideoplayer;
    FXString       oldmountcmd;
    FXString       oldumountcmd;
    FXCheckButton* autosave;
    FXCheckButton* savewinpos;
    FXCheckButton* diropen;
    FXCheckButton* fileopen;
    FXCheckButton* filetooltips;
    FXCheckButton* relativeresize;
    FXCheckButton* showpathlink;
    FXCheckButton* rootmode;
    FXCheckButton* trashcan;
    FXCheckButton* trashbypass;
    FXCheckButton* dnd;
    FXCheckButton* trashmv;
    FXCheckButton* del;
    FXCheckButton* properties;
    FXCheckButton* del_emptydir;
    FXCheckButton* overwrite;
    FXCheckButton* exec;
    FXCheckButton* ask;
    FXCheckButton* bg;
    FXCheckButton* folder_warning;
    FXCheckButton* preserve_date_warning;
    FXCheckButton* root_warning;
    FXCheckButton* mount;
    FXCheckButton* show_mount;
    FXCheckButton* scroll;
    FXCheckButton* controls;
    FXDataTarget   startdirtarget;
    int            startdirmode;
    int            oldstartdirmode;
#ifdef STARTUP_NOTIFICATION
    FXCheckButton* usesn;
#endif
    FXColorWell* cwell;
    Theme        Themes[NUM_THEMES];
    Theme        currTheme;
    Theme        currTheme_prev;
    FXbool       use_sudo;
    FXbool       use_sudo_prev;
    FXbool       trashcan_prev;
    FXbool       trashbypass_prev;
    FXbool       autosave_prev;
    FXbool       savewinpos_prev;
    FXbool       diropen_prev;
    FXbool       fileopen_prev;
    FXbool       filetooltips_prev;
    FXbool       relativeresize_prev;
    FXbool       show_pathlink;
    FXbool       show_pathlink_prev;
    FXuint       wheellines_prev;
    FXint        scrollbarsize_prev;
    FXbool       ask_prev;
    FXbool       dnd_prev;
    FXbool       trashmv_prev;
    FXbool       del_prev;
    FXbool       properties_prev;
    FXbool       del_emptydir_prev;
    FXbool       overwrite_prev;
    FXbool       exec_prev;
    FXbool       use_clearlooks;
    FXbool       use_clearlooks_prev;
    FXbool       rootmode_prev;
#ifdef STARTUP_NOTIFICATION
    FXbool usesn_prev;
#endif
#if defined(linux)
    FXbool mount_prev;
    FXbool show_mount_prev;
#endif
    FXbool          root_warning_prev;
    FXbool          folder_warning_prev;
    FXbool          preserve_date_warning_prev;
    FXuint          themelist_prev;
    FXbool          smoothscroll_prev;
    KeybindingsBox* bindingsbox;
    FXStringDict*   glbBindingsDict;
    FXStringDict*   xfeBindingsDict;
    FXStringDict*   xfiBindingsDict;
    FXStringDict*   xfwBindingsDict;

    PreferencesBox() : colorsBox(NULL), themesBox(NULL), themesList(NULL), iconpath(NULL), txtviewer(NULL), txteditor(NULL),
                       filecomparator(NULL), timeformat(NULL), imgviewer(NULL), xterm(NULL), imgeditor(NULL), archiver(NULL),
                       pdfviewer(NULL), videoplayer(NULL), audioplayer(NULL), normalfont(NULL), textfont(NULL), autosave(NULL), savewinpos(NULL),
                       diropen(NULL), fileopen(NULL), filetooltips(NULL), relativeresize(NULL), showpathlink(NULL), rootmode(NULL), trashcan(NULL),
                       trashbypass(NULL), dnd(NULL), trashmv(NULL), del(NULL), properties(NULL), del_emptydir(NULL),
                       overwrite(NULL), exec(NULL), ask(NULL), bg(NULL), folder_warning(NULL), preserve_date_warning(NULL),
                       root_warning(NULL), mount(NULL), show_mount(NULL), scroll(NULL), controls(NULL), startdirmode(0), oldstartdirmode(0),
#ifdef STARTUP_NOTIFICATION
                       usesn(NULL),
#endif
                       cwell(NULL), use_sudo(false), use_sudo_prev(false), trashcan_prev(false), trashbypass_prev(false),
                       autosave_prev(false), savewinpos_prev(false), diropen_prev(false), fileopen_prev(false),
                       filetooltips_prev(false), relativeresize_prev(false), show_pathlink(false), show_pathlink_prev(false),
                       wheellines_prev(0), scrollbarsize_prev(0), ask_prev(false), dnd_prev(false), trashmv_prev(false), del_prev(false), properties_prev(false),
                       del_emptydir_prev(false), overwrite_prev(false), exec_prev(false),
                       use_clearlooks(false), use_clearlooks_prev(false), rootmode_prev(false),
#ifdef STARTUP_NOTIFICATION
                       usesn_prev(false),
#endif
#if defined(linux)
                       mount_prev(false), show_mount_prev(false),
#endif
                       root_warning_prev(false), folder_warning_prev(false), preserve_date_warning_prev(false),
                       themelist_prev(0), smoothscroll_prev(false), bindingsbox(NULL), glbBindingsDict(NULL),
                       xfeBindingsDict(NULL), xfiBindingsDict(NULL), xfwBindingsDict(NULL)
    {}

public:
    enum
    {
        ID_ACCEPT=DialogBox::ID_LAST,
        ID_CANCEL,
        ID_BROWSE_TXTVIEW,
        ID_BROWSE_TXTEDIT,
        ID_BROWSE_FILECOMP,
        ID_BROWSE_IMGVIEW,
        ID_BROWSE_ARCHIVER,
        ID_BROWSE_PDFVIEW,
        ID_BROWSE_VIDEOPLAY,
        ID_BROWSE_AUDIOPLAY,
        ID_BROWSE_XTERM,
        ID_BROWSE_MOUNTCMD,
        ID_BROWSE_UMOUNTCMD,
        ID_COLOR,
        ID_NORMALFONT,
        ID_TEXTFONT,
        ID_THEME,
        ID_BROWSE_ICON_PATH,
        ID_TRASH_BYPASS,
        ID_CONFIRM_TRASH,
        ID_CONFIRM_DEL_EMPTYDIR,
        ID_SU_CMD,
        ID_SUDO_CMD,
        ID_STANDARD_CONTROLS,
        ID_CLEARLOOKS_CONTROLS,
        ID_WHEELADJUST,
        ID_SCROLLBARSIZE,
        ID_SINGLE_CLICK_FILEOPEN,
        ID_FILE_TOOLTIPS,
        ID_RELATIVE_RESIZE,
        ID_SHOW_PATHLINK,
        ID_CHANGE_KEYBINDINGS,
        ID_RESTORE_KEYBINDINGS,
        ID_START_HOMEDIR,
        ID_START_CURRENTDIR,
        ID_START_LASTDIR,
        ID_LAST
    };

public:
    PreferencesBox(FXWindow* win, FXColor listbackcolor = FXRGB(255, 255, 255), FXColor listforecolor = FXRGB(0, 0, 0), FXColor highlightcolor = FXRGB(238, 238, 238), FXColor pbarcolor = FXRGB(0, 0, 255), FXColor attentioncolor = FXRGB(255, 0, 0), FXColor scrollbackcolor = FXRGB(237, 233, 227));
    long   onCmdAccept(FXObject*, FXSelector, void*);
    long   onCmdBrowse(FXObject*, FXSelector, void*);
    long   onCmdColor(FXObject*, FXSelector, void*);
    long   onUpdColor(FXObject*, FXSelector, void*);
    long   onCmdTheme(FXObject*, FXSelector, void*);
    long   onCmdBrowsePath(FXObject*, FXSelector, void*);
    long   onCmdNormalFont(FXObject*, FXSelector, void*);
    long   onCmdTextFont(FXObject*, FXSelector, void*);
    long   onUpdTrash(FXObject*, FXSelector, void*);
    long   onUpdConfirmDelEmptyDir(FXObject*, FXSelector, void*);
    long   onCmdSuMode(FXObject*, FXSelector, void*);
    long   onUpdSuMode(FXObject*, FXSelector, void*);
    long   onCmdWheelAdjust(FXObject*, FXSelector, void*);
    long   onUpdWheelAdjust(FXObject*, FXSelector, void*);
    long   onCmdScrollBarSize(FXObject*, FXSelector, void*);
    long   onUpdScrollBarSize(FXObject*, FXSelector, void*);
    long   onUpdSingleClickFileopen(FXObject*, FXSelector, void*);
    FXuint execute(FXuint);
    long   onCmdCancel(FXObject*, FXSelector, void*);
    long   onCmdControls(FXObject*, FXSelector, void*);
    long   onUpdControls(FXObject*, FXSelector, void*);
    long   onCmdChangeKeyBindings(FXObject*, FXSelector, void*);
    long   onCmdRestoreKeyBindings(FXObject*, FXSelector, void*);
    long   onCmdStartDir(FXObject*, FXSelector, void*);
    long   onUpdStartDir(FXObject*, FXSelector, void*);
};
#endif

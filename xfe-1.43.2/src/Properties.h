#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "TextLabel.h"
#include "DialogBox.h"

class PropertiesBox;

class PermFrame : public FXVerticalFrame
{
    FXDECLARE(PermFrame)
    friend class PropertiesBox;
private:
    FXCheckButton* ur;
    FXCheckButton* uw;
    FXCheckButton* ux;
    FXCheckButton* gr;
    FXCheckButton* gw;
    FXCheckButton* gx;
    FXCheckButton* or_;
    FXCheckButton* ow;
    FXCheckButton* ox;
    FXCheckButton* suid;
    FXCheckButton* sgid;
    FXCheckButton* svtx;
    FXDataTarget   cmd_radiotarget;
    FXDataTarget   flt_radiotarget;
    FXRadioButton* set;
    FXRadioButton* clear;
    FXRadioButton* dironly;
    FXRadioButton* fileonly;
    FXRadioButton* all;
    FXRadioButton* add;
    int            cmd;
    int            flt;
    FXCheckButton* rec;
    FXCheckButton* own;
    FXComboBox*    user;
    FXComboBox*    grp;

    PermFrame() : ur(NULL), uw(NULL), ux(NULL), gr(NULL), gw(NULL), gx(NULL), or_(NULL), ow(NULL), ox(NULL), suid(NULL), sgid(NULL), svtx(NULL),
                  set(NULL), clear(NULL), dironly(NULL), fileonly(NULL), all(NULL), add(NULL), cmd(0), flt(0), rec(NULL), own(NULL), user(NULL), grp(NULL)
    {}

public:
    PermFrame(FXComposite* parent, FXObject* target);
};

class PropertiesBox : public DialogBox
{
    FXDECLARE(PropertiesBox)

private:
    int     pid;                     // Proccess ID of child (valid if busy).
    int     pipes[2];                // Pipes to communicate with child process.

	FXuint 	     totalnbfiles;
	FXuint       totalnbsubdirs;
	FXulong      totaldirsize;
	int          nbseldirs;
    FXLabel*     fileSize;
    FXLabel*     fileSizeDetails;
    TextLabel*   location;
    FXLabel*     origlocation;
    FXLabel*     linkto;
    FXLabel*     deletiondate;
    FXTextField* ext;
    FXString*    files;
    FXString*    paths;
    FXLabel*     name_encoding;
    FXString     source;
    FXString     parentdir;
    FXString     filename;
    FXString     oldusr;
    FXString     oldgrp;
    FXString     descr_prev;
    FXString     open_prev;
    FXString     view_prev;
    FXString     edit_prev;
    FXString     bigic_prev;
    FXString     miniic_prev;
    int          num;
    FXString     trashfileslocation;
    FXString     trashinfolocation;
    FXbool       executable;
#ifdef STARTUP_NOTIFICATION
    FXCheckButton* snbutton;
    FXGroupBox*    sngroup;
    FXbool         sndisable_prev;
#endif
    FXTextField* input;
    FXTextField* username;
    FXTextField* grpname;
    FXTextField* open;
    FXTextField* view;
    FXTextField* edit;
    FXTextField* descr;
    FXTextField* bigic;
    FXButton*    bigicbtn;
    FXTextField* miniic;
    FXButton*    miniicbtn;
    FXbool       isDirectory;
    FXbool       isMountpoint;
    FXbool       recsize;
    mode_t       mode;
    mode_t       orig_mode;
    PermFrame*   perm;

    PropertiesBox() : totalnbfiles(0), totalnbsubdirs(0), totaldirsize(0), nbseldirs(0), fileSize(NULL), fileSizeDetails(NULL), location(NULL), origlocation(NULL), linkto(NULL),
                      deletiondate(NULL), ext(NULL), files(NULL), paths(NULL), name_encoding(NULL), num(0), executable(false),
#ifdef STARTUP_NOTIFICATION
                      snbutton(NULL), sngroup(NULL), sndisable_prev(false),
#endif
                      input(NULL), username(NULL), grpname(NULL), open(NULL), view(NULL), edit(NULL), descr(NULL), bigic(NULL), bigicbtn(NULL),
                      miniic(NULL), miniicbtn(NULL), isDirectory(false), isMountpoint(false), recsize(false), mode(0), orig_mode(0), perm(NULL)
    {}

public:
    enum
    {
        ID_ACCEPT_SINGLE=DialogBox::ID_LAST,
        ID_ACCEPT_MULT,
        ID_CANCEL,
        ID_SET,
        ID_CLEAR,
        ID_ADD,
        ID_DIRONLY,
        ID_FILEONLY,
        ID_SNDISABLE,
        ID_WATCHPROCESS,
        ID_ALL,
        ID_BIG_ICON,
        ID_MINI_ICON,
        ID_BROWSE_OPEN,
        ID_BROWSE_VIEW,
        ID_BROWSE_EDIT,
        ID_RUSR,
        ID_WUSR,
        ID_XUSR,
        ID_RGRP,
        ID_WGRP,
        ID_XGRP,
        ID_ROTH,
        ID_WOTH,
        ID_XOTH,
        ID_SUID,
        ID_SGID,
        ID_SVTX,
        ID_LAST
    };

public:
    virtual void create();

    PropertiesBox(FXWindow* win, FXString file, FXString path);
    PropertiesBox(FXWindow* win, FXString* file, int num, FXString* path);
    long onCmdAcceptSingle(FXObject*, FXSelector, void*);
    long onCmdAcceptMult(FXObject*, FXSelector, void*);
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onCmdCheck(FXObject*, FXSelector, void*);
    long onCmdCommand(FXObject*, FXSelector, void*);
    long onCmdFilter(FXObject*, FXSelector, void*);
    long onCmdBrowseIcon(FXObject*, FXSelector, void*);
    long onCmdBrowse(FXObject*, FXSelector, void*);
    long onUpdSizeAndPerm(FXObject*, FXSelector, void*);
    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onWatchProcess(FXObject*, FXSelector, void*);
#ifdef STARTUP_NOTIFICATION
    long onUpdSnDisable(FXObject*, FXSelector, void*);
#endif
};

#endif

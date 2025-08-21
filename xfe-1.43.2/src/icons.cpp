// Global icons for all applications

#include <iostream>
#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "icons.h"


// Icons (global variables)

FXIcon *archaddicon, *archexticon, *attribicon, *bigattribicon, *bigblockdevicon, *bigbrokenlinkicon, *bigcdromicon, *bigchardevicon;
FXIcon *bigcompareicon, *bigdocicon, *bigexecicon, *bigfileopenicon, *bigfiltericon, *bigfloppyicon, *bigfolderlockedicon;
FXIcon *bigfolderopenicon, *bigfoldericon, *bigfolderupicon, *bigharddiskicon, *bigiconsicon, *biglinkicon, *bignewfileicon;
FXIcon *bignewfoldericon, *bignewlinkicon, *bignfsdriveicon;
FXIcon *bignfsdriveumticon, *bigpipeicon, *bigsocketicon, *bigzipicon, *cdromicon, *charticon;
FXIcon *closefileicon, *clrbookicon, *collfoldericon, *copy_bigicon, *colltreeicon;
FXIcon *copy_clpicon, *cut_clpicon, *delete_big_permicon, *delete_bigicon, *deselicon, *detailsicon;
FXIcon *dirupicon, *editicon, *entericon, *errorbigicon, *exptreeicon, *compareicon;
FXIcon *filedelete_permicon, *filedeleteicon, *fileopenicon;
FXIcon *viewicon, *filtericon, *find_againicon, *fliplricon, *flipudicon, *floppyicon;
FXIcon *fontsicon, *gotobigicon, *gotodiricon, *gotolineicon, *harddiskicon, *helpicon, *hidehiddenicon;
FXIcon *hidenumbersicon, *hidethumbicon, *homeicon, *infobigicon, *invselicon, *link_bigicon;
FXIcon *locationicon, *lowercaseicon, *maphosticon, *miniappicon, *miniblockdevicon, *minibrokenlinkicon;
FXIcon *minichardevicon, *minidocicon, *miniexecicon, *minifolderclosedicon;
FXIcon *minifolderlockedicon, *minifolderopenicon, *minifoldericon, *minifolderupicon, *minilinkicon;
FXIcon *minipipeicon, *minishellicon, *minisocketicon;
FXIcon *move_bigicon, *moveiticon, *newfileicon, *newfoldericon, *nfsdriveicon, *nfsdriveumticon;
FXIcon *onepanelicon, *packageicon, *paste_clpicon, *prefsicon, *printbigicon, *printicon;
FXIcon *questionbigicon, *quiticon, *redoicon, *reloadicon, *renameiticon, *replaceicon;
FXIcon *reverticon, *rotatelefticon, *rotaterighticon, *runicon, *saveasicon, *savefileicon;
FXIcon *searchnexticon, *searchicon, *searchprevicon, *selallicon, *setbookicon, *shellicon;
FXIcon *showhiddenicon, *shownumbersicon, *showthumbicon, *smalliconsicon, *iconsmenuicon;
FXIcon *trash_full_bigicon, *trash_fullicon, *treeonepanelicon, *treetwopanelsicon, *twopanelsicon;
FXIcon *undoicon, *unmaphosticon, *uppercaseicon, *warningbigicon, *workicon, *wrapofficon, *wraponicon, *xfeicon, *realxfeicon, *xfiicon;
FXIcon *xfpicon, *xfwicon, *zipicon, *zoom100icon, *zoominicon, *zoomouticon, *zoomwinicon;
FXIcon *totrashicon, *dirbackicon, *dirforwardicon, *minixfeicon, *minixferooticon, *filedialogicon, *bigarchaddicon;
FXIcon *switchpanelsicon, *syncpanelsicon, *newlinkicon, *greenbuttonicon, *graybuttonicon, *closeicon, *throbicon, *throbanimicon, *webviewbgicon;
FXIcon *keybindingsicon, *minikeybindingsicon, *filerestoreicon, *restore_bigicon, *vertpanelsicon, *horzpanelsicon, *comboarrowicon;

FXIcon *tbarchaddicon, *tbarchexticon, *tbattribicon/*, *tbbigattribicon, *tbbigblockdevicon, *tbbigbrokenlinkicon, *tbbigcdromicon, *tbbigchardevicon */;
//FXIcon *tbbigcompareicon, *tbbigdocicon, *tbbigexecicon, *tbbigfileopenicon, *tbbigfiltericon, *tbbigfloppyicon, *tbbigfolderlockedicon;
FXIcon /**tbbigfolderopenicon, *tbbigfoldericon, *tbbigfolderupicon, *tbbigharddiskicon, */*tbbigiconsicon/*, *tbbiglinkicon, *tbbignewfileicon*/;
//FXIcon *tbbignewfoldericon, *tbbignewlinkicon, *tbbignfsdriveicon;
FXIcon /* *tbbignfsdriveumticon, *tbbigpipeicon, *tbbigsocketicon, *tbbigzipicon, */ *tbcdromicon, *tbcharticon, *webviewxp;
FXIcon *tbclosefileicon, *tbclrbookicon, *tbcollfoldericon, *tbcopy_bigicon, *tbcolltreeicon;
FXIcon *tbcopy_clpicon, *tbcut_clpicon, *tbdelete_big_permicon, *tbdelete_bigicon, *tbdeselicon, *tbdetailsicon;
FXIcon *tbdirupicon, *tbediticon, *tbentericon, *tberrorbigicon, *tbexptreeicon, *tbcompareicon;
FXIcon *tbfiledelete_permicon, *tbfiledeleteicon, *tbfileopenicon;
FXIcon *tbviewicon, *tbfiltericon, *tbfind_againicon, *tbfliplricon, *tbflipudicon, *tbfloppyicon;
FXIcon *tbfontsicon, *tbgotobigicon, *tbgotodiricon, *tbgotolineicon, *tbharddiskicon, *tbhelpicon, *tbhidehiddenicon;
FXIcon *tbhidenumbersicon, *tbhidethumbicon, *tbhomeicon, *tbinfobigicon, *tbinvselicon, *tblink_bigicon;
FXIcon *tblocationicon, *tblowercaseicon, *tbmaphosticon, *tbminiappicon, *tbminiblockdevicon, *tbminibrokenlinkicon;
FXIcon *tbminichardevicon, *tbminidocicon, *tbminiexecicon, *tbminifolderclosedicon;
FXIcon *tbminifolderlockedicon, *tbminifolderopenicon, *tbminifoldericon, *tbminifolderupicon, *tbminilinkicon;
FXIcon *tbminipipeicon, *tbminishellicon, *tbminisocketicon;
FXIcon *tbmove_bigicon, *tbmoveiticon, *tbnewfileicon, *tbnewfoldericon, *tbnfsdriveicon, *tbnfsdriveumticon;
FXIcon *tbonepanelicon, *tbpackageicon, *tbpaste_clpicon, *tbprefsicon, *tbprintbigicon, *tbprinticon;
FXIcon *tbquestionbigicon, *tbquiticon, *tbredoicon, *tbreloadicon, *tbrenameiticon, *tbreplaceicon;
FXIcon *tbreverticon, *tbrotatelefticon, *tbrotaterighticon, *tbrunicon, *tbsaveasicon, *tbsavefileicon;
FXIcon *tbsearchnexticon, *tbsearchicon, *tbsearchprevicon, *tbselallicon, *tbsetbookicon, *tbshellicon;
FXIcon *tbshowhiddenicon, *tbshownumbersicon, *tbshowthumbicon, *tbsmalliconsicon, *tbiconsmenuicon;
FXIcon *tbtrash_full_bigicon, *tbtrash_fullicon, *tbtreeonepanelicon, *tbtreetwopanelsicon, *tbtwopanelsicon;
FXIcon *tbundoicon, *tbunmaphosticon, *tbuppercaseicon, *tbwarningbigicon, *tbworkicon, *tbwrapofficon, *tbwraponicon, *tbxfeicon, *tbxfiicon;
FXIcon *tbxfpicon, *tbxfwicon, *tbzipicon, *tbzoom100icon, *tbzoominicon, *tbzoomouticon, *tbzoomwinicon;
FXIcon *tbtotrashicon, *tbdirbackicon, *tbdirforwardicon, *tbminixfeicon, *tbminixferooticon, *tbfiledialogicon/*, *tbbigarchaddicon*/;
FXIcon *tbswitchpanelsicon, *tbsyncpanelsicon, *tbnewlinkicon, *tbgreenbuttonicon, *tbgraybuttonicon, *tbcloseicon, *tbthrobicon, *tbthrobanimicon, *tbwebviewbgicon;
FXIcon *tbkeybindingsicon, *tbminikeybindingsicon, *tbfilerestoreicon, *tbrestore_bigicon, *tbvertpanelsicon, *tbhorzpanelsicon, *tbcomboarrowicon, *tbcomputericon;
FXIcon *resizecorner, *arrowicon;


FXImage* imgstipple;

const FXString ext = ".png";

// Load all application icons as global variables
FXbool loadAppIcons(FXApp* app)
{
    FXbool success = true;
    FXColor iconhilitecolor = app->getHiliteColor();
    FXColor iconshadowcolor = app->getShadowColor();
    FXColor iconforecolor = app->getForeColor();
    // FXColor iconforecolor = app->getForeColor();
    FXColor backcolor = app->getBackColor();
    FXColor basecolor = app->getBaseColor();

    // Icon path
    FXString iconpath = app->reg().readStringEntry("SETTINGS", "iconpath", DEFAULTICONPATH);

    // Load icons and set the success flag
    success = ((archaddicon = loadiconfile(app, iconpath, "archadd" + ext, backcolor)) != NULL) & success;
    success = ((archexticon = loadiconfile(app, iconpath, "archext" + ext, backcolor)) != NULL) & success;
    success = ((attribicon = loadiconfile(app, iconpath, "attrib" + ext, backcolor)) != NULL) & success;
    success = ((bigattribicon = loadiconfile(app, iconpath, "bigattrib" + ext, backcolor)) != NULL) & success;
    success = ((bigblockdevicon = loadiconfile(app, iconpath, "bigblockdev" + ext, backcolor)) != NULL) & success;
    success = ((bigbrokenlinkicon = loadiconfile(app, iconpath, "bigbrokenlink" + ext, backcolor)) != NULL) & success;
    success = ((bigcdromicon = loadiconfile(app, iconpath, "bigcdrom" + ext, backcolor)) != NULL) & success;
    success = ((bigchardevicon = loadiconfile(app, iconpath, "bigchardev" + ext, backcolor)) != NULL) & success;
    success = ((bigcompareicon = loadiconfile(app, iconpath, "bigcompare" + ext, backcolor)) != NULL) & success;
    success = ((bigdocicon = loadiconfile(app, iconpath, "bigdoc" + ext, backcolor)) != NULL) & success;
    success = ((bigexecicon = loadiconfile(app, iconpath, "bigexec" + ext, backcolor)) != NULL) & success;
    success = ((bigfileopenicon = loadiconfile(app, iconpath, "bigfileopen" + ext, backcolor)) != NULL) & success;
    success = ((bigfiltericon = loadiconfile(app, iconpath, "bigfilter" + ext, backcolor)) != NULL) & success;
    success = ((bigfloppyicon = loadiconfile(app, iconpath, "bigfloppy" + ext, backcolor)) != NULL) & success;
    success = ((bigfolderlockedicon = loadiconfile(app, iconpath, "bigfolderlocked" + ext, backcolor)) != NULL) & success;
    success = ((bigfolderopenicon = loadiconfile(app, iconpath, "bigfolderopen" + ext, backcolor)) != NULL) & success;
    success = ((bigfoldericon = loadiconfile(app, iconpath, "bigfolder" + ext, backcolor)) != NULL) & success;
    success = ((bigfolderupicon = loadiconfile(app, iconpath, "bigfolderup" + ext, backcolor)) != NULL) & success;
    success = ((bigharddiskicon = loadiconfile(app, iconpath, "bigharddisk" + ext, backcolor)) != NULL) & success;
    success = ((bigiconsicon = loadiconfile(app, iconpath, "bigicons" + ext, backcolor)) != NULL) & success;
    success = ((biglinkicon = loadiconfile(app, iconpath, "biglink" + ext, backcolor)) != NULL) & success;
    success = ((bignewfileicon = loadiconfile(app, iconpath, "bignewfile" + ext, backcolor)) != NULL) & success;
    success = ((bignewfoldericon = loadiconfile(app, iconpath, "bignewfolder" + ext, backcolor)) != NULL) & success;
    success = ((bignewlinkicon = loadiconfile(app, iconpath, "bignewlink" + ext, backcolor)) != NULL) & success;
    success = ((bignfsdriveicon = loadiconfile(app, iconpath, "bignfsdrive" + ext, backcolor)) != NULL) & success;
    success = ((bignfsdriveumticon = loadiconfile(app, iconpath, "bignfsdriveumt" + ext, backcolor)) != NULL) & success;
    success = ((bigpipeicon = loadiconfile(app, iconpath, "bigpipe" + ext, backcolor)) != NULL) & success;
    success = ((bigsocketicon = loadiconfile(app, iconpath, "bigsocket" + ext, backcolor)) != NULL) & success;
    success = ((bigzipicon = loadiconfile(app, iconpath, "bigzip" + ext, backcolor)) != NULL) & success;
    success = ((cdromicon = loadiconfile(app, iconpath, "cdrom" + ext, backcolor)) != NULL) & success;
    success = ((charticon = loadiconfile(app, iconpath, "chart" + ext, backcolor)) != NULL) & success;
    success = ((closefileicon = loadiconfile(app, iconpath, "closefile" + ext, backcolor)) != NULL) & success;
    success = ((clrbookicon = loadiconfile(app, iconpath, "clrbook" + ext, backcolor)) != NULL) & success;
    success = ((colltreeicon = loadiconfile(app, iconpath, "colltree" + ext, backcolor)) != NULL) & success;
    success = ((copy_bigicon = loadiconfile(app, iconpath, "copy_big" + ext, backcolor)) != NULL) & success;
    success = ((copy_clpicon = loadiconfile(app, iconpath, "copy_clp" + ext, backcolor)) != NULL) & success;
    success = ((cut_clpicon = loadiconfile(app, iconpath, "cut_clp" + ext, backcolor)) != NULL) & success;
    success = ((delete_big_permicon = loadiconfile(app, iconpath, "delete_big_perm" + ext, backcolor)) != NULL) & success;
    success = ((delete_bigicon = loadiconfile(app, iconpath, "delete_big" + ext, backcolor)) != NULL) & success;
    success = ((deselicon = loadiconfile(app, iconpath, "desel" + ext, backcolor)) != NULL) & success;
    success = ((detailsicon = loadiconfile(app, iconpath, "details" + ext, backcolor)) != NULL) & success;
    success = ((dirupicon = loadiconfile(app, iconpath, "dirup" + ext, backcolor)) != NULL) & success;
    success = ((editicon = loadiconfile(app, iconpath, "edit" + ext, backcolor)) != NULL) & success;
    success = ((entericon = loadiconfile(app, iconpath, "enter" + ext, backcolor)) != NULL) & success;
    success = ((errorbigicon = loadiconfile(app, iconpath, "errorbig" + ext, backcolor)) != NULL) & success;
    success = ((exptreeicon = loadiconfile(app, iconpath, "exptree" + ext, backcolor)) != NULL) & success;
    success = ((compareicon = loadiconfile(app, iconpath, "compare" + ext, backcolor)) != NULL) & success;
    success = ((filedelete_permicon = loadiconfile(app, iconpath, "filedelete_perm" + ext, backcolor)) != NULL) & success;
    success = ((filedeleteicon = loadiconfile(app, iconpath, "filedelete" + ext, backcolor)) != NULL) & success;
    success = ((fileopenicon = loadiconfile(app, iconpath, "fileopen" + ext, backcolor)) != NULL) & success;
    success = ((viewicon = loadiconfile(app, iconpath, "view" + ext, backcolor)) != NULL) & success;
    success = ((filtericon = loadiconfile(app, iconpath, "filter" + ext, backcolor)) != NULL) & success;
    success = ((find_againicon = loadiconfile(app, iconpath, "find_again" + ext, backcolor)) != NULL) & success;
    success = ((fliplricon = loadiconfile(app, iconpath, "fliplr" + ext, backcolor)) != NULL) & success;
    success = ((flipudicon = loadiconfile(app, iconpath, "flipud" + ext, backcolor)) != NULL) & success;
    success = ((floppyicon = loadiconfile(app, iconpath, "floppy" + ext, backcolor)) != NULL) & success;
    success = ((fontsicon = loadiconfile(app, iconpath, "fonts" + ext, backcolor)) != NULL) & success;
    success = ((gotobigicon = loadiconfile(app, iconpath, "gotobig" + ext, backcolor)) != NULL) & success;
    success = ((gotodiricon = loadiconfile(app, iconpath, "gotodir" + ext, backcolor)) != NULL) & success;
    success = ((gotolineicon = loadiconfile(app, iconpath, "gotoline" + ext, backcolor)) != NULL) & success;
    success = ((harddiskicon = loadiconfile(app, iconpath, "harddisk" + ext, backcolor)) != NULL) & success;
    success = ((helpicon = loadiconfile(app, iconpath, "help" + ext, backcolor)) != NULL) & success;
    success = ((hidehiddenicon = loadiconfile(app, iconpath, "hidehidden" + ext, backcolor)) != NULL) & success;
    success = ((hidenumbersicon = loadiconfile(app, iconpath, "hidenumbers" + ext, backcolor)) != NULL) & success;
    success = ((hidethumbicon = loadiconfile(app, iconpath, "hidethumb" + ext, backcolor)) != NULL) & success;
    success = ((homeicon = loadiconfile(app, iconpath, "home" + ext, backcolor)) != NULL) & success;
    success = ((infobigicon = loadiconfile(app, iconpath, "infobig" + ext, backcolor)) != NULL) & success;
    success = ((invselicon = loadiconfile(app, iconpath, "invsel" + ext, backcolor)) != NULL) & success;
    success = ((link_bigicon = loadiconfile(app, iconpath, "link_big" + ext, backcolor)) != NULL) & success;
    success = ((locationicon = loadiconfile(app, iconpath, "location" + ext, backcolor)) != NULL) & success;
    success = ((lowercaseicon = loadiconfile(app, iconpath, "lowercase" + ext, backcolor)) != NULL) & success;
    success = ((maphosticon = loadiconfile(app, iconpath, "maphost" + ext, backcolor)) != NULL) & success;
    success = ((miniappicon = loadiconfile(app, iconpath, "miniapp" + ext, backcolor)) != NULL) & success;
    success = ((miniblockdevicon = loadiconfile(app, iconpath, "miniblockdev" + ext, backcolor)) != NULL) & success;
    success = ((minibrokenlinkicon = loadiconfile(app, iconpath, "minibrokenlink" + ext, backcolor)) != NULL) & success;
    success = ((minichardevicon = loadiconfile(app, iconpath, "minichardev" + ext, backcolor)) != NULL) & success;
    success = ((minidocicon = loadiconfile(app, iconpath, "minidoc" + ext, backcolor)) != NULL) & success;
    success = ((miniexecicon = loadiconfile(app, iconpath, "miniexec" + ext, backcolor)) != NULL) & success;
    success = ((minifolderclosedicon = loadiconfile(app, iconpath, "minifolderclosed" + ext, backcolor)) != NULL) & success;
    success = ((minifolderlockedicon = loadiconfile(app, iconpath, "minifolderlocked" + ext, backcolor)) != NULL) & success;
    success = ((minifolderopenicon = loadiconfile(app, iconpath, "minifolderopen" + ext, backcolor)) != NULL) & success;
    success = ((minifoldericon = loadiconfile(app, iconpath, "minifolder" + ext, backcolor)) != NULL) & success;
    success = ((minifolderupicon = loadiconfile(app, iconpath, "minifolderup" + ext, backcolor)) != NULL) & success;
    success = ((minilinkicon = loadiconfile(app, iconpath, "minilink" + ext, backcolor)) != NULL) & success;
    success = ((minipipeicon = loadiconfile(app, iconpath, "minipipe" + ext, backcolor)) != NULL) & success;
    success = ((minishellicon = loadiconfile(app, iconpath, "minishell" + ext, backcolor)) != NULL) & success;
    success = ((minisocketicon = loadiconfile(app, iconpath, "minisocket" + ext, backcolor)) != NULL) & success;
    success = ((move_bigicon = loadiconfile(app, iconpath, "move_big" + ext, backcolor)) != NULL) & success;
    success = ((moveiticon = loadiconfile(app, iconpath, "moveit" + ext, backcolor)) != NULL) & success;
    success = ((newfileicon = loadiconfile(app, iconpath, "newfile" + ext, backcolor)) != NULL) & success;
    success = ((newfoldericon = loadiconfile(app, iconpath, "newfolder" + ext, backcolor)) != NULL) & success;
    success = ((nfsdriveicon = loadiconfile(app, iconpath, "nfsdrive" + ext, backcolor)) != NULL) & success;
    success = ((nfsdriveumticon = loadiconfile(app, iconpath, "nfsdriveumt" + ext, backcolor)) != NULL) & success;
    success = ((onepanelicon = loadiconfile(app, iconpath, "onepanel" + ext, backcolor)) != NULL) & success;
    success = ((packageicon = loadiconfile(app, iconpath, "package" + ext, backcolor)) != NULL) & success;
    success = ((paste_clpicon = loadiconfile(app, iconpath, "paste_clp" + ext, backcolor)) != NULL) & success;
    success = ((prefsicon = loadiconfile(app, iconpath, "prefs" + ext, backcolor)) != NULL) & success;
    success = ((printbigicon = loadiconfile(app, iconpath, "printbig" + ext, backcolor)) != NULL) & success;
    success = ((printicon = loadiconfile(app, iconpath, "print" + ext, backcolor)) != NULL) & success;
    success = ((questionbigicon = loadiconfile(app, iconpath, "questionbig" + ext, backcolor)) != NULL) & success;
    success = ((quiticon = loadiconfile(app, iconpath, "quit" + ext, backcolor)) != NULL) & success;
    success = ((redoicon = loadiconfile(app, iconpath, "redo" + ext, backcolor)) != NULL) & success;
    success = ((reloadicon = loadiconfile(app, iconpath, "reload" + ext, backcolor)) != NULL) & success;
    success = ((renameiticon = loadiconfile(app, iconpath, "renameit" + ext, backcolor)) != NULL) & success;
    success = ((replaceicon = loadiconfile(app, iconpath, "replace" + ext, backcolor)) != NULL) & success;
    success = ((reverticon = loadiconfile(app, iconpath, "revert" + ext, backcolor)) != NULL) & success;
    success = ((rotatelefticon = loadiconfile(app, iconpath, "rotateleft" + ext, backcolor)) != NULL) & success;
    success = ((rotaterighticon = loadiconfile(app, iconpath, "rotateright" + ext, backcolor)) != NULL) & success;
    success = ((runicon = loadiconfile(app, iconpath, "run" + ext, backcolor)) != NULL) & success;
    success = ((saveasicon = loadiconfile(app, iconpath, "saveas" + ext, backcolor)) != NULL) & success;
    success = ((savefileicon = loadiconfile(app, iconpath, "savefile" + ext, backcolor)) != NULL) & success;
    success = ((searchnexticon = loadiconfile(app, iconpath, "searchnext" + ext, backcolor)) != NULL) & success;
    success = ((searchicon = loadiconfile(app, iconpath, "search" + ext, backcolor)) != NULL) & success;
    success = ((searchprevicon = loadiconfile(app, iconpath, "searchprev" + ext, backcolor)) != NULL) & success;
    success = ((selallicon = loadiconfile(app, iconpath, "selall" + ext, backcolor)) != NULL) & success;
    success = ((setbookicon = loadiconfile(app, iconpath, "setbook" + ext, backcolor)) != NULL) & success;
    success = ((shellicon = loadiconfile(app, iconpath, "shell" + ext, backcolor)) != NULL) & success;
    success = ((showhiddenicon = loadiconfile(app, iconpath, "showhidden" + ext, backcolor)) != NULL) & success;
    success = ((shownumbersicon = loadiconfile(app, iconpath, "shownumbers" + ext, backcolor)) != NULL) & success;
    success = ((showthumbicon = loadiconfile(app, iconpath, "showthumb" + ext, backcolor)) != NULL) & success;
    success = ((smalliconsicon = loadiconfile(app, iconpath, "smallicons" + ext, backcolor)) != NULL) & success;
    success = ((iconsmenuicon = loadiconfile(app, iconpath, "iconsmenu" + ext, backcolor, 1)) != NULL) & success;
    success = ((trash_full_bigicon = loadiconfile(app, iconpath, "trash_full_big" + ext, backcolor)) != NULL) & success;
    success = ((trash_fullicon = loadiconfile(app, iconpath, "trash_full" + ext, backcolor)) != NULL) & success;
    success = ((treeonepanelicon = loadiconfile(app, iconpath, "treeonepanel" + ext, backcolor)) != NULL) & success;
    success = ((treetwopanelsicon = loadiconfile(app, iconpath, "treetwopanels" + ext, backcolor)) != NULL) & success;
    success = ((twopanelsicon = loadiconfile(app, iconpath, "twopanels" + ext, backcolor)) != NULL) & success;
    success = ((undoicon = loadiconfile(app, iconpath, "undo" + ext, backcolor)) != NULL) & success;
    success = ((unmaphosticon = loadiconfile(app, iconpath, "unmaphost" + ext, backcolor)) != NULL) & success;
    success = ((uppercaseicon = loadiconfile(app, iconpath, "uppercase" + ext, backcolor)) != NULL) & success;
    success = ((warningbigicon = loadiconfile(app, iconpath, "warningbig" + ext, backcolor)) != NULL) & success;
    success = ((workicon = loadiconfile(app, iconpath, "work" + ext, backcolor)) != NULL) & success;
    success = ((wrapofficon = loadiconfile(app, iconpath, "wrapoff" + ext, backcolor)) != NULL) & success;
    success = ((wraponicon = loadiconfile(app, iconpath, "wrapon" + ext, backcolor)) != NULL) & success;
    success = ((xfeicon = loadiconfile(app, iconpath, "xfe" + ext, backcolor)) != NULL) & success;
    success = ((realxfeicon = loadiconfile(app, iconpath, "xfe" + ext, 0)) != NULL) & success;
    success = ((xfiicon = loadiconfile(app, iconpath, "xfi" + ext, backcolor)) != NULL) & success;
    success = ((xfpicon = loadiconfile(app, iconpath, "xfp" + ext, backcolor)) != NULL) & success;
    success = ((xfwicon = loadiconfile(app, iconpath, "xfw" + ext, backcolor)) != NULL) & success;
    success = ((zipicon = loadiconfile(app, iconpath, "zip" + ext, backcolor)) != NULL) & success;
    success = ((zoom100icon = loadiconfile(app, iconpath, "zoom100" + ext, backcolor)) != NULL) & success;
    success = ((zoominicon = loadiconfile(app, iconpath, "zoomin" + ext, backcolor)) != NULL) & success;
    success = ((zoomouticon = loadiconfile(app, iconpath, "zoomout" + ext, backcolor)) != NULL) & success;
    success = ((zoomwinicon = loadiconfile(app, iconpath, "zoomwin" + ext, backcolor)) != NULL) & success;
    success = ((totrashicon = loadiconfile(app, iconpath, "totrash" + ext, backcolor)) != NULL) & success;
    success = ((dirbackicon = loadiconfile(app, iconpath, "dirback" + ext, backcolor)) != NULL) & success;
    success = ((dirforwardicon = loadiconfile(app, iconpath, "dirforward" + ext, backcolor)) != NULL) & success;
    success = ((minixferooticon = loadiconfile(app, iconpath, "minixferoot" + ext, backcolor)) != NULL) & success;
    success = ((minixfeicon = loadiconfile(app, iconpath, "minixfe" + ext, backcolor)) != NULL) & success;
    success = ((filedialogicon = loadiconfile(app, iconpath, "filedialog" + ext, backcolor)) != NULL) & success;
    success = ((bigarchaddicon = loadiconfile(app, iconpath, "bigarchadd" + ext, backcolor)) != NULL) & success;
    success = ((switchpanelsicon = loadiconfile(app, iconpath, "switchpanels" + ext, backcolor)) != NULL) & success;
    success = ((syncpanelsicon = loadiconfile(app, iconpath, "syncpanels" + ext, backcolor)) != NULL) & success;
    success = ((newlinkicon = loadiconfile(app, iconpath, "newlink" + ext, backcolor)) != NULL) & success;
    success = ((greenbuttonicon = loadiconfile(app, iconpath, "greenbutton" + ext, backcolor)) != NULL) & success;
    success = ((graybuttonicon = loadiconfile(app, iconpath, "graybutton" + ext, backcolor)) != NULL) & success;
    success = ((keybindingsicon = loadiconfile(app, iconpath, "keybindings" + ext, backcolor)) != NULL) & success;
    success = ((minikeybindingsicon = loadiconfile(app, iconpath, "minikeybindings" + ext, backcolor)) != NULL) & success;
    success = ((filerestoreicon = loadiconfile(app, iconpath, "filerestore" + ext, backcolor)) != NULL) & success;
    success = ((restore_bigicon = loadiconfile(app, iconpath, "restore_big" + ext, backcolor)) != NULL) & success;
    success = ((horzpanelsicon = loadiconfile(app, iconpath, "horzpanels" + ext, backcolor)) != NULL) & success;
    success = ((vertpanelsicon = loadiconfile(app, iconpath, "vertpanels" + ext, backcolor)) != NULL) & success;
    // success = ((comboarrowicon = loadiconfile(app, iconpath, "comboarrow" + ext, backcolor)) != NULL) & success;
    // success = ((closeicon = loadiconfile(app, iconpath, "close" + ext, backcolor)) != NULL) & success;
    success = ((throbicon = loadiconfile(app, iconpath, "throb" + ext, backcolor)) != NULL) & success;
    success = ((throbanimicon = loadiconfile(app, iconpath, "throbanim" + ext, backcolor)) != NULL) & success;
    success = ((webviewbgicon = loadiconfile(app, iconpath, "webviewbg" + ext, backcolor)) != NULL) ; success;

    success = ((tbarchaddicon = loadiconfile(app, iconpath, "archadd" + ext, basecolor)) != NULL) & success;
    success = ((tbarchexticon = loadiconfile(app, iconpath, "archext" + ext, basecolor)) != NULL) & success;
    success = ((tbattribicon = loadiconfile(app, iconpath, "attrib" + ext, basecolor)) != NULL) & success;
 /* success = ((tbbigattribicon = loadiconfile(app, iconpath, "bigattrib" + ext, basecolor)) != NULL) & success;
    success = ((tbbigblockdevicon = loadiconfile(app, iconpath, "bigblockdev" + ext, basecolor)) != NULL) & success;
    success = ((tbbigbrokenlinkicon = loadiconfile(app, iconpath, "bigbrokenlink" + ext, basecolor)) != NULL) & success;
    success = ((tbbigcdromicon = loadiconfile(app, iconpath, "bigcdrom" + ext, basecolor)) != NULL) & success;
    success = ((tbbigchardevicon = loadiconfile(app, iconpath, "bigchardev" + ext, basecolor)) != NULL) & success;
    success = ((tbbigcompareicon = loadiconfile(app, iconpath, "bigcompare" + ext, basecolor)) != NULL) & success;
    success = ((tbbigdocicon = loadiconfile(app, iconpath, "bigdoc" + ext, basecolor)) != NULL) & success;
    success = ((tbbigexecicon = loadiconfile(app, iconpath, "bigexec" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfileopenicon = loadiconfile(app, iconpath, "bigfileopen" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfiltericon = loadiconfile(app, iconpath, "bigfilter" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfloppyicon = loadiconfile(app, iconpath, "bigfloppy" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfolderlockedicon = loadiconfile(app, iconpath, "bigfolderlocked" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfolderopenicon = loadiconfile(app, iconpath, "bigfolderopen" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfoldericon = loadiconfile(app, iconpath, "bigfolder" + ext, basecolor)) != NULL) & success;
    success = ((tbbigfolderupicon = loadiconfile(app, iconpath, "bigfolderup" + ext, basecolor)) != NULL) & success;
    success = ((tbbigharddiskicon = loadiconfile(app, iconpath, "bigharddisk" + ext, basecolor)) != NULL) & success; */
    success = ((tbbigiconsicon = loadiconfile(app, iconpath, "bigicons" + ext, basecolor)) != NULL) & success; /*
    success = ((tbbiglinkicon = loadiconfile(app, iconpath, "biglink" + ext, basecolor)) != NULL) & success;
    success = ((tbbignewfileicon = loadiconfile(app, iconpath, "bignewfile" + ext, basecolor)) != NULL) & success;
    success = ((tbbignewfoldericon = loadiconfile(app, iconpath, "bignewfolder" + ext, basecolor)) != NULL) & success;
    success = ((tbbignewlinkicon = loadiconfile(app, iconpath, "bignewlink" + ext, basecolor)) != NULL) & success;
    success = ((tbbignfsdriveicon = loadiconfile(app, iconpath, "bignfsdrive" + ext, basecolor)) != NULL) & success;
    success = ((tbbignfsdriveumticon = loadiconfile(app, iconpath, "bignfsdriveumt" + ext, basecolor)) != NULL) & success;
    success = ((tbbigpipeicon = loadiconfile(app, iconpath, "bigpipe" + ext, basecolor)) != NULL) & success;
    success = ((tbbigsocketicon = loadiconfile(app, iconpath, "bigsocket" + ext, basecolor)) != NULL) & success;
    success = ((tbbigzipicon = loadiconfile(app, iconpath, "bigzip" + ext, basecolor)) != NULL) & success; */
    success = ((tbcdromicon = loadiconfile(app, iconpath, "cdrom" + ext, basecolor)) != NULL) & success;
    success = ((tbcharticon = loadiconfile(app, iconpath, "chart" + ext, basecolor)) != NULL) & success;
    success = ((tbclosefileicon = loadiconfile(app, iconpath, "closefile" + ext, basecolor)) != NULL) & success;
    success = ((tbclrbookicon = loadiconfile(app, iconpath, "clrbook" + ext, basecolor)) != NULL) & success;
    success = ((tbcolltreeicon = loadiconfile(app, iconpath, "colltree" + ext, basecolor)) != NULL) & success;
    success = ((tbcopy_bigicon = loadiconfile(app, iconpath, "copy_big" + ext, basecolor)) != NULL) & success;
    success = ((tbcopy_clpicon = loadiconfile(app, iconpath, "copy_clp" + ext, basecolor)) != NULL) & success;
    success = ((tbcut_clpicon = loadiconfile(app, iconpath, "cut_clp" + ext, basecolor)) != NULL) & success;
    success = ((tbdelete_big_permicon = loadiconfile(app, iconpath, "delete_big_perm" + ext, basecolor)) != NULL) & success;
    success = ((tbdelete_bigicon = loadiconfile(app, iconpath, "delete_big" + ext, basecolor)) != NULL) & success;
    success = ((tbdeselicon = loadiconfile(app, iconpath, "desel" + ext, basecolor)) != NULL) & success;
    success = ((tbdetailsicon = loadiconfile(app, iconpath, "details" + ext, basecolor)) != NULL) & success;
    success = ((tbdirupicon = loadiconfile(app, iconpath, "dirup" + ext, basecolor)) != NULL) & success;
    success = ((tbediticon = loadiconfile(app, iconpath, "edit" + ext, basecolor)) != NULL) & success;
    success = ((tbentericon = loadiconfile(app, iconpath, "enter" + ext, basecolor)) != NULL) & success;
    success = ((tberrorbigicon = loadiconfile(app, iconpath, "errorbig" + ext, basecolor)) != NULL) & success;
    success = ((tbexptreeicon = loadiconfile(app, iconpath, "exptree" + ext, basecolor)) != NULL) & success;
    success = ((tbcompareicon = loadiconfile(app, iconpath, "compare" + ext, basecolor)) != NULL) & success;
    success = ((tbfiledelete_permicon = loadiconfile(app, iconpath, "filedelete_perm" + ext, basecolor)) != NULL) & success;
    success = ((tbfiledeleteicon = loadiconfile(app, iconpath, "filedelete" + ext, basecolor)) != NULL) & success;
    success = ((tbfileopenicon = loadiconfile(app, iconpath, "fileopen" + ext, basecolor)) != NULL) & success;
    success = ((tbviewicon = loadiconfile(app, iconpath, "view" + ext, basecolor)) != NULL) & success;
    success = ((tbfiltericon = loadiconfile(app, iconpath, "filter" + ext, basecolor)) != NULL) & success;
    success = ((tbfind_againicon = loadiconfile(app, iconpath, "find_again" + ext, basecolor)) != NULL) & success;
    success = ((tbfliplricon = loadiconfile(app, iconpath, "fliplr" + ext, basecolor)) != NULL) & success;
    success = ((tbflipudicon = loadiconfile(app, iconpath, "flipud" + ext, basecolor)) != NULL) & success;
    success = ((tbfloppyicon = loadiconfile(app, iconpath, "floppy" + ext, basecolor)) != NULL) & success;
    success = ((tbfontsicon = loadiconfile(app, iconpath, "fonts" + ext, basecolor)) != NULL) & success;
    success = ((tbgotobigicon = loadiconfile(app, iconpath, "gotobig" + ext, basecolor)) != NULL) & success;
    success = ((tbgotodiricon = loadiconfile(app, iconpath, "gotodir" + ext, basecolor)) != NULL) & success;
    success = ((tbgotolineicon = loadiconfile(app, iconpath, "gotoline" + ext, basecolor)) != NULL) & success;
    success = ((tbharddiskicon = loadiconfile(app, iconpath, "harddisk" + ext, basecolor)) != NULL) & success;
    success = ((tbhelpicon = loadiconfile(app, iconpath, "help" + ext, basecolor)) != NULL) & success;
    success = ((tbhidehiddenicon = loadiconfile(app, iconpath, "hidehidden" + ext, basecolor)) != NULL) & success;
    success = ((tbhidenumbersicon = loadiconfile(app, iconpath, "hidenumbers" + ext, basecolor)) != NULL) & success;
    success = ((tbhidethumbicon = loadiconfile(app, iconpath, "hidethumb" + ext, basecolor)) != NULL) & success;
    success = ((tbhomeicon = loadiconfile(app, iconpath, "home" + ext, basecolor)) != NULL) & success;
    success = ((tbinfobigicon = loadiconfile(app, iconpath, "infobig" + ext, basecolor)) != NULL) & success;
    success = ((tbinvselicon = loadiconfile(app, iconpath, "invsel" + ext, basecolor)) != NULL) & success;
    success = ((tblink_bigicon = loadiconfile(app, iconpath, "link_big" + ext, basecolor)) != NULL) & success;
    success = ((tblocationicon = loadiconfile(app, iconpath, "location" + ext, basecolor)) != NULL) & success;
    success = ((tblowercaseicon = loadiconfile(app, iconpath, "lowercase" + ext, basecolor)) != NULL) & success;
    success = ((tbmaphosticon = loadiconfile(app, iconpath, "maphost" + ext, basecolor)) != NULL) & success;
    success = ((tbminiappicon = loadiconfile(app, iconpath, "miniapp" + ext, basecolor)) != NULL) & success;
    success = ((tbminiblockdevicon = loadiconfile(app, iconpath, "miniblockdev" + ext, basecolor)) != NULL) & success;
    success = ((tbminibrokenlinkicon = loadiconfile(app, iconpath, "minibrokenlink" + ext, basecolor)) != NULL) & success;
    success = ((tbminichardevicon = loadiconfile(app, iconpath, "minichardev" + ext, basecolor)) != NULL) & success;
    success = ((tbminidocicon = loadiconfile(app, iconpath, "minidoc" + ext, basecolor)) != NULL) & success;
    success = ((tbminiexecicon = loadiconfile(app, iconpath, "miniexec" + ext, basecolor)) != NULL) & success;
    success = ((tbminifolderclosedicon = loadiconfile(app, iconpath, "minifolderclosed" + ext, basecolor)) != NULL) & success;
    success = ((tbminifolderlockedicon = loadiconfile(app, iconpath, "minifolderlocked" + ext, basecolor)) != NULL) & success;
    success = ((tbminifolderopenicon = loadiconfile(app, iconpath, "minifolderopen" + ext, basecolor)) != NULL) & success;
    success = ((tbminifoldericon = loadiconfile(app, iconpath, "minifolder" + ext, basecolor)) != NULL) & success;
    success = ((tbminifolderupicon = loadiconfile(app, iconpath, "minifolderup" + ext, basecolor)) != NULL) & success;
    success = ((tbminilinkicon = loadiconfile(app, iconpath, "minilink" + ext, basecolor)) != NULL) & success;
    success = ((tbminipipeicon = loadiconfile(app, iconpath, "minipipe" + ext, basecolor)) != NULL) & success;
    success = ((tbminishellicon = loadiconfile(app, iconpath, "minishell" + ext, basecolor)) != NULL) & success;
    success = ((tbminisocketicon = loadiconfile(app, iconpath, "minisocket" + ext, basecolor)) != NULL) & success;
    success = ((tbmove_bigicon = loadiconfile(app, iconpath, "move_big" + ext, basecolor)) != NULL) & success;
    success = ((tbmoveiticon = loadiconfile(app, iconpath, "moveit" + ext, basecolor)) != NULL) & success;
    success = ((tbnewfileicon = loadiconfile(app, iconpath, "newfile" + ext, basecolor)) != NULL) & success;
    success = ((tbnewfoldericon = loadiconfile(app, iconpath, "newfolder" + ext, basecolor)) != NULL) & success;
    success = ((tbnfsdriveicon = loadiconfile(app, iconpath, "nfsdrive" + ext, basecolor)) != NULL) & success;
    success = ((tbnfsdriveumticon = loadiconfile(app, iconpath, "nfsdriveumt" + ext, basecolor)) != NULL) & success;
    success = ((tbonepanelicon = loadiconfile(app, iconpath, "onepanel" + ext, basecolor)) != NULL) & success;
    success = ((tbpackageicon = loadiconfile(app, iconpath, "package" + ext, basecolor)) != NULL) & success;
    success = ((tbpaste_clpicon = loadiconfile(app, iconpath, "paste_clp" + ext, basecolor)) != NULL) & success;
    success = ((tbprefsicon = loadiconfile(app, iconpath, "prefs" + ext, basecolor)) != NULL) & success;
    success = ((tbprintbigicon = loadiconfile(app, iconpath, "printbig" + ext, basecolor)) != NULL) & success;
    success = ((tbprinticon = loadiconfile(app, iconpath, "print" + ext, basecolor)) != NULL) & success;
    success = ((tbquestionbigicon = loadiconfile(app, iconpath, "questionbig" + ext, basecolor)) != NULL) & success;
    success = ((tbquiticon = loadiconfile(app, iconpath, "quit" + ext, basecolor)) != NULL) & success;
    success = ((tbredoicon = loadiconfile(app, iconpath, "redo" + ext, basecolor)) != NULL) & success;
    success = ((tbreloadicon = loadiconfile(app, iconpath, "reload" + ext, basecolor)) != NULL) & success;
    success = ((tbrenameiticon = loadiconfile(app, iconpath, "renameit" + ext, basecolor)) != NULL) & success;
    success = ((tbreplaceicon = loadiconfile(app, iconpath, "replace" + ext, basecolor)) != NULL) & success;
    success = ((tbreverticon = loadiconfile(app, iconpath, "revert" + ext, basecolor)) != NULL) & success;
    success = ((tbrotatelefticon = loadiconfile(app, iconpath, "rotateleft" + ext, basecolor)) != NULL) & success;
    success = ((tbrotaterighticon = loadiconfile(app, iconpath, "rotateright" + ext, basecolor)) != NULL) & success;
    success = ((tbrunicon = loadiconfile(app, iconpath, "run" + ext, basecolor)) != NULL) & success;
    success = ((tbsaveasicon = loadiconfile(app, iconpath, "saveas" + ext, basecolor)) != NULL) & success;
    success = ((tbsavefileicon = loadiconfile(app, iconpath, "savefile" + ext, basecolor)) != NULL) & success;
    success = ((tbsearchnexticon = loadiconfile(app, iconpath, "searchnext" + ext, basecolor)) != NULL) & success;
    success = ((tbsearchicon = loadiconfile(app, iconpath, "search" + ext, basecolor)) != NULL) & success;
    success = ((tbsearchprevicon = loadiconfile(app, iconpath, "searchprev" + ext, basecolor)) != NULL) & success;
    success = ((tbselallicon = loadiconfile(app, iconpath, "selall" + ext, basecolor)) != NULL) & success;
    success = ((tbsetbookicon = loadiconfile(app, iconpath, "setbook" + ext, basecolor)) != NULL) & success;
    success = ((tbshellicon = loadiconfile(app, iconpath, "shell" + ext, basecolor)) != NULL) & success;
    success = ((tbshowhiddenicon = loadiconfile(app, iconpath, "showhidden" + ext, basecolor)) != NULL) & success;
    success = ((tbshownumbersicon = loadiconfile(app, iconpath, "shownumbers" + ext, basecolor)) != NULL) & success;
    success = ((tbshowthumbicon = loadiconfile(app, iconpath, "showthumb" + ext, basecolor)) != NULL) & success;
    success = ((tbsmalliconsicon = loadiconfile(app, iconpath, "smallicons" + ext, basecolor)) != NULL) & success;
    success = ((tbiconsmenuicon = loadiconfile(app, iconpath, "iconsmenu" + ext, basecolor, 1)) != NULL) & success;
    success = ((tbtrash_full_bigicon = loadiconfile(app, iconpath, "trash_full_big" + ext, basecolor)) != NULL) & success;
    success = ((tbtrash_fullicon = loadiconfile(app, iconpath, "trash_full" + ext, basecolor)) != NULL) & success;
    success = ((tbtreeonepanelicon = loadiconfile(app, iconpath, "treeonepanel" + ext, basecolor)) != NULL) & success;
    success = ((tbtreetwopanelsicon = loadiconfile(app, iconpath, "treetwopanels" + ext, basecolor)) != NULL) & success;
    success = ((tbtwopanelsicon = loadiconfile(app, iconpath, "twopanels" + ext, basecolor)) != NULL) & success;
    success = ((tbundoicon = loadiconfile(app, iconpath, "undo" + ext, basecolor)) != NULL) & success;
    success = ((tbunmaphosticon = loadiconfile(app, iconpath, "unmaphost" + ext, basecolor)) != NULL) & success;
    success = ((tbuppercaseicon = loadiconfile(app, iconpath, "uppercase" + ext, basecolor)) != NULL) & success;
    success = ((tbwarningbigicon = loadiconfile(app, iconpath, "warningbig" + ext, basecolor)) != NULL) & success;
    success = ((tbworkicon = loadiconfile(app, iconpath, "work" + ext, basecolor)) != NULL) & success;
    success = ((tbwrapofficon = loadiconfile(app, iconpath, "wrapoff" + ext, basecolor)) != NULL) & success;
    success = ((tbwraponicon = loadiconfile(app, iconpath, "wrapon" + ext, basecolor)) != NULL) & success;
    success = ((tbxfeicon = loadiconfile(app, iconpath, "xfe" + ext, basecolor)) != NULL) & success;
    success = ((tbxfiicon = loadiconfile(app, iconpath, "xfi" + ext, basecolor)) != NULL) & success;
    success = ((tbxfpicon = loadiconfile(app, iconpath, "xfp" + ext, basecolor)) != NULL) & success;
    success = ((tbxfwicon = loadiconfile(app, iconpath, "xfw" + ext, basecolor)) != NULL) & success;
    success = ((tbzipicon = loadiconfile(app, iconpath, "zip" + ext, basecolor)) != NULL) & success;
    success = ((tbzoom100icon = loadiconfile(app, iconpath, "zoom100" + ext, basecolor)) != NULL) & success;
    success = ((tbzoominicon = loadiconfile(app, iconpath, "zoomin" + ext, basecolor)) != NULL) & success;
    success = ((tbzoomouticon = loadiconfile(app, iconpath, "zoomout" + ext, basecolor)) != NULL) & success;
    success = ((tbzoomwinicon = loadiconfile(app, iconpath, "zoomwin" + ext, basecolor)) != NULL) & success;
    success = ((tbtotrashicon = loadiconfile(app, iconpath, "totrash" + ext, basecolor)) != NULL) & success;
    success = ((tbdirbackicon = loadiconfile(app, iconpath, "dirback" + ext, basecolor)) != NULL) & success;
    success = ((tbdirforwardicon = loadiconfile(app, iconpath, "dirforward" + ext, basecolor)) != NULL) & success;
    success = ((tbminixferooticon = loadiconfile(app, iconpath, "minixferoot" + ext, basecolor)) != NULL) & success;
    success = ((tbminixfeicon = loadiconfile(app, iconpath, "minixfe" + ext, basecolor)) != NULL) & success;
    success = ((tbfiledialogicon = loadiconfile(app, iconpath, "filedialog" + ext, basecolor)) != NULL) & success;
    // success = ((tbbigarchaddicon = loadiconfile(app, iconpath, "bigarchadd" + ext, basecolor)) != NULL) & success;
    success = ((tbswitchpanelsicon = loadiconfile(app, iconpath, "switchpanels" + ext, basecolor)) != NULL) & success;
    success = ((tbsyncpanelsicon = loadiconfile(app, iconpath, "syncpanels" + ext, basecolor)) != NULL) & success;
    success = ((tbnewlinkicon = loadiconfile(app, iconpath, "newlink" + ext, basecolor)) != NULL) & success;
    success = ((tbgreenbuttonicon = loadiconfile(app, iconpath, "greenbutton" + ext, basecolor)) != NULL) & success;

    success = ((tbgraybuttonicon = loadiconfile(app, iconpath, "graybutton" + ext, basecolor)) != NULL) & success;
    success = ((tbkeybindingsicon = loadiconfile(app, iconpath, "keybindings" + ext, basecolor)) != NULL) & success;
    success = ((tbminikeybindingsicon = loadiconfile(app, iconpath, "minikeybindings" + ext, basecolor)) != NULL) & success;
    success = ((tbfilerestoreicon = loadiconfile(app, iconpath, "filerestore" + ext, basecolor)) != NULL) & success;
    success = ((tbrestore_bigicon = loadiconfile(app, iconpath, "restore_big" + ext, basecolor)) != NULL) & success;
    success = ((tbhorzpanelsicon = loadiconfile(app, iconpath, "horzpanels" + ext, basecolor)) != NULL) & success;
    success = ((tbvertpanelsicon = loadiconfile(app, iconpath, "vertpanels" + ext, basecolor)) != NULL) & success;
    // success = ((tbcomboarrowicon = loadiconfile(app, iconpath, "comboarrow" + ext, basecolor)) != NULL) & success;
    success = ((tbcomputericon = loadiconfile(app, iconpath, "computer" + ext, basecolor)) != NULL) & success;
    success = ((tbcloseicon = loadiconfile(app, iconpath, "close" + ext, basecolor)) != NULL) & success;
    success = ((tbthrobicon = loadiconfile(app, iconpath, "throb" + ext, basecolor)) != NULL) & success;
    success = ((tbthrobanimicon = loadiconfile(app, iconpath, "throbanim" + ext, basecolor)) != NULL) & success;
    // success = ((tbwebviewbgicon = loadiconfile(app, iconpath, "webviewbg" + ext, basecolor)) != NULL) & success;
    // ((tbwebviewbgicon = loadiconfile(app, iconpath, "webviewbg" + ext, basecolor)) != NULL);
    ((webviewxp = loadiconfile(app, iconpath, "xpwv" + ext, basecolor)) != NULL);


//resizecorner = new FXIcon(app, NULL, IMAGE_KEEP, 13, 13);
//resizecorner = new FXIcon(app, NULL, FXRGB(255,0,0), IMAGE_KEEP, 13, 13);
//FXIcon* resizecorner = NULL;
//resizecorner = new FXIcon(app, NULL, IMAGE_KEEP|IMAGE_SHMI|IMAGE_SHMP|IMAGE_OPAQUE|IMAGE_OWNED, 13, 13);
//FXColor resizecornerbuff[13*13];

//resizecorner = new FXIcon(app, NULL, 0, IMAGE_OPAQUE|IMAGE_DITHER|IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, 13, 13);
resizecorner = new FXIcon(app, NULL, 0, IMAGE_OWNED, 13, 13);
//resizecorner = new FXIcon(app);

resizecorner->resize(13, 13);
//resizecorner->fill(basecolor);
//resizecorner->fill(FXRGB(255,0,0));

//resizecorner->create();

resizecorner->setPixel(11, 0, iconhilitecolor);
resizecorner->setPixel(10, 1, iconhilitecolor);
resizecorner->setPixel(9, 2, iconhilitecolor);
resizecorner->setPixel(8, 3, iconhilitecolor);
resizecorner->setPixel(7, 4, iconhilitecolor);
resizecorner->setPixel(11, 4, iconhilitecolor);
resizecorner->setPixel(6, 5, iconhilitecolor);
resizecorner->setPixel(10, 5, iconhilitecolor);
resizecorner->setPixel(5, 6, iconhilitecolor);
resizecorner->setPixel(9, 6, iconhilitecolor);
resizecorner->setPixel(4, 7, iconhilitecolor);
resizecorner->setPixel(8, 7, iconhilitecolor);
resizecorner->setPixel(3, 8, iconhilitecolor);
resizecorner->setPixel(7, 8, iconhilitecolor);
resizecorner->setPixel(11, 8, iconhilitecolor);
resizecorner->setPixel(2, 9, iconhilitecolor);
resizecorner->setPixel(6, 9, iconhilitecolor);
resizecorner->setPixel(10, 9, iconhilitecolor);
resizecorner->setPixel(1, 10, iconhilitecolor);
resizecorner->setPixel(5, 10, iconhilitecolor);
resizecorner->setPixel(9, 10, iconhilitecolor);
resizecorner->setPixel(0, 11, iconhilitecolor);
resizecorner->setPixel(4, 11, iconhilitecolor);
resizecorner->setPixel(8, 11, iconhilitecolor);

resizecorner->setPixel(11, 1, iconshadowcolor);
resizecorner->setPixel(10, 2, iconshadowcolor);
resizecorner->setPixel(11, 2, iconshadowcolor);
resizecorner->setPixel(9, 3, iconshadowcolor);
resizecorner->setPixel(10, 3, iconshadowcolor);
resizecorner->setPixel(8, 4, iconshadowcolor);
resizecorner->setPixel(9, 4, iconshadowcolor);
resizecorner->setPixel(7, 5, iconshadowcolor);
resizecorner->setPixel(8, 5, iconshadowcolor);
resizecorner->setPixel(11, 5, iconshadowcolor);
resizecorner->setPixel(6, 6, iconshadowcolor);
resizecorner->setPixel(7, 6, iconshadowcolor);
resizecorner->setPixel(10, 6, iconshadowcolor);
resizecorner->setPixel(11, 6, iconshadowcolor);
resizecorner->setPixel(5, 7, iconshadowcolor);
resizecorner->setPixel(6, 7, iconshadowcolor);
resizecorner->setPixel(9, 7, iconshadowcolor);
resizecorner->setPixel(10, 7, iconshadowcolor);
resizecorner->setPixel(4, 8, iconshadowcolor);
resizecorner->setPixel(5, 8, iconshadowcolor);
resizecorner->setPixel(8, 8, iconshadowcolor);
resizecorner->setPixel(9, 8, iconshadowcolor);
resizecorner->setPixel(3, 9, iconshadowcolor);
resizecorner->setPixel(4, 9, iconshadowcolor);
resizecorner->setPixel(7, 9, iconshadowcolor);
resizecorner->setPixel(8, 9, iconshadowcolor);
resizecorner->setPixel(11, 9, iconshadowcolor);
resizecorner->setPixel(2, 10, iconshadowcolor);
resizecorner->setPixel(3, 10, iconshadowcolor);
resizecorner->setPixel(6, 10, iconshadowcolor);
resizecorner->setPixel(7, 10, iconshadowcolor);
resizecorner->setPixel(10, 10, iconshadowcolor);
resizecorner->setPixel(11, 10, iconshadowcolor);
resizecorner->setPixel(1, 11, iconshadowcolor);
resizecorner->setPixel(2, 11, iconshadowcolor);
resizecorner->setPixel(5, 11, iconshadowcolor);
resizecorner->setPixel(6, 11, iconshadowcolor);
resizecorner->setPixel(9, 11, iconshadowcolor);
resizecorner->setPixel(10, 11, iconshadowcolor);

//resizecorner->render();
//resizecorner->create();
resizecorner->create();
//resizecorner->create();
//resizecorner->destroy();
//std::cout << resizecorner;
//printf("\n");

//FXColor comboarrowiconbuff[11*4];


arrowicon = new FXIcon(app, NULL, 0, IMAGE_DITHER|IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, 5, 3);

arrowicon->resize(5, 3);

arrowicon->setPixel(0, 0, iconforecolor);
arrowicon->setPixel(1, 0, iconforecolor);
arrowicon->setPixel(2, 0, iconforecolor);
arrowicon->setPixel(3, 0, iconforecolor);
arrowicon->setPixel(4, 0, iconforecolor);
arrowicon->setPixel(1, 1, iconforecolor);
arrowicon->setPixel(2, 1, iconforecolor);
arrowicon->setPixel(3, 1, iconforecolor);
arrowicon->setPixel(2, 2, iconforecolor);


comboarrowicon = new FXIcon(app, NULL, 0, IMAGE_DITHER|IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, 12, 4);
//resizecorner = new FXIcon(app);

comboarrowicon->resize(12, 4);

comboarrowicon->setPixel(2, 0, iconforecolor);
comboarrowicon->setPixel(3, 0, iconforecolor);
comboarrowicon->setPixel(4, 0, iconforecolor);
comboarrowicon->setPixel(5, 0, iconforecolor);
comboarrowicon->setPixel(6, 0, iconforecolor);
comboarrowicon->setPixel(7, 0, iconforecolor);
comboarrowicon->setPixel(8, 0, iconforecolor);
comboarrowicon->setPixel(3, 1, iconforecolor);
comboarrowicon->setPixel(4, 1, iconforecolor);
comboarrowicon->setPixel(5, 1, iconforecolor);
comboarrowicon->setPixel(6, 1, iconforecolor);
comboarrowicon->setPixel(7, 1, iconforecolor);
comboarrowicon->setPixel(4, 2, iconforecolor);
comboarrowicon->setPixel(5, 2, iconforecolor);
comboarrowicon->setPixel(6, 2, iconforecolor);
comboarrowicon->setPixel(5, 3, iconforecolor);

comboarrowicon->render();
comboarrowicon->create();

tbcomboarrowicon = new FXIcon(app, NULL, 0, IMAGE_DITHER|IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, 11, 4);
tbcomboarrowicon->resize(11, 4);

tbcomboarrowicon->setPixel(2, 0, iconforecolor);
tbcomboarrowicon->setPixel(3, 0, iconforecolor);
tbcomboarrowicon->setPixel(4, 0, iconforecolor);
tbcomboarrowicon->setPixel(5, 0, iconforecolor);
tbcomboarrowicon->setPixel(6, 0, iconforecolor);
tbcomboarrowicon->setPixel(7, 0, iconforecolor);
tbcomboarrowicon->setPixel(8, 0, iconforecolor);
tbcomboarrowicon->setPixel(3, 1, iconforecolor);
tbcomboarrowicon->setPixel(4, 1, iconforecolor);
tbcomboarrowicon->setPixel(5, 1, iconforecolor);
tbcomboarrowicon->setPixel(6, 1, iconforecolor);
tbcomboarrowicon->setPixel(7, 1, iconforecolor);
tbcomboarrowicon->setPixel(4, 2, iconforecolor);
tbcomboarrowicon->setPixel(5, 2, iconforecolor);
tbcomboarrowicon->setPixel(6, 2, iconforecolor);
tbcomboarrowicon->setPixel(5, 3, iconforecolor);

tbcomboarrowicon->render();
tbcomboarrowicon->create();


closeicon = new FXIcon(app, NULL, 0, IMAGE_DITHER|IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, 18, 11);
closeicon->setPixel(5, 2, iconforecolor);
closeicon->setPixel(6, 2, iconforecolor);
closeicon->setPixel(11, 2, iconforecolor);
closeicon->setPixel(12, 2, iconforecolor);
closeicon->setPixel(6, 3, iconforecolor);
closeicon->setPixel(7, 3, iconforecolor);
closeicon->setPixel(10, 3, iconforecolor);
closeicon->setPixel(11, 3, iconforecolor);
closeicon->setPixel(7, 4, iconforecolor);
closeicon->setPixel(8, 4, iconforecolor);
closeicon->setPixel(9, 4, iconforecolor);
closeicon->setPixel(10, 4, iconforecolor);
closeicon->setPixel(8, 5, iconforecolor);
closeicon->setPixel(9, 5, iconforecolor);
closeicon->setPixel(7, 6, iconforecolor);
closeicon->setPixel(8, 6, iconforecolor);
closeicon->setPixel(9, 6, iconforecolor);
closeicon->setPixel(10, 6, iconforecolor);
closeicon->setPixel(6, 7, iconforecolor);
closeicon->setPixel(7, 7, iconforecolor);
closeicon->setPixel(10, 7, iconforecolor);
closeicon->setPixel(11, 7, iconforecolor);
closeicon->setPixel(5, 8, iconforecolor);
closeicon->setPixel(6, 8, iconforecolor);
closeicon->setPixel(11, 8, iconforecolor);
closeicon->setPixel(12, 8, iconforecolor);


closeicon->render();
closeicon->create();


imgstipple = new FXImage(app, NULL, IMAGE_NEAREST|IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, 2, 2);
imgstipple->setPixel(0, 0, iconhilitecolor);
imgstipple->setPixel(1, 1, iconhilitecolor);
imgstipple->setPixel(1, 0, basecolor);
imgstipple->setPixel(0, 1, basecolor);

imgstipple->render();
imgstipple->create();


/* if (tbiconsmenuicon->getHeight() == 16) {
	tbiconsmenuicon->setPixel(18, 7, iconforecolor);
	tbiconsmenuicon->setPixel(19, 7, iconforecolor);
	tbiconsmenuicon->setPixel(20, 7, iconforecolor);
	tbiconsmenuicon->setPixel(21, 7, iconforecolor);
	tbiconsmenuicon->setPixel(22, 7, iconforecolor);
	tbiconsmenuicon->setPixel(19, 8, iconforecolor);
	tbiconsmenuicon->setPixel(20, 8, iconforecolor);
	tbiconsmenuicon->setPixel(21, 8, iconforecolor);
	tbiconsmenuicon->setPixel(20, 9, iconforecolor);
} else if (tbiconsmenuicon->getHeight() == 24) {
	tbiconsmenuicon->setPixel(26, 12, iconforecolor);
	tbiconsmenuicon->setPixel(27, 12, iconforecolor);
	tbiconsmenuicon->setPixel(28, 12, iconforecolor);
	tbiconsmenuicon->setPixel(29, 12, iconforecolor);
	tbiconsmenuicon->setPixel(30, 12, iconforecolor);
	tbiconsmenuicon->setPixel(27, 13, iconforecolor);
	tbiconsmenuicon->setPixel(28, 13, iconforecolor);
	tbiconsmenuicon->setPixel(29, 13, iconforecolor);
	tbiconsmenuicon->setPixel(28, 14, iconforecolor);
}


tbiconsmenuicon->render();
tbiconsmenuicon->create(); */

    return(success);
}


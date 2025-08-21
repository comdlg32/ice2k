#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "startupnotification.h"
#include "MessageBox.h"
#include "FilePanel.h"
#include "XFileExplorer.h"


// Add FOX hacks
#include "foxhacks.cpp"
#include "clearlooks.cpp"

// Main window
FXMainWindow* mainWindow;


// Startup notification
#ifdef STARTUP_NOTIFICATION

static time_t startup_end = 0;
static int    error_trap_depth = 0;

static int x_error_handler(Display* xdisplay, XErrorEvent* error)
{
    char buf[64];

    XGetErrorText(xdisplay, error->error_code, buf, 63);

    if (error_trap_depth == 0)
    {
        // XSetInputFocus can cause BadMatch errors
        // we ignore this in x_error_handler
        if (error->request_code == 42)
        {
            return(0);
        }

        fprintf(stderr, "Unexpected X error: %s serial %ld error_code %d request_code %d minor_code %d)\n",
                buf,
                error->serial,
                error->error_code,
                error->request_code,
                error->minor_code);
        //exit(EXIT_FAILURE);
    }

    return(1); // Return value is meaningless
}


static void error_trap_push(SnDisplay* display, Display*   xdisplay)
{
    ++error_trap_depth;
}


static void error_trap_pop(SnDisplay* display, Display*   xdisplay)
{
    if (error_trap_depth == 0)
    {
        fprintf(stderr, "Error: Trap underflow\n");
        exit(EXIT_FAILURE);
    }

    XSync(xdisplay, False);  // Get all errors out of the queue
    --error_trap_depth;
}


// Startup notification monitoring function
static void snmonitor(SnMonitorEvent* event, void* user_data)
{
    //SnMonitorContext *context;
    //SnStartupSequence *sequence;

    //context = sn_monitor_event_get_context (event);
    //sequence = sn_monitor_event_get_startup_sequence (event);
    sn_monitor_event_get_context(event);
    sn_monitor_event_get_startup_sequence(event);

    switch (sn_monitor_event_get_type(event))
    {
    case SN_MONITOR_EVENT_INITIATED:
    case SN_MONITOR_EVENT_CHANGED:

        // Startup timeout time
        time_t t;
        t = time(NULL);
        startup_end = t+STARTUP_TIMEOUT;

        ::setWaitCursor(mainWindow->getApp(), BEGIN_CURSOR);

        /* For debugging purpose
         *
         * const char *s;
         *
         * if (sn_monitor_event_get_type (event) == SN_MONITOR_EVENT_INITIATED)
         *  fprintf(stderr,"Initiated sequence %s\n",sn_startup_sequence_get_id (sequence));
         * else
         *  fprintf(stderr,"Changed sequence %s\n",sn_startup_sequence_get_id (sequence));
         *
         * s = sn_startup_sequence_get_id (sequence);
         * fprintf(stderr," id %s\n", s ? s : "(unset)");
         *
         * s = sn_startup_sequence_get_name (sequence);
         * fprintf(stderr," name %s\n", s ? s : "(unset)");
         *
         * s = sn_startup_sequence_get_description (sequence);
         * fprintf(stderr," description %s\n", s ? s : "(unset)");
         *
         * fprintf(stderr," workspace %d\n", sn_startup_sequence_get_workspace (sequence));
         *
         * s = sn_startup_sequence_get_binary_name (sequence);
         * fprintf(stderr," binary name %s\n", s ? s : "(unset)");
         *
         * s = sn_startup_sequence_get_icon_name (sequence);
         * fprintf(stderr," icon name %s\n", s ? s : "(unset)");
         *
         * s = sn_startup_sequence_get_wmclass (sequence);
         * fprintf(stderr," wm class %s\n", s ? s : "(unset)");
         *
         */

        break;

    case SN_MONITOR_EVENT_COMPLETED:

           ::setWaitCursor(mainWindow->getApp(), END_CURSOR);

        break;

    case SN_MONITOR_EVENT_CANCELED:

           ::setWaitCursor(mainWindow->getApp(), END_CURSOR);

        break;
    }
}


// Perform one event dispatch with startup notification
bool FXApp::runOneEvent(bool blocking)
{
    FXRawEvent ev;

    // We have to select for property events on at least one
    // root window (but not all as INITIATE messages go to
    // all root windows)

    static FXbool     firstcall = true;
    static SnDisplay* sndisplay;

    //static SnMonitorContext *context;
    if (firstcall)
    {
        XSetErrorHandler(x_error_handler);
        XSelectInput((Display*)display, DefaultRootWindow((Display*)display), PropertyChangeMask);
        sndisplay = sn_display_new((Display*)display, error_trap_push, error_trap_pop);
        //context = sn_monitor_context_new (sndisplay, DefaultScreen ((Display*)display),snmonitor,NULL, NULL);
        sn_monitor_context_new(sndisplay, DefaultScreen((Display*)display), snmonitor, NULL, NULL);
        firstcall = false;
    }

    if (getNextEvent(ev, blocking))
    {
        // Check if startup timeout expired
        time_t t;
        t = time(NULL);
        if ((startup_end != 0) && (startup_end-t < 0))
        {
            ::setWaitCursor(mainWindow->getApp(), END_CURSOR);
            startup_end = 0;
        }

        sn_display_process_event(sndisplay, &ev);

        dispatchEvent(ev);
        return(true);
    }

    return(false);
}


#endif


// Global variables
char** args;
FXbool xim_used = false;

#if defined(linux)
FXuint pkg_format;
#endif

// Base directories (according to the Freedesktop specification version 0.7)
FXString homedir;
FXString xdgdatahome;
FXString xdgconfighome;

// Used to force panel view mode from command line
int panel_mode = -1;


// Hand cursor replacement
#define hand_width     32
#define hand_height    32
#define hand_x_hot     6
#define hand_y_hot     1
static const FXuchar hand_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00,
    0x90, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x90, 0x07, 0x00, 0x00,
    0x97, 0x1a, 0x00, 0x00, 0x99, 0x2a, 0x00, 0x00, 0x11, 0x28, 0x00, 0x00,
    0x12, 0x20, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00,
    0x04, 0x20, 0x00, 0x00, 0x04, 0x10, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00,
    0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const FXuchar hand_mask_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,
    0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00,
    0xf7, 0x1f, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00,
    0xfe, 0x3f, 0x00, 0x00, 0xfe, 0x3f, 0x00, 0x00, 0xfe, 0x3f, 0x00, 0x00,
    0xfc, 0x3f, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00,
    0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Usage message
#define USAGE_MSG    _("\
\nUsage: xfe [options...] [FOLDER|FILE...]\n\
\n\
    [options...] are the following:\n\
\n\
        -h,   --help         Print (this) help screen and exit.\n\
        -v,   --version      Print version information and exit.\n\
        -i,   --iconic       Start iconified.\n\
        -m,   --maximized    Start maximized.\n\
        -p n, --panel n      Force panel view mode to n (n=0 => Tree and one panel,\n\
                             n=1 => One panel, n=2 => Two panels, n=3 => Tree and two panels).\n\
\n\
    [FOLDER|FILE...] is a list of folders or files to open on startup.\n\
    The first two folders are displayed in the file panels, the others are ignored.\n\
    The number of files to open is not limited.\n\
\n")



int main(int argc, char* argv[])
{
    const char* title = "Xfe";
    const char* appname = "xfe";
    const char* vdrname = "Xfe";
    int         i;
    FXbool      loadicons;
    FXString    startfiledir1 = "";
    FXString    startdir2 = "";
    FXbool      iconic = false;
    FXbool      maximized = false;
    FXString    xmodifiers;
    FXString	cmd;

    // Vector of URIs to open on startup
    vector_FXString startURIs;

    // Get environment variables $HOME, $XDG_DATA_HOME and $XDG_CONFIG_HOME
    homedir = FXSystem::getHomeDirectory();
    if (homedir == "")
    {
        homedir = ROOTDIR;
    }
    xdgdatahome = getenv("XDG_DATA_HOME");
    if (xdgdatahome == "")
    {
        xdgdatahome = homedir + PATHSEPSTRING DATAPATH;
    }
    xdgconfighome = getenv("XDG_CONFIG_HOME");
    if (xdgconfighome == "")
    {
        xdgconfighome = homedir + PATHSEPSTRING CONFIGPATH;
    }

    // Detect if an X input method is used
    xmodifiers = getenv("XMODIFIERS");
    if ((xmodifiers == "") || (xmodifiers == "@im=none"))
    {
        xim_used = false;
    }
    else
    {
        xim_used = true;
    }

#ifdef HAVE_SETLOCALE
    // Set locale via LC_ALL.
    setlocale(LC_ALL, "");
#endif

#if ENABLE_NLS
    // Set the text message domain.
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "utf-8");
    textdomain(PACKAGE);
#endif

#if defined(linux)
    // For package query on Linux systems, try to guess if the default package format is deb or rpm:
    //   - if dpkg exists then the system uses deb packages
    //   - else if rpm exists, then the system uses rpm packages
    //   - else another (unsupported) package manager is used

    cmd = "dpkg --version";
    FXString str = getCommandOutput(cmd);

    if (str.find("Debian") != -1)
    {
        pkg_format = DEB_PKG; // deb based system
    }
    else
    {
        cmd = "rpm --version";
        str = getCommandOutput(cmd);

        if (str.find("RPM") != -1)
        {
            pkg_format = RPM_PKG; // rpm based system
        }
        else
        {
            pkg_format = OTHER_PKG; // other (unsupported) package system
        }
    }
#endif

    // Parse basic arguments
    for (i = 1; i < argc; ++i)
    {
        if ((compare(argv[i], "-v") == 0) || (compare(argv[i], "--version") == 0))
        {
            fprintf(stdout, "%s version %s\n", PACKAGE, VERSION);
            exit(EXIT_SUCCESS);
        }
        else if ((compare(argv[i], "-h") == 0) || (compare(argv[i], "--help") == 0))
        {
            fprintf(stdout, USAGE_MSG);
            exit(EXIT_SUCCESS);
        }
        else if ((compare(argv[i], "-i") == 0) || (compare(argv[i], "--iconic") == 0))
        {
            iconic = true;
        }
        else if ((compare(argv[i], "-m") == 0) || (compare(argv[i], "--maximized") == 0))
        {
            maximized = true;
        }
        else if ((compare(argv[i], "-p") == 0) || (compare(argv[i], "--panel") == 0) || (compare(argv[i], "--panels") == 0))
        {
            if (++i < argc)
            {
                panel_mode = atoi(argv[i]);
            }
            if ((panel_mode < 0) || (panel_mode > 3))
            {
                fprintf(stderr, _("Warning: Unknown panel mode, revert to last saved panel mode\n"));
                panel_mode = -1;
            }
        }
        else if (compare(argv[i], "-p0") == 0)
        {
            panel_mode = 0;
        }
        else if (compare(argv[i], "-p1") == 0)
        {
            panel_mode = 1;
        }
        else if (compare(argv[i], "-p2") == 0)
        {
            panel_mode = 2;
        }
        else if (compare(argv[i], "-p3") == 0)
        {
            panel_mode = 3;
        }
        else
        {
            // Starting URIs, if any
            startURIs.push_back(::filePath(::fileFromURI(argv[i])));
        }
    }

    // Global variable (used to properly restart Xfe)
    args = argv;

    // Application creation
    FXApp* application = new FXApp(appname, vdrname);
    application->init(argc, argv);

    // Redefine the default hand cursor
    FXCursor* hand = new FXCursor(application, hand_bits, hand_mask_bits, hand_width, hand_height, hand_x_hot, hand_y_hot);
    application->setDefaultCursor(DEF_HAND_CURSOR, hand);

    // Read registry thru foxhacks
    application->reg().read();

    // Set base color (to change the default base color at first run)
    FXColor basecolor = application->reg().readColorEntry("SETTINGS", "basecolor", FXRGB(237, 233, 227));
    application->setBaseColor(basecolor);

    // Load all application icons
    loadicons = loadAppIcons(application);

    // Set normal font
    FXString fontspec;
    fontspec = application->reg().readStringEntry("SETTINGS", "font", DEFAULT_NORMAL_FONT);
    if (!fontspec.empty())
    {
        FXFont* normalFont = new FXFont(application, fontspec);
        application->setNormalFont(normalFont);
    }

    // If root
    if (getuid() == 0)
    {
        title = "Xfe (root)";
    }

    // Create and run application
    mainWindow = new XFileExplorer(application, startURIs, iconic, maximized, title, realxfeicon, minixfeicon);

    // Catch SIGCHLD to harvest zombie child processes
    application->addSignal(SIGCHLD, mainWindow, XFileExplorer::ID_HARVEST, true);

    // Also catch interrupt so we can gracefully terminate
    application->addSignal(SIGINT, mainWindow, XFileExplorer::ID_QUIT);

    application->create();

    // Tooltips setup time and duration
    application->setTooltipPause(TOOLTIP_PAUSE);
    application->setTooltipTime(TOOLTIP_TIME);

    if (!loadicons)
    {
        MessageBox::error(application, BOX_OK, _("Error loading icons"), _("Unable to load some icons. Please check your icons path!"));
    }

    return(application->run());
}

#include "config.h"
#include "i18n.h"

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <fx.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "startupnotification.h"



#ifdef STARTUP_NOTIFICATION  // Use startup notification


// Indicate that launchee has completed startup
void startup_completed(void)
{
    SnLauncheeContext* launchee;
    Display*           xdisplay;
    SnDisplay*         display;

    // Open display
    xdisplay = XOpenDisplay(NULL);
    if (xdisplay != NULL)
    {
        // Create startup notification context
        display = sn_display_new(xdisplay, NULL, NULL);
        launchee = sn_launchee_context_new_from_environment(display, DefaultScreen(xdisplay));

        // Indicate startup has completed and free resources
        if (launchee)
        {
            sn_launchee_context_complete(launchee);
            sn_launchee_context_unref(launchee);
        }
        sn_display_unref(display);
        XCloseDisplay(xdisplay);
    }
}


// Hack to obtain a timestamp for startup notification
// Create a fake window and set up a property change event
// Code snippet borrowed from the Internet
Time gettimestamp(Display *display)
{
    Window   window;
    XEvent   event;
    Atom     atom_name, atom_type;

    window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, InputOnly, 0, 0, NULL);
    XSelectInput(display, window, PropertyChangeMask);
    atom_name = XInternAtom(display, "_NET_WM_USER_TIME_WINDOW", False);
    atom_type = XInternAtom(display, "WINDOW", true);
    XChangeProperty(display, window, atom_name, atom_type, 8, PropModeReplace, (const FXuchar*)&window, 1);
    XNextEvent(display, &event);
    assert(event.type == PropertyNotify);

    return(((XPropertyEvent*)&event)->time);
}


// Launch a command and initiate a startup notification
int runcmd(FXString cmd, FXString cmdname, FXString dir, FXString startdir, FXbool usesn = true, FXString snexcepts = "")
{
    int ret;

    // Change to current directory
    ret = chdir(dir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            fprintf(stderr, _("Error: Can't enter folder %s: %s"), dir.text(), strerror(errcode));
        }
        else
        {
            fprintf(stderr, _("Error: Can't enter folder %s"), dir.text());
        }

        return(-1);
    }

    // Get rid of possible command options
    cmdname = cmdname.before(' ');

    // Check if command is in the startup notification exception list
    FXbool startup_notify = true;
    if (snexcepts != "")
    {
        FXString entry;
        for (int i = 0; ; i++)
        {
            entry = snexcepts.section(':', i);
            if (streq(entry.text(), ""))
            {
                break;
            }
            if (streq(entry.text(), cmdname.text()))
            {
                startup_notify = false;
                break;
            }
        }
    }

    // Run command with startup notification
    if (usesn && startup_notify)
    {
        Display*           xdisplay;
        SnDisplay*         display;
        SnLauncherContext* context;
        Time               timestamp;

        // Open display
        xdisplay = XOpenDisplay(NULL);
        if (xdisplay == NULL)
        {
            fprintf(stderr, _("Error: Can't open display\n"));
            ret = chdir(startdir.text());
            if (ret < 0)
            {
                int errcode = errno;
                if (errcode)
                {
                    fprintf(stderr, _("Error: Can't enter folder %s: %s"), startdir.text(), strerror(errcode));
                }
                else
                {
                    fprintf(stderr, _("Error: Can't enter folder %s"), startdir.text());
                }
            }
            return(-1);
        }

        // Message displayed in the task bar (if any)
        FXString message;
        message.format(_("Start of %s"), cmdname.text());

        // Initiate launcher context
        display = sn_display_new(xdisplay, NULL, NULL);
        context = sn_launcher_context_new(display, DefaultScreen(xdisplay));
        sn_launcher_context_set_name(context, message.text());
        sn_launcher_context_set_binary_name(context, cmdname.text());
        sn_launcher_context_set_description(context, message.text());
        sn_launcher_context_set_icon_name(context, cmdname.text());
        timestamp = gettimestamp(xdisplay);
        sn_launcher_context_initiate(context, "Xfe", cmd.text(), timestamp);

        // Run command in background
        cmd += " &";

        static pid_t child_pid = 0;
        switch ((child_pid = fork()))
        {
        case -1:
            fprintf(stderr, _("Error: Fork failed: %s\n"), strerror(errno));
            break;

        case 0: // Child
            sn_launcher_context_setup_child_process(context);
            execl("/bin/sh", "sh", "-c", cmd.text(), (char*)NULL);
            _exit(EXIT_SUCCESS);
            break;
        }
        sn_launcher_context_unref(context);
        XCloseDisplay(xdisplay);
    }

    // Run command without startup notification
    else
    {
        // Run command in background
        cmd += " &";
        ret = system(cmd.text());
        if (ret < 0)
        {
            fprintf(stderr, _("Error: Can't execute command %s"), cmd.text());
            return(-1);
        }

        // Just display the wait cursor during a second
        sleep(1);
    }

    // Go back to startup directory
    ret = chdir(startdir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            fprintf(stderr, _("Error: Can't enter folder %s: %s"), startdir.text(), strerror(errcode));
        }
        else
        {
            fprintf(stderr, _("Error: Can't enter folder %s"), startdir.text());
        }

        return(-1);
    }

    return(0);
}


#else  // Don't use startup notification

// Run a command and simulate a startup time
int runcmd(FXString cmd, FXString dir, FXString startdir)
{
    int ret;

    // Change to current directory
    ret = chdir(dir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            fprintf(stderr, _("Error: Can't enter folder %s: %s"), dir.text(), strerror(errcode));
        }
        else
        {
            fprintf(stderr, _("Error: Can't enter folder %s"), dir.text());
        }

        return(-1);
    }

    // Run the command in background
    cmd += " &";
    ret = system(cmd.text());
    if (ret < 0)
    {
        fprintf(stderr, _("Error: Can't execute command %s"), cmd.text());
        return(-1);
    }

    // Very ugly simulation of a startup time!!!
    sleep(SIMULATED_STARTUP_TIME);

    // Go back to startup directory
    ret = chdir(startdir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            fprintf(stderr, _("Error: Can't enter folder %s: %s"), startdir.text(), strerror(errcode));
        }
        else
        {
            fprintf(stderr, _("Error: Can't enter folder %s"), startdir.text());
        }

        return(-1);
    }

    return(0);
}


#endif

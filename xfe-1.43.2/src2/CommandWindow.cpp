// Command window object
// Executes a command and returns the results in the command window
// Close button to close the window (but not kill the child process)
// Cancel button to kill the child process (but not close the window)
// The object deletes itself when the close button is pressed
// The command window can be a free-floating window or can be
// a window which will always float over the owner window

#include "config.h"
#include "i18n.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <fx.h>

#include "xfedefs.h"
#include "icons.h"
#include "MessageBox.h"
#include "CommandWindow.h"
#include "xfedefs.h"
#include "XFileExplorer.h"

// Map
FXDEFMAP(CommandWindow) CommandWindowMap[] =
{
    FXMAPFUNC(SEL_COMMAND, CommandWindow::ID_CLOSE, CommandWindow::onCmdClose),
    FXMAPFUNC(SEL_COMMAND, CommandWindow::ID_KILLPROCESS, CommandWindow::onCmdKillProcess),
    FXMAPFUNC(SEL_UPDATE, CommandWindow::ID_KILLPROCESS, CommandWindow::onUpdKillProcess),
    FXMAPFUNC(SEL_UPDATE, CommandWindow::ID_CLOSE, CommandWindow::onUpdClose),
    FXMAPFUNC(SEL_CHORE, CommandWindow::ID_WATCHPROCESS, CommandWindow::onWatchProcess),
};


// Object implementation
FXIMPLEMENT(CommandWindow, DialogBox, CommandWindowMap, ARRAYNUMBER(CommandWindowMap))


// Construct window which will always float over the owner window
CommandWindow::CommandWindow(FXWindow* owner, const FXString& name, FXString strcmd, int nblines, int nbcols) :
    DialogBox(owner, name, DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_MAXIMIZE|DECOR_CLOSE, 0, 0, 0, 0, 6, 6, 6, 6, 4, 4)
{
    // Get command to execute
    command = strcmd;

    // Bottom part
    FXHorizontalFrame* buttonbox = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    new FXButton(buttonbox, _("Cl&ose"), NULL, this, ID_CLOSE, BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0, 0, 0, 0, 20, 20, 5, 5);
    FXButton* cancelbutton = new FXButton(buttonbox, _("&Cancel"), NULL, this, ID_KILLPROCESS, BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0, 0, 0, 0, 20, 20, 5, 5);

    // Text part
    FXHorizontalFrame* textbox = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 0, 0, 0, 0);
    text = new FXText(textbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text->setVisibleRows(nblines);
    text->setVisibleColumns(nbcols);

	appendText(_("Please wait...\n\n"));


    cancelbutton->setFocus();

    // Initialize variables
    pid = -1;
    killed = false;
    closed = false;
}


// Construct free-floating window
CommandWindow::CommandWindow(FXApp* a, const FXString& name, FXString strcmd, int nblines, int nbcols) :
    DialogBox(a, name, DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_MAXIMIZE|DECOR_MINIMIZE|DECOR_CLOSE, 0, 0, 0, 0, 6, 6, 6, 6, 4, 4)
{
    // Get command to execute
    command = strcmd;

    // Bottom part
    FXHorizontalFrame* buttonbox = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    new FXButton(buttonbox, _("Cl&ose"), NULL, this, ID_CLOSE, BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0, 0, 0, 0, 20, 20, 5, 5);
    FXButton* cancelbutton = new FXButton(buttonbox, _("&Cancel"), NULL, this, ID_KILLPROCESS, BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0, 0, 0, 0, 20, 20, 5, 5);

    // Text part
    FXHorizontalFrame* textbox = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 0, 0, 0, 0);
    text = new FXText(textbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text->setVisibleRows(nblines);
    text->setVisibleColumns(nbcols);

	appendText(_("Please wait...\n\n"));

    cancelbutton->setFocus();

    // Initialize variables
    pid = -1;
    killed = false;
    closed = false;
}


// Make window
void CommandWindow::create()
{
    // Set text font
    FXString fontspec;

    fontspec = getApp()->reg().readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
    if (!fontspec.empty())
    {
        FXFont* font = new FXFont(getApp(), fontspec);
        font->create();
        text->setFont(font);
    }

    DialogBox::create();
    show(PLACEMENT_OWNER);

    // Execute command
    execCmd(command.text());
}


// Kill process when clicking on the cancel button
long CommandWindow::onCmdKillProcess(FXObject*, FXSelector, void*)
{
    kill((-1*pid), SIGTERM); // Kills the process group
    killed = true;
    return(0);
}


// Update cancel button
long CommandWindow::onUpdKillProcess(FXObject* sender, FXSelector, void*)
{
    FXButton* btn = (FXButton*)sender;

    if (!getApp()->hasChore(this, ID_WATCHPROCESS))
    {
        btn->disable();
    }
    else
    {
        btn->enable();
    }
    return(1);
}


// Update close button
long CommandWindow::onUpdClose(FXObject* sender, FXSelector, void*)
{
    FXButton* btn = (FXButton*)sender;

    if (!getApp()->hasChore(this, ID_WATCHPROCESS))
    {
        btn->enable();
    }
    else
    {
        btn->disable();
    }
    return(1);
}


// Execute a command and capture its output
int CommandWindow::execCmd(FXString command)
{
    // Open pipes to communicate with child process
    if (pipe(pipes) == -1)
    {
        return(-1);
    }

    // Create child process
    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, _("Error: Fork failed: %s\n"), strerror(errno));
        return(-1);
    }
    if (pid == 0) // Child
    {
        char* args[4];
        int   ret1 = dup2(pipes[0], STDIN_FILENO);   // Use the pipes as the new channels
        int   ret2 = dup2(pipes[1], STDOUT_FILENO);  // (where stdout and stderr
        int   ret3 = dup2(pipes[1], STDERR_FILENO);  // go to the same pipe!).

        if ((ret1 < 0) || (ret2 < 0) || (ret3 < 0))
        {
            int errcode = errno;
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't duplicate pipes: %s"), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't duplicate pipes"));
            }

            return(-1);
        }

        args[0] = (char*)"sh";           // Setup arguments
        args[1] = (char*)"-vc";          // to run command (option -v to display the command to execute)
        args[2] = (char*)command.text(); // in a shell in
        args[3] = NULL;                  // a new process.
        setpgid(0, 0);                   // Allows to kill the whole group
        execvp(args[0], args);           // Start a new process which will execute the command.
        _exit(EXIT_FAILURE);             // We'll get here only if an error occurred.
    }
    else // Parent
    {
        // Make sure we get called so we can check when child has finished
        getApp()->addChore(this, ID_WATCHPROCESS);
    }
    return(0);
}


// Watch progress of child process
long CommandWindow::onWatchProcess(FXObject*, FXSelector, void*)
{
    char buf[1024];
    int  nread;

    if (closed)
    {
        // The close button was pressed : just close the pipes
        // and delete the object

        // Close pipes
        ::close(pipes[0]);
        ::close(pipes[1]);

        // Object deletes itself!
        delete this;
    }

    else if ((waitpid(pid, NULL, WNOHANG) == 0))
    {
        // Child is still running, just wait
        getApp()->addChore(this, ID_WATCHPROCESS);

        // Read data from the running child (first, set I-O to non-blocking)
        int pflags;
        if ((pflags = fcntl(pipes[0], F_GETFL)) >= 0)
        {
            pflags |= O_NONBLOCK;
            if (fcntl(pipes[0], F_SETFL, pflags) >= 0)
            {
                // Now read the data from the pipe
                while ((nread = read(pipes[0], buf, sizeof(buf)-1)) > 0)
                {
                    buf[nread] = '\0';
                    // Remove backspace characters, if any
                    FXString strbuf = buf;
                    strbuf = strbuf.substitute("\b", ".");
                    text->appendText(strbuf.text(), strlen(strbuf.text()));
                    scrollToLastLine();
                    if (nread < (int)(sizeof(buf)-1))
                    {
                        break;
                    }
                }
            }
        }
    }

    else
    {
        // Child has finished.
        // Read data from the finished child
        while ((nread = read(pipes[0], buf, sizeof(buf)-1)) > 0)
        {
            buf[nread] = '\0';
            // Remove backspace characters, if any
            FXString strbuf = buf;
            strbuf = strbuf.substitute("\b", ".");
            text->appendText(strbuf.text(), strlen(strbuf.text()));
            scrollToLastLine();
            if (nread < (int)(sizeof(buf)-1))
            {
                break;
            }
        }
        if (killed)
        {
            appendText(_("\n>>>> COMMAND CANCELLED <<<<"));
        }
        else
        {
            appendText(_("\n>>>> END OF COMMAND <<<<"));
        }
        scrollToLastLine();

        // Close pipes
        ::close(pipes[0]);
        ::close(pipes[1]);
    }

    return(1);
}


// Close dialog when clicking on the close button
long CommandWindow::onCmdClose(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this, true);
    hide();
    closed = true;

    // Object deletes itself
    delete this;

    return(1);
}


// Change the text in the buffer to new text
void CommandWindow::setText(const char* str)
{
    text->setText(str, strlen(str));
    getApp()->repaint();
}


// Append new text at the end of the buffer
void CommandWindow::appendText(const char* str)
{
    text->appendText(str, strlen(str));
    getApp()->repaint();
}


// Scroll to the last line
void CommandWindow::scrollToLastLine(void)
{
    text->makePositionVisible(text->getLength());
    getApp()->repaint();
}


// Get text length
int CommandWindow::getLength(void)
{
    return(text->getLength());
}


// Clean up
CommandWindow::~CommandWindow()
{
    getApp()->removeChore(this, ID_WATCHPROCESS);

    text = (FXText*)-1;
}

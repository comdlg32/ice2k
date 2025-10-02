/* number of buttons, toggles, and string options */

/* This structure put together the description
   in the menu , the data , and commande associate
*/

typedef struct optionstruct {
	char *description;
	char *cmd;
	char *userdata;
} OptionStruct;

static OptionStruct Opt[] =
{
      {(char *) "Enter the user name.", (char *)"username", NULL},
      {(char *) "Enter the password message.", (char *)"password", NULL},
      {(char *) "Enter the info message.", (char *)"info", NULL},
      {(char *) "Enter the valid message.", (char *)"validate", NULL},
      {(char *) "Enter the invalid message.", (char *)"invalid", NULL},
      {(char *) "Enter the geometry mxn.", (char *)"geometry", NULL}
};

static char *toggleNames[] =
{
	(char *) "allowaccess",
	(char *) "allowroot",
	(char *) "debug",
	(char *) "echokeys",
	(char *) "echokeys",
	(char *) "enablesaver",
	(char *) "fullrandom",
	(char *) "grabmouse",
	(char *) "grabserver",
	(char *) "install",
	(char *) "mousemotion",
	(char *) "nolock",
	(char *) "remote",
	(char *) "sound",
	(char *) "timeelapsed",
	(char *) "trackmouse",
	(char *) "use3d",
	(char *) "usefirst",
	(char *) "verbose",
	(char *) "wireframe",
	(char *) "mono"
};

#if 0
#define numOptions 6
#define numToggleNames 21
#else
#define numOptions (sizeof(Opt) / sizeof(OptionStruct))
#define numToggleNames (sizeof(toggleNames) / sizeof(toggleNames[0]))
#endif

#define XLOCK "xlock"
#define WINDOW_WIDTH 160
#define WINDOW_HEIGHT 100
#define WINDOW_GEOMETRY "160x100"

void exitCallback(Widget w, XtPointer client_data, XtPointer call_data);

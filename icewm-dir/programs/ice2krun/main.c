	//All code here is licensed under LGPLv2.

#include <gtk/gtk.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <unistd.h>
#include "error.h"
#include <gdk/gdkkeysyms.h>

GtkWidget *entry; 
GtkWidget *window; 

#define BUTTON_Y 115
#define WINDOW_WIDTH 341
#define WINDOW_HEIGHT 157
#define BUTTON_WIDTH 74
#define BUTTON_HEIGHT 22

void version (){
	printf("Congrats to you if you found this!\n"
               "Ice2k.sys Run Dialogue 1.0\n"
               "Made by m1nuscle - https://m1nuscle.neocities.org/\n"
               "Modified by 0penrc\n");
}


void child(GPid pid, gint status, gpointer user_data) {
    g_spawn_close_pid(pid); 
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            error("An error occurred - program exited abnormally or is not installed/available");
        }
    } else if (WIFSIGNALED(status)) {
        error("Program was terminated by signal; What on earth are you doing");
    } else {
        error("An error occurred");
    }
    gtk_main_quit();
}


char* getprogpath(char *argv0) {
    static char path[PATH_MAX];

    // what the fuck?
    //char *argv0 = getenv("_");
    if (argv0 == NULL) {
        fprintf(stderr, "Failed to retrieve how the program was opened (how the fuck does this happen?)\n");
        return NULL;
    }

    // if program is ran thru an absolute path
    /* if (argv0[0] == '/') {
        char *dir = dirname(argv0);
        //strncpy(path, dir, PATH_MAX);
        strncpy(path, argv0, PATH_MAX);
        return path;
    } */

    // if program is ran thru an relative path
    if (strchr(argv0, '/') != NULL) {
        if (realpath(argv0, path) != NULL) {
            char *dir = dirname(path);
            strncpy(path, dir, PATH_MAX);
            return path;
        } else {
            perror("realpath() error");
            return NULL;
        }
    }

    // if program is ran thru a plain word somehow if some mf changes the path to this dir for whatever the fuck
    char *env_path = getenv("PATH");
    if (env_path != NULL) {
        char *token = strtok(env_path, ":");
        while (token != NULL) {
            snprintf(path, sizeof(path), "%s/%s", token, argv0);
            if (access(path, X_OK) == 0) {
                char *dir = dirname(path);
                strncpy(path, dir, PATH_MAX);
                return path;
            }
            token = strtok(NULL, ":");
        }
    }

    // if the executable is not found in any of the PATH directories
    fprintf(stderr, "Executable %s not found in $PATH\n", argv0);
    return NULL;
}


void run(GtkWidget* widget, GtkWidget* window) {
    const gchar *program = gtk_entry_get_text(GTK_ENTRY(entry)); // acquire the text from entryw idget

    if (program != NULL && *program != '\0') {
        gtk_widget_hide(window);
        GPid child_pid;
        GError *err = NULL;
        gchar *argv[] = {"/bin/sh", "-c", (gchar *)program, NULL};
        /*1. "/bin/sh" - path to the shell because were running this using shell
           2. "-c" - flag for shell which tells the shell to execute the next arg as a command
           3 program - the program you want to execute
           4. NULL - arg list terminator*/
        if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &child_pid, &err)) {
            /*g_spawn_async creates a new asynchronous process which means it forks a new one and then immideately returns to
            the original process to allow further code execution
            1. NULL - means that it'll just run in the same directory as the parent
            2. gchar *argv[] - see top comments
            3. NULL - child inherits same env variables from parent
            4. G_SPAWN_DO_NOT_REAP_CHILD - makes the programmer responsible for cleaning up whatever the child process left behind
            5. NULL - no setup function will be ran in the chld process before executing the cmd
            6. NULL - and since there is no setup function we can't pass any data to it
            7. &child_pid - this variable will store the pid of the child process
            8. &err - error information will be store here if anything fails*/
        
            error("An error occurred - creation of child process failed miserably");
            gtk_main_quit();
        } else {
            g_child_watch_add(child_pid, child, NULL);
        }
    } else {
        error("An error occurred - Enter a name");
    }
}
void destroy(GtkWidget* widget, gpointer data) {
    gtk_main_quit();
}

GList* get_programs() {
    /*this function scans the system's PATH directories to find all executable files and returns them as a sorted list, 
     opens each directory in the PATH, checks each file to see if it is executable, and if so, adds it to le list.*/

    GList *programs = NULL; //initialize a list which is empty used for holding the program names
    const gchar *path_env = g_getenv("PATH"); //get PATH
    gchar **paths = g_strsplit(path_env, ":", -1); // split the PATH variable into individual directories
    
    for (gchar **path = paths; *path != NULL; ++path) {
        GDir *dir = g_dir_open(*path, 0, NULL);
        if (dir) {
            const gchar *filename;
            while ((filename = g_dir_read_name(dir)) != NULL) {
                gchar *filepath = g_build_filename(*path, filename, NULL); // Construct the full path to the file
                /*check if file is executable and if yes add to the list*/
                if (g_file_test(filepath, G_FILE_TEST_IS_EXECUTABLE)) {
                    programs = g_list_prepend(programs, g_strdup(filename));
                }
                g_free(filepath); 
            }
            g_dir_close(dir);
        }
    }

    g_strfreev(paths); //free the array of paths and autisticly organize everything in a case sensitive manner!!
    return g_list_sort(programs, (GCompareFunc)g_ascii_strcasecmp);
    
}

GtkEntryCompletion* setup_autocomplete() {
    //we making it to a mental asylum with this one !!!
    //https://www.youtube.com/watch?v=XMCVCJ8ZuPs
    //const gchar *program = gtk_entry_get_text(GTK_ENTRY(entry)); // acquire the text from entryw idget
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    //create a list with one column (the autocomplete window)
    GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
    GList *programs = get_programs(); 
    //get the programs and iterate over eacjh program in the list
    for (GList *l = programs; l != NULL; l = l->next) {
        GtkTreeIter iter;
        //ad a new row to the list
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, (gchar *)l->data, -1);
        g_free(l->data);
    }
    g_list_free(programs);

    //set the model for the completion obj.
    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
    g_object_unref(store); // <-- and unreference it here because it's now being managed by the completion obj.

    gtk_entry_completion_set_text_column(completion, 0);
    gtk_entry_completion_set_popup_set_width(completion, TRUE);
    gtk_entry_completion_set_inline_completion(completion, FALSE);
    gtk_entry_completion_set_popup_completion(completion, FALSE);

    return completion;
}




gboolean escExit(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    //allow the application to exit when esc is pressed
    if (event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
        return TRUE; 
    }

    return FALSE; 
}

void browse(GtkWidget* widget, GtkWidget* window) {
    //make a file chooser diualogue
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);
    // run the dialog, and check if the user has selected a file
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        //paste the name of the file into the entry field
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(entry), filename);
        g_free(filename);
    }
    //we done here so we destroy the dialogue
    gtk_widget_destroy(dialog);
}

gboolean completenow(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    GtkEntryCompletion *completion = GTK_ENTRY_COMPLETION(user_data);
    gtk_entry_completion_complete(completion);
    return TRUE;  // Allow further handling
}
//gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    GtkEntryCompletion *completion = GTK_ENTRY_COMPLETION(user_data);
    gtk_entry_set_completion(GTK_ENTRY(widget), completion);
    //gtk_entry_completion_set_inline_completion(completion, TRUE);
    //gtk_entry_completion_set_popup_completion(completion, TRUE);
        gtk_entry_completion_complete(completion);
    if (event->keyval == GDK_KEY_Tab) {
        gtk_entry_completion_set_inline_completion(completion, TRUE);
        gtk_entry_completion_set_popup_completion(completion, TRUE);
        completenow(widget, event, user_data);
        //gtk_entry_completion_complete(completion);
        return TRUE;  // Return TRUE to stop further handling
    }
    return FALSE;  // Allow further handling
}

////~~millions must alt~~
//nuh millions must tab
/* gboolean altAutocomplete(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
     static gboolean tab_pressed = FALSE; //track the state of the alt key
    GtkEntryCompletion *completion = GTK_ENTRY_COMPLETION(user_data);

    if (event->type == GDK_KEY_PRESS) {
        //if alt-l or alt-r is pressed, 
        if (event->keyval == GDK_KEY_Tab) {
            if ( tab_pressed = TRUE ) {
	            tab_pressed = TRUE;
	            //trigger autocompletion
	            gtk_entry_set_completion(GTK_ENTRY(widget), completion);
	            gtk_entry_completion_complete(completion);
	            return TRUE; //event handled
            } else {
		    tab_pressed = FALSE;
                    gtk_entry_set_completion(GTK_ENTRY(widget), NULL);
                    return TRUE;
        }
    }
    }

    return FALSE; //event not handled
} */

////~~millions must alt~~
//nuh millions must tab
gboolean altAutocomplete(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
     static gboolean tab_pressed = FALSE; //track the state of the alt key
    GtkEntryCompletion *completion = GTK_ENTRY_COMPLETION(user_data);

    if (event->type == GDK_KEY_PRESS) {
        //if alt-l or alt-r is pressed, 
        if (event->keyval == GDK_KEY_Tab) {
            if ( tab_pressed = TRUE ) {
	            tab_pressed = TRUE;
	            //trigger autocompletion
	            gtk_entry_set_completion(GTK_ENTRY(widget), completion);
	            gtk_entry_completion_complete(completion);
	            return TRUE; //event handled
            } else {
		    tab_pressed = FALSE;
                    gtk_entry_set_completion(GTK_ENTRY(widget), NULL);
                    return TRUE;
        }
    }
    }

    return FALSE; //event not handled
}
  
  
int main(int argc, char* argv[]) 
{ 
    GdkScreen *screen;
    GtkWidget* button;
	GtkWidget* button2;
	GtkWidget* button3;
	GtkWidget* fixed;
	GtkWidget* text_open;
    GtkWidget* image;
    GtkWidget* instruction1;
    GtkWidget* instruction2;
	
	version();

    gtk_init(&argc, &argv); 
	
    screen = gdk_screen_get_default();
    gint screen_height = gdk_screen_get_height(screen);

   gint x = 12, y = screen_height - WINDOW_HEIGHT - 60;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_icon_from_file(window, g_build_filename(getprogpath(argv[0]), "run_icon_16.png", NULL), NULL);
    gtk_window_set_title(GTK_WINDOW(window), "Run");
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);

    gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_NORTH_EAST);
    gtk_window_move(GTK_WINDOW(window), x, y);
  
    g_signal_connect(window, "destroy", 
                     G_CALLBACK(destroy), NULL); 
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);	

	gtk_widget_set_size_request(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);
  

    gtk_window_set_resizable(window, FALSE);
    // make a fixed container
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);
	
	//create text
	text_open = gtk_label_new("Open:");
    instruction1 = gtk_label_new("Type the name of a program, folder, document, or");
    instruction2 = gtk_label_new("Internet Resource, and Ice2K.sys will open it for you.");
	
	// create the entry widget
    entry = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), entry, 50, 60);
    gtk_widget_set_size_request(entry, 279, 21);
	
	//create the buttons  
    button = gtk_button_new_with_label("OK");
	button2 = gtk_button_new_with_label("Cancel");
	button3 = gtk_button_new_with_label("Browse...");
	
    GtkEntryCompletion *completion = setup_autocomplete();
	            //gtk_entry_set_completion(GTK_ENTRY(widget), completion);

    g_signal_connect(entry, "key-press-event", G_CALLBACK(on_key_press), completion);

    // configure the button size...
    gtk_widget_set_size_request(button, BUTTON_WIDTH, BUTTON_HEIGHT);
	gtk_widget_set_size_request(button2, BUTTON_WIDTH, BUTTON_HEIGHT);
	gtk_widget_set_size_request(button3, BUTTON_WIDTH, BUTTON_HEIGHT);
	
    // ...and add the buttone at the right position
    gtk_fixed_put(GTK_FIXED(fixed), button, 100, BUTTON_Y);
	gtk_fixed_put(GTK_FIXED(fixed), button2, 180, BUTTON_Y);
	gtk_fixed_put(GTK_FIXED(fixed), button3, 258, BUTTON_Y);

	//add "open" text and add the instructions
    gtk_fixed_put(GTK_FIXED(fixed), text_open, 10, 63);
    gtk_fixed_put(GTK_FIXED(fixed), instruction1, 53, 21);
    gtk_fixed_put(GTK_FIXED(fixed), instruction2, 53, 34);

	// construct the full path to the image file
    /*okay the reason i did it this way was, because running ice2krun via the icewm menu didnt display
    the image properly. instead it just displayed the standard replacement image for when an image isnt
    found. so i have to construct the path here in order to make the image appear when running via the
    icewm menu. this isn't the case when running via terminal. i have no idea why this is the case and i
    don't want to know honestly*/
    printf(getprogpath(argv[0]));
    printf("\n");
    gchar *image_path = g_build_filename(getprogpath(argv[0]), "run_icon.png", NULL);

    //printf("Image path: %s\n", image_path); 

    if (g_file_test(image_path, G_FILE_TEST_EXISTS)) {
        image = gtk_image_new_from_file(image_path);
    } else {
        printf("Image file does not exist: %s\n", image_path);
        image = gtk_image_new_from_icon_name("image-missing", GTK_ICON_SIZE_DIALOG);
    }

    g_free(image_path);

    gtk_fixed_put(GTK_FIXED(fixed), image, 11, 18);

	// link ok-button to run-functio
    g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(run), window);
    g_signal_connect(GTK_OBJECT(button2), "clicked", G_CALLBACK(destroy), NULL);
    //link enter button to same function as ok
    g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(run), window);
    g_signal_connect(entry, "key-press-event", G_CALLBACK(escExit), NULL);
    g_signal_connect(GTK_OBJECT(button3), "clicked", G_CALLBACK(browse), window);
    
    gtk_widget_show_all(window); 

    gtk_main(); 
  
    return 0; 
} 

/* 
 * Copyright (C) 2002 Red Hat, Inc.
 * Copyright (C) 2009 Julien Danjou <julien@danjou.info>
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <config.h>
#include "sn-common.h"
#include "sn-internals.h"

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>

#include <X11/Xlib-xcb.h>

struct SnDisplay
{
  int refcount;
  Display *xdisplay;
  xcb_connection_t *xconnection;
  xcb_screen_t **screens;
  xcb_atom_t UTF8_STRING, NET_STARTUP_ID,
    NET_STARTUP_INFO, NET_STARTUP_INFO_BEGIN;
  SnDisplayErrorTrapPush push_trap_func;
  SnDisplayErrorTrapPop  pop_trap_func;
  SnXcbDisplayErrorTrapPush xcb_push_trap_func;
  SnXcbDisplayErrorTrapPop  xcb_pop_trap_func;
  int n_screens;
  SnList *xmessage_funcs;
  SnList *pending_messages;
};

/**
 * sn_display_new:
 * @xdisplay: an X window system display
 * @push_trap_func: function to push an X error trap
 * @pop_trap_func: function to pop an X error trap
 * 
 * Creates a new #SnDisplay object, containing
 * data that libsn associates with an X display.
 *
 * @push_trap_func should be a function that causes X errors to be
 * ignored until @pop_trap_func is called as many times as
 * @push_trap_func has been called. (Nested push/pop pairs must be
 * supported.) The outermost @pop_trap_func in a set of nested pairs
 * must call XSync() to ensure that all errors that will occur have in
 * fact occurred. These functions are used to avoid X errors due to
 * BadWindow and such.
 * 
 * Return value: the new #SnDisplay
 **/
SnDisplay*
sn_display_new (Display                *xdisplay,
                SnDisplayErrorTrapPush  push_trap_func,
                SnDisplayErrorTrapPop   pop_trap_func)
{
  SnDisplay *display = sn_xcb_display_new(XGetXCBConnection(xdisplay),
                                          NULL, NULL);

  display->xdisplay = xdisplay;
  display->push_trap_func = push_trap_func;
  display->pop_trap_func = pop_trap_func;

  return display;
}


/**
 * sn_xcb_display_new:
 * @xdisplay: an X window system connection
 * @push_trap_func: function to push an X error trap
 * @pop_trap_func: function to pop an X error trap
 *
 * Creates a new #SnDisplay object, containing
 * data that libsn associates with an X connection.
 *
 * @push_trap_func should be a function that causes X errors to be
 * ignored until @pop_trap_func is called as many times as
 * @push_trap_func has been called. (Nested push/pop pairs must be
 * supported.)
 * These functions are used to avoid X errors due to BadWindow and
 * such.
 *
 * Return value: the new #SnDisplay
 **/
SnDisplay*
sn_xcb_display_new (xcb_connection_t          *xconnection,
                    SnXcbDisplayErrorTrapPush  push_trap_func,
                    SnXcbDisplayErrorTrapPop   pop_trap_func)
{
  SnDisplay *display;
  int i;

  /* Get all atoms we will need in the future */
  xcb_intern_atom_cookie_t atom_utf8_string_c =
    xcb_intern_atom(xconnection, FALSE,
                    sizeof("UTF8_STRING") - 1, "UTF8_STRING");

  xcb_intern_atom_cookie_t atom_net_startup_info_begin_c =
    xcb_intern_atom(xconnection, FALSE,
                    sizeof("_NET_STARTUP_INFO_BEGIN") - 1,
                    "_NET_STARTUP_INFO_BEGIN");

  xcb_intern_atom_cookie_t atom_net_startup_info_c =
    xcb_intern_atom(xconnection, FALSE,
                    sizeof("_NET_STARTUP_INFO") - 1, "_NET_STARTUP_INFO");

  xcb_intern_atom_cookie_t atom_net_startup_id_c =
    xcb_intern_atom(xconnection, FALSE,
                    sizeof("_NET_STARTUP_ID") - 1, "_NET_STARTUP_ID");

  display = sn_new0 (SnDisplay, 1);

  display->xconnection = xconnection;
  display->n_screens = xcb_setup_roots_length (xcb_get_setup (xconnection));
  display->screens = sn_new (xcb_screen_t*, display->n_screens);
  display->refcount = 1;

  display->xcb_push_trap_func = push_trap_func;
  display->xcb_pop_trap_func = pop_trap_func;

  for (i = 0; i < display->n_screens; ++i)
    display->screens[i] = xcb_aux_get_screen(xconnection, i);

  xcb_intern_atom_reply_t *atom_reply;

  atom_reply = xcb_intern_atom_reply(display->xconnection,
                                     atom_utf8_string_c,
                                     NULL);
  display->UTF8_STRING = atom_reply->atom;
  free(atom_reply);

  atom_reply = xcb_intern_atom_reply(display->xconnection,
                                     atom_net_startup_info_begin_c,
                                     NULL);
  display->NET_STARTUP_INFO_BEGIN = atom_reply->atom;
  free(atom_reply);

  atom_reply = xcb_intern_atom_reply(display->xconnection,
                                     atom_net_startup_info_c,
                                     NULL);
  display->NET_STARTUP_INFO = atom_reply->atom;
  free(atom_reply);

  atom_reply = xcb_intern_atom_reply(display->xconnection,
                                     atom_net_startup_id_c,
                                     NULL);
  display->NET_STARTUP_ID = atom_reply->atom;
  free(atom_reply);

  return display;
}



/**
 * sn_display_ref:
 * @display: an #SnDisplay
 * 
 * Increment the reference count for @display
 **/
void
sn_display_ref (SnDisplay *display)
{
  display->refcount += 1;
}

/**
 * sn_display_unref:
 * @display: an #SnDisplay
 * 
 * Decrement the reference count for @display, freeing
 * display if the reference count reaches zero.
 **/
void
sn_display_unref (SnDisplay *display)
{
  display->refcount -= 1;
  if (display->refcount == 0)
    {
      if (display->xmessage_funcs)
        sn_list_free (display->xmessage_funcs);
      if (display->pending_messages)
        sn_list_free (display->pending_messages);
      sn_free (display->screens);
      sn_free (display);
    }
}

/**
 * sn_display_get_x_display:
 * @display: an #SnDisplay
 * This function only returns a value if the SnDisplay
 * has been created with sn_display_new().
 * 
 * 
 * 
 * Return value: X display for this #SnDisplay
 **/
Display*
sn_display_get_x_display (SnDisplay *display)
{
  return display->xdisplay;
}

/**
 * sn_display_get_x_connection:
 * @display: an #SnDisplay
 * This function only returns a value if the SnDisplay
 * has been created with sn_xcb_display_new().
 *
 *
 *
 * Return value: X connection for this #SnDisplay
 **/
xcb_connection_t*
sn_display_get_x_connection(SnDisplay *display)
{
  return display->xconnection;
}

/**
 * sn_internal_display_get_id:
 * @display: an #SnDisplay
 *
 *
 *
 * Return value: X display id.
 **/
void *
sn_internal_display_get_id (SnDisplay *display)
{
  return display->xconnection;
}

/**
 * sn_internal_display_get_x_screen:
 * @display: an #SnDisplay
 * @number: screen number to get
 *
 * Gets a screen by number; if the screen number
 * does not exist, returns %NULL.
 *
 * Return value: X screen or %NULL
 **/
xcb_screen_t*
sn_internal_display_get_x_screen (SnDisplay *display,
                                  int        number)
{
  if (number < 0 || number >= display->n_screens)
    return NULL;
  else
    return display->screens[number];
}

/**
 * sn_internal_display_get_root_window:
 * @display: an #SnDisplay
 * @number: screen number to get root window from
 *
 * Gets a root window; if the screen number
 * does not exist, returns %NULL.
 *
 * Return value: X root window or %NULL
 **/
xcb_window_t
sn_internal_display_get_root_window (SnDisplay *display,
                                     int       number)
{
    if (number >= 0 && number < display->n_screens)
        return display->screens[number]->root;
    return None;
}

/**
 * sn_internal_display_get_screen_number:
 * @display an #SnDisplay
 *
 *
 *
 * Return value: The number of screen for this #SnDisplay
 **/
int
sn_internal_display_get_screen_number (SnDisplay *display)
{
    return display->n_screens;
}

/**
 * sn_display_process_event:
 * @display: a display
 * @xevent: X event
 * 
 * libsn should be given a chance to see all X events by passing them
 * to this function. If the event was a property notify or client
 * message related to the launch feedback protocol, the
 * sn_display_process_event() returns true. Calling
 * sn_display_process_event() is not currently required for launchees,
 * only launchers and launch feedback displayers. The function returns
 * false for mapping, unmapping, window destruction, and selection
 * events even if they were involved in launch feedback.
 * 
 * Return value: true if the event was a property notify or client message involved in launch feedback
 **/
sn_bool_t
sn_display_process_event (SnDisplay *display,
                          XEvent    *xevent)
{
  sn_bool_t retval;

  retval = FALSE;

  if (sn_internal_monitor_process_event (display))
    retval = TRUE;

  switch(xevent->xany.type)
  {
    case ClientMessage:
      if (sn_internal_xmessage_process_client_message (display,
                                                       xevent->xclient.window,
                                                       xevent->xclient.message_type,
                                                       xevent->xclient.data.b))
          retval = TRUE;
      break;
    default:
      break;
  }

  return retval;
}

/**
 * sn_xcb_display_process_event:
 * @display: a display
 * @xevent: X event
 *
 * libsn should be given a chance to see all X events by passing them
 * to this function. If the event was a property notify or client
 * message related to the launch feedback protocol, the
 * sn_display_process_event() returns true. Calling
 * sn_display_process_event() is not currently required for launchees,
 * only launchers and launch feedback displayers. The function returns
 * false for mapping, unmapping, window destruction, and selection
 * events even if they were involved in launch feedback.
 *
 * Return value: true if the event was a property notify or client message involved in launch feedback
 **/
sn_bool_t
sn_xcb_display_process_event (SnDisplay           *display,
                              xcb_generic_event_t *xevent)
{
  sn_bool_t retval;

  retval = FALSE;

  if (sn_internal_monitor_process_event (display))
    retval = TRUE;

  switch(XCB_EVENT_RESPONSE_TYPE(xevent))
  {
    case XCB_CLIENT_MESSAGE:
      {
        xcb_client_message_event_t *ev = (xcb_client_message_event_t *) xevent;
        if (sn_internal_xmessage_process_client_message (display,
                                                         ev->window,
                                                         ev->type,
                                                         (const char *) ev->data.data8))
          retval = TRUE;
      }
      break;
    default:
      break;
  }

  return retval;
}

/**
 * sn_display_error_trap_push:
 * @display: a display
 *
 *  Calls the push_trap_func from sn_display_new() if non-NULL.
 **/
void
sn_display_error_trap_push (SnDisplay *display)
{
  /* SnDisplay has been created for Xlib */
  if (display->xdisplay)
  {
    if (display->push_trap_func)
      (* display->push_trap_func) (display, display->xdisplay);
  }
  else
  {
    if (display->xcb_push_trap_func)
      (* display->xcb_push_trap_func) (display, display->xconnection);
  }
}

/**
 * sn_display_error_trap_pop:
 * @display: a display
 *
 *  Calls the pop_trap_func from sn_display_new() if non-NULL.
 **/
void
sn_display_error_trap_pop  (SnDisplay *display)
{
  /* SnDisplay has been created for Xlib */
  if (display->xdisplay)
  {
    if (display->pop_trap_func)
      (* display->pop_trap_func) (display, display->xdisplay);
  }
  else
  {
    if (display->xcb_pop_trap_func)
      (* display->xcb_pop_trap_func) (display, display->xconnection);
  }
}

void
sn_internal_display_get_xmessage_data (SnDisplay              *display,
                                       SnList                **funcs,
                                       SnList                **pending)
{
  if (display->xmessage_funcs == NULL)
    display->xmessage_funcs = sn_list_new ();

  if (display->pending_messages == NULL)
    display->pending_messages = sn_list_new ();
  
  if (funcs)
    *funcs = display->xmessage_funcs;
  if (pending)
    *pending = display->pending_messages;
}

xcb_atom_t
sn_internal_get_utf8_string_atom(SnDisplay *display)
{
  return display->UTF8_STRING;
}

xcb_atom_t
sn_internal_get_net_startup_id_atom(SnDisplay *display)
{
  return display->NET_STARTUP_ID;
}

xcb_atom_t
sn_internal_get_net_startup_info_atom(SnDisplay *display)
{
  return display->NET_STARTUP_INFO;
}

xcb_atom_t
sn_internal_get_net_startup_info_begin_atom(SnDisplay *display)
{
  return display->NET_STARTUP_INFO_BEGIN;
}

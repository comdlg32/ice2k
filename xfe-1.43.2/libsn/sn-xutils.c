/*
 * Copyright (C) 2002 Red Hat, Inc.
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
#include "sn-internals.h"

#include <xcb/xcb.h>

void
sn_internal_set_utf8_string (SnDisplay  *display,
                             xcb_window_t xwindow,
                             xcb_atom_t  property,
                             const char *str)
{
  sn_display_error_trap_push (display);

  xcb_connection_t *c = sn_display_get_x_connection (display);
  xcb_atom_t UTF8_STRING = sn_internal_get_utf8_string_atom(display);

  xcb_change_property (c,
                       XCB_PROP_MODE_REPLACE,
                       xwindow,
                       property,
                       UTF8_STRING,
                       8, strlen (str), str);

  sn_display_error_trap_pop (display);
}

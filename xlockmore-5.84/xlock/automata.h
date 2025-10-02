#ifndef _AUTOMATA_H_
#define _AUTOMATA_H_

#if 0
/* #ident	"@(#)automata.h	4.14 99/06/17 xlockmore" */

#endif

/*-
 * Cellular Automata stuff
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 17-06-99: Started log. :)
 */

extern XPoint hexagonUnit[6];
extern XPoint triangleUnit[2][3];
extern XPoint pentagonUnit[4][5];

#define NUMSTIPPLES 11
#define STIPPLESIZE 8
extern unsigned char stipples[NUMSTIPPLES][STIPPLESIZE];

#endif /* _AUTOMATA_H_ */

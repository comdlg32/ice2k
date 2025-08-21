/*
MIT/X Consortium License

© 2009-2012 Aurélien APTEL <aurelien dot aptel at gmail dot com>
© 2009 Anselm R Garbe <garbeam at gmail dot com>
© 2012-2015 Roberto E. Vargas Caballero <k0ga at shike2 dot com>
© 2012-2015 Christoph Lohmann <20h at r-36 dot net>
© 2013 Eon S. Jeon <esjeon at hyunmu dot am>
© 2013 Alexander Sedov <alex0player at gmail dot com>
© 2013 Mark Edgar <medgar123 at gmail dot com>
© 2013 Eric Pruitt <eric.pruitt at gmail dot com>
© 2013 Michael Forney <mforney at mforney dot org>
© 2013-2014 Markus Teich <markus dot teich at stusta dot mhn dot de>
© 2014 Laslo Hunhold <dev at frign dot de>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


#ifndef ARG_H__
#define ARG_H__

extern char *argv0;

/* use main(int argc, char *argv[]) */
#define ARGBEGIN	for (argv0 = *argv, argv++, argc--;\
					argv[0] && argv[0][1]\
					&& argv[0][0] == '-';\
					argc--, argv++) {\
				char argc_;\
				char **argv_;\
				int brk_;\
				if (argv[0][1] == '-' && argv[0][2] == '\0') {\
					argv++;\
					argc--;\
					break;\
				}\
				for (brk_ = 0, argv[0]++, argv_ = argv;\
						argv[0][0] && !brk_;\
						argv[0]++) {\
					if (argv_ != argv)\
						break;\
					argc_ = argv[0][0];\
					switch (argc_)

/* Handles obsolete -NUM syntax */
#define ARGNUM				case '0':\
					case '1':\
					case '2':\
					case '3':\
					case '4':\
					case '5':\
					case '6':\
					case '7':\
					case '8':\
					case '9'

#define ARGEND			}\
			}

#define ARGC()		argc_

#define ARGNUMF(base)	(brk_ = 1, estrtol(argv[0], (base)))

#define EARGF()	((argv[0][1] == '\0' && argv[1] == NULL)?\
				((exit(EXIT_FAILURE)), abort(), (char *)0) :\
				(brk_ = 1, (argv[0][1] != '\0')?\
					(&argv[0][1]) :\
					(argc--, argv++, argv[0])))

#define ARGF()		((argv[0][1] == '\0' && argv[1] == NULL)?\
				(char *)0 :\
				(brk_ = 1, (argv[0][1] != '\0')?\
					(&argv[0][1]) :\
					(argc--, argv++, argv[0])))

#endif

#ifndef XFEUTILS_H
#define XFEUTILS_H

// The functions comparenat(), comparewnat(), comparenat_left(), comparenat_right()
// comparewnat_left() and comparewnat_right() for natural sort order
// are adapted from the following software:

/*
 * strnatcmp.c -- Perform 'natural order' comparisons of strings in C.
 * Copyright (C) 2000, 2004 by Martin Pool <mbp sourcefrog net>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software
 *   in a product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *   misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

// The convaccents() function and the accents table are adapted from
// code found here: http://rosettacode.org/wiki/Natural_sorting


#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <wctype.h>
#include <vector>

#include <fx.h>


// Global variables
#if defined(linux)
extern FXStringDict* mtdevices;
extern FXStringDict* updevices;
#endif


// Vector of strings
typedef std::vector<FXString>   vector_FXString;


// Single click types
enum
{
    SINGLE_CLICK_NONE,
    SINGLE_CLICK_DIR,
    SINGLE_CLICK_DIR_FILE,
};


// Wait cursor states
enum
{
    BEGIN_CURSOR,
    END_CURSOR,
    QUERY_CURSOR
};


// Indexes of default programs
enum
{
    NONE            = 0,
    TXTVIEWER       = 1,
    TXTEDITOR       = 2,
    IMGVIEWER       = 3,
    IMGEDITOR       = 4,
    PDFVIEWER       = 5,
    AUDIOPLAYER     = 6,
    VIDEOPLAYER     = 7,
    ARCHIVER        = 8,
};


// Start directory modes
enum
{
    START_HOMEDIR       = 0,
    START_CURRENTDIR    = 1,
    START_LASTDIR       = 2,
};


// Note : some inline functions must be declared in the header file or they won't compile!


static inline int comparenat_right(const char* a, const char* b)
{
    int bias = 0;

    /* The longest run of digits wins.  That aside, the greatest
     * value wins, but we can't know that it will until we've scanned
     * both numbers to know that they have the same magnitude, so we
     * remember it in BIAS. */
    for ( ; ; a++, b++)
    {
        if (!isdigit(*a) && !isdigit(*b))
        {
            return(bias);
        }
        else if (!isdigit(*a))
        {
            return(-1);
        }
        else if (!isdigit(*b))
        {
            return(+1);
        }
        else if (*a < *b)
        {
            if (!bias)
            {
                bias = -1;
            }
        }
        else if (*a > *b)
        {
            if (!bias)
            {
                bias = +1;
            }
        }
        else if (!*a && !*b)
        {
            return(bias);
        }
    }

    return(0);
}


static inline int comparenat_left(const char* a, const char* b)
{
    /* Compare two left-aligned numbers: the first to have a
     * different value wins. */
    for ( ; ; a++, b++)
    {
        if (!isdigit(*a) && !isdigit(*b))
        {
            return(0);
        }
        else if (!isdigit(*a))
        {
            return(-1);
        }
        else if (!isdigit(*b))
        {
            return(+1);
        }
        else if (*a < *b)
        {
            return(-1);
        }
        else if (*a > *b)
        {
            return(+1);
        }
    }

    return(0);
}


// Perform natural comparison on single byte strings (so foo10 comes after foo2, 0.2foo comes before 10.2foo, etc.)
static inline int comparenat(const char* a, const char* b, FXbool igncase)
{
    int  ai, bi;
    char ca, cb;
    int  fractional, result;

    ai = bi = 0;
    while (1)
    {
        ca = a[ai];
        cb = b[bi];

        if ((ca == '\t') && (cb == '\t'))
        {
            return(0);
        }

        /* skip over leading spaces or zeros */
        while (isspace(ca))
        {
            ca = a[++ai];
        }

        while (isspace(cb))
        {
            cb = b[++bi];
        }

        /* process run of digits */
        if (isdigit(ca) && isdigit(cb))
        {
            fractional = (ca == '0' || cb == '0');

            if (fractional)
            {
                if ((result = comparenat_left(a+ai, b+bi)) != 0)
                {
                    return(result);
                }
            }
            else
            {
                if ((result = comparenat_right(a+ai, b+bi)) != 0)
                {
                    return(result);
                }
            }
        }

        if (!ca && !cb)
        {
            /* The strings compare the same.  Perhaps the caller
             *     will want to call strcmp to break the tie. */
            return(0);
        }

        if (igncase)
        {
            ca = tolower(ca);
            cb = tolower(cb);
        }

        if (ca < cb)
        {
            return(-1);
        }
        else if (ca > cb)
        {
            return(+1);
        }

        ++ai;
        ++bi;
    }
}


// Lookup table of accents and ligatures
// For comparisons, À is converted to A, é to e, etc.
static const wchar_t* const accents[] =   /* copied from Perl6 code */
{
    L"À", L"A", L"Á", L"A", L"Â", L"A", L"Ã", L"A", L"Ä", L"A", L"Å", L"A", L"à",
    L"a", L"á", L"a", L"â", L"a", L"ã", L"a", L"ä", L"a", L"å", L"a",
    L"Ç", L"C", L"ç", L"c", L"È", L"E", L"É", L"E", L"Ê", L"E", L"Ë",
    L"E", L"è", L"e", L"é", L"e", L"ê", L"e", L"ë", L"e", L"Ì",
    L"I", L"Í", L"I", L"Î", L"I", L"Ï", L"I", L"ì", L"i", L"í",
    L"i", L"î", L"i", L"ï", L"i", L"Ò", L"O", L"Ó", L"O", L"Ô",
    L"O", L"Õ", L"O", L"Ö", L"O", L"Ø", L"O", L"ò", L"o", L"ó", L"o",
    L"ô", L"o", L"õ", L"o", L"ö", L"o", L"ø", L"o", L"Ñ", L"N", L"ñ", L"n",
    L"Ù", L"U", L"Ú", L"U", L"Û", L"U", L"Ü", L"U", L"ù", L"u", L"ú", L"u",
    L"û", L"u", L"ü", L"u", L"Ý", L"Y", L"ÿ", L"y", L"ý", L"y",
    L"Þ", L"TH", L"þ", L"th", L"Ð", L"TH", L"ð", L"th",
    L"Æ", L"AE", L"æ", L"ae", L"ß", L"ss",
    L"ﬄ", L"ffl", L"ﬃ", L"ffi", L"ﬁ", L"fi", L"ﬀ", L"ff", L"ﬂ", L"fl",
    L"ſ", L"s", L"ʒ", L"z", L"ﬆ", L"st", L"œ", L"oe", /* ... come on ... */
};


// Convert accents and ligatures to Ascii for comparison purpose
// So when comparing wide chars, À is evaluated as A, é as e, etc.
static inline wchar_t convaccents(const wchar_t wc, const wchar_t* const* tbl, int len)
{
    // Don't convert an Ascii char
    if (wc < 127)
    {
        return(wc);
    }

    wchar_t wr = wc;

    // Search the lookup table
    // and get the converted char if any
    for (int n = 0; n < len; n += 2)
    {
        if (wc != tbl[n][0])
        {
            continue;
        }
        else
        {
            wr = tbl[n+1][0];
            break;
        }
    }

    return(wr);
}


static inline int comparewnat_right(const wchar_t* wa, const wchar_t* wb)
{
    int bias = 0;

    /* The longest run of digits wins.  That aside, the greatest
     * value wins, but we can't know that it will until we've scanned
     * both numbers to know that they have the same magnitude, so we
     * remember it in BIAS. */
    for ( ; ; wa++, wb++)
    {
        if (!iswdigit(*wa) && !iswdigit(*wb))
        {
            return(bias);
        }
        else if (!iswdigit(*wa))
        {
            return(-1);
        }
        else if (!iswdigit(*wb))
        {
            return(+1);
        }
        else if (*wa < *wb)
        {
            if (!bias)
            {
                bias = -1;
            }
        }
        else if (*wa > *wb)
        {
            if (!bias)
            {
                bias = +1;
            }
        }
        else if (!*wa && !*wb)
        {
            return(bias);
        }
    }

    return(0);
}


static inline int comparewnat_left(const wchar_t* wa, const wchar_t* wb)
{
    /* Compare two left-aligned numbers: the first to have a
     * different value wins. */
    for ( ; ; wa++, wb++)
    {
        if (!iswdigit(*wa) && !iswdigit(*wb))
        {
            return(0);
        }
        else if (!iswdigit(*wa))
        {
            return(-1);
        }
        else if (!iswdigit(*wb))
        {
            return(+1);
        }
        else if (*wa < *wb)
        {
            return(-1);
        }
        else if (*wa > *wb)
        {
            return(+1);
        }
    }

    return(0);
}


// Perform natural comparison on wide strings (so foo10 comes after foo2, 0.2foo comes before 10.2foo, etc.)
static inline int comparewnat(const wchar_t* wa, const wchar_t* wb, int igncase)
{
    wint_t  ai, bi;
    wchar_t wca, wcb;
    int     fractional, result;

    ai = bi = 0;
    while (1)
    {
        wca = wa[ai];
        wcb = wb[bi];

        /* skip over leading spaces or zeros */
        while (iswspace(wca))
        {
            wca = wa[++ai];
        }

        while (iswspace(wcb))
        {
            wcb = wb[++bi];
        }

        /* convert accents  */
        wca = convaccents(wca, accents, sizeof(accents)/sizeof(wchar_t*));
        wcb = convaccents(wcb, accents, sizeof(accents)/sizeof(wchar_t*));

        /* process run of digits */
        if (iswdigit(wca) && iswdigit(wcb))
        {
            fractional = (wca == L'0' || wcb == L'0');

            if (fractional)
            {
                if ((result = comparewnat_left(wa+ai, wb+bi)) != 0)
                {
                    return(result);
                }
            }
            else
            {
                if ((result = comparewnat_right(wa+ai, wb+bi)) != 0)
                {
                    return(result);
                }
            }
        }

        if (!wca && !wcb)
        {
            /* The strings compare the same.  Perhaps the caller
             *     will want to call strcmp to break the tie. */
            return(0);
        }

        if (igncase)
        {
            wca = towlower(wca);
            wcb = towlower(wcb);
        }

        if (wca < wcb)
        {
            return(-1);
        }
        else if (wca > wcb)
        {
            return(+1);
        }

        ++ai;
        ++bi;
    }
}


// Convert a character to lower case
static inline int toLower(int c)
{
    return('A' <= c && c <= 'Z' ? c + 32 : c);
}


// To test if two strings are equal (strcmp replacement, thanks to Francesco Abbate)
static inline int streq(const char* a, const char* b)
{
    if ((a == NULL) || (b == NULL))
    {
        return(0);
    }
    return(strcmp(a, b) == 0);
}


// Convert a string to lower cases and returns the string size
static inline void strlow(char* str)
{
    while (*str)
    {
        *str = ::toLower(*str);
        ++str;
    }
}


// Replacement of the stat function
static inline int statrep(const char* filename, struct stat* buf)
{
#if defined(linux)
    static int ret;

    // It's a mount point
    if (mtdevices->find(filename))
    {
        // Mount point is down
        if (streq(updevices->find(filename), "down"))
        {
            return(-1);
        }

        // Mount point is up
        else
        {
            ret = stat(filename, buf);
            if ((ret == -1) && (errno != EACCES))
            {
                updevices->remove(filename);
                updevices->insert(filename, "down");
            }
            return(ret);
        }
    }

    // It's not a mount point
    else
#endif
    return(stat(filename, buf));
}


// Replacement of the lstat function
static inline int lstatrep(const char* filename, struct stat* buf)
{
#if defined(linux)
    static int ret;

    // It's a mount point
    if (mtdevices->find(filename))
    {
        // Mount point is down
        if (streq(updevices->find(filename), "down"))
        {
            return(-1);
        }

        // Mount point is up
        else
        {
            ret = lstat(filename, buf);
            if ((ret == -1) && (errno != EACCES))
            {
                updevices->remove(filename);
                updevices->insert(filename, "down");
            }
            return(ret);
        }
    }

    // It's not a mount point
    else
#endif
    return(lstat(filename, buf));
}


FXHotKey _parseAccel(const FXString&);
FXbool existCommand(const FXString);
FXString getKeybinding(FXEvent*);

int mkpath(const char*, mode_t);
FXString createTrashpathname(FXString, FXString);
int createTrashinfo(FXString, FXString, FXString, FXString);
FXString mimetype(FXString);
FXString quote(FXString);
FXbool isUtf8(const char*, FXuint);
int statrep(const char*, struct stat*);
int lstatrep(const char*, struct stat*);

#if defined(linux)
int lstatmt(const char*, struct stat*);

#endif
size_t strlcpy(char*, const char*, size_t);
size_t strlcat(char*, const char*, size_t);
FXulong dirsize(const char*);
FXulong pathsize(char*, FXuint*, FXuint*, FXulong*, int* =  NULL);
FXString hSize(char*);
FXString cleanPath(const FXString);
FXString filePath(const FXString);
FXString filePath(const FXString, const FXString);

FXString fileFromURI(FXString);
FXString fileToURI(const FXString&);
FXString buildCopyName(const FXString&);

FXlong deltime(FXString);
int isEmptyDir(const FXString);
int hasSubDirs(const FXString);
FXbool exists(const FXString&);
FXbool isDirectory(const FXString&);
FXbool isFile(const FXString&);

FXbool isGroupMember(gid_t);
FXbool isWritable(const FXString&);
FXbool isReadable(const FXString&);
FXbool isReadExecutable(const FXString&);
FXbool isLink(const FXString&);
FXbool info(const FXString&, struct stat&);

FXString permissions(FXuint);
FXString readLink(const FXString&);
FXbool identical(const FXString&, const FXString&);

int setWaitCursor(FXApp*, FXuint);
int runst(FXString);
FXString getCommandOutput(FXString);
FXIcon* loadiconfile(FXApp*, const FXString, const FXString, FXColor blendcolor = -1, int arrow = 0);
FXIcon* loadxbmiconfile(FXApp*, const FXString, const FXString, FXColor blendcolor = -1);

FXString truncLine(FXString, FXuint);
FXString multiLines(FXString, FXuint);

#endif

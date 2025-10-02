#if 0
static const char sccsid[] = "@(#)ras.c	4.00 97/01/01 xlockmore";

#endif

/*-
 * Utilities for Sun rasterfile processing
 *
 * Copyright (c) 1995 by Tobias Gloth
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 *  5-Apr-06: Correction for TrueColor.
 *  3-Mar-96: Added random image selection.
 * 12-Dec-95: Modified to be a used in more than one mode
 *            <joukj AT hrem.nano.tudelft.nl>
 * 22-May-95: Written.
 */

#include "xlock.h"
#include "iostuff.h"
#include "ras.h"
#include <time.h>

XLockImage xlockimage;

static unsigned long get_long(int n);

static void
analyze_header(void)
{
	xlockimage.sign = get_long(0);
	xlockimage.width = get_long(1);
	xlockimage.height = get_long(2);
	xlockimage.depth = get_long(3);
	xlockimage.colors = get_long(7) / 3;
}

static unsigned long
get_long(int n)
{
	return
		(((unsigned long) xlockimage.header[4 * n + 0]) << 24) +
		(((unsigned long) xlockimage.header[4 * n + 1]) << 16) +
		(((unsigned long) xlockimage.header[4 * n + 2]) << 8) +
		(((unsigned long) xlockimage.header[4 * n + 3]) << 0);
}

int
RasterFileToImage(ModeInfo * mi, char *filename, XImage ** image ,
		  Colormap m_cmap )
{
	int         read_width , depth , format , i;
	FILE       *file;
   unsigned char* rasdata;
   unsigned char* tmpdata;
   unsigned long black, white, fgpix, bgpix;
   XColor      fgcol, bgcol;
   unsigned long* pixel_num;
   int x , y;

	if ((file = my_fopen(filename, "r")) == NULL) {
		/*(void) fprintf(stderr, "could not read file \"%s\"\n", filename); */
		return RasterOpenFailed;
	}
	(void) fread((void *) xlockimage.header, 8, 4, file);
	analyze_header();
	if (xlockimage.sign != 0x59a66a95) {
		/* not a raster file */
		(void) fclose(file);
		return RasterFileInvalid;
	}
	if (xlockimage.depth != 8) {
		(void) fclose(file);
		(void) fprintf(stderr, "only 8-bit Raster files are supported\n");
		return RasterColorFailed;
	}
	read_width = (int) xlockimage.width;
	if ((xlockimage.width & 1) != 0)
		read_width++;

   depth = MI_DEPTH(mi);
   rasdata = (unsigned char *) malloc((int) (read_width * xlockimage.height));
   if ( depth < 9 )
     xlockimage.data = (unsigned char *) malloc((int) ( read_width *
							xlockimage.height));
   else
     xlockimage.data = (unsigned char *) malloc((int) ( read_width *
							xlockimage.height *
							4 ));
	if (!xlockimage.data) {
		(void) fclose(file);
		(void) fprintf(stderr, "out of memory for Raster file\n");
		free( rasdata );
		return RasterNoMemory;
	}
   format=(depth == 1) ? XYBitmap : ZPixmap;
   if ( MI_IS_VERBOSE( mi ) )
	(void) printf ( "XCreateImage depth = %d\n", depth );
   if ( depth < 9 )
     *image = XCreateImage( MI_DISPLAY(mi), MI_VISUAL(mi), depth, format, 0,
			 (char *) xlockimage.data, (int) xlockimage.width,
			 (int) xlockimage.height, 16, (int) xlockimage.width );
   else
     *image = XCreateImage( MI_DISPLAY(mi), MI_VISUAL(mi), depth, format, 0,
			 (char *) xlockimage.data, (int) xlockimage.width,
			 (int) xlockimage.height, 16, (int) xlockimage.width*4 );
	if (!*image) {
		(void) fclose(file);
		(void) fprintf(stderr, "could not create image from Raster file\n");
		free( rasdata );
		return RasterColorError;
	}
	(void) fread((void *) xlockimage.color, (int) xlockimage.colors, 3, file);
	(void) fread((void *) rasdata, read_width, (int) xlockimage.height, file);
	(void) fclose(file);

   /*set up colourmap */
	black = MI_BLACK_PIXEL(mi);
	white = MI_WHITE_PIXEL(mi);
	fgpix = MI_FG_PIXEL(mi);
	bgpix = MI_BG_PIXEL(mi);
	fgcol.pixel = fgpix;
	bgcol.pixel = bgpix;
	XQueryColor(MI_DISPLAY(mi), MI_COLORMAP(mi), &fgcol);
	XQueryColor(MI_DISPLAY(mi), MI_COLORMAP(mi), &bgcol);

	/* Set these, if Image does not overwrite some, so much the better. */
	if (fgpix < COLORMAP_SIZE) {
		xlockimage.color[fgpix] = fgcol.red >> 8;
		xlockimage.color[fgpix+xlockimage.colors] = fgcol.green >> 8;
		xlockimage.color[fgpix+2*xlockimage.colors] = fgcol.blue >> 8;
	}
	if (bgpix < COLORMAP_SIZE) {
		xlockimage.color[bgpix] = bgcol.red >> 8;
		xlockimage.color[bgpix+xlockimage.colors] = bgcol.green >> 8;
		xlockimage.color[bgpix+2*xlockimage.colors] = bgcol.blue >> 8;
	}
	if (white < COLORMAP_SIZE) {
		xlockimage.color[white] = 0xFF;
		xlockimage.color[white+xlockimage.colors] = 0xFF;
		xlockimage.color[white+2*xlockimage.colors] = 0xFF;
	}
	if (black < COLORMAP_SIZE) {
		xlockimage.color[black] = 0;
		xlockimage.color[black+xlockimage.colors] = 0;
		xlockimage.color[black+2*xlockimage.colors] = 0;
	}
   /* supply data and colours*/
	pixel_num = (unsigned long *) malloc((int) (xlockimage.colors * sizeof( unsigned long )) );
	for ( i=0 ; i<xlockimage.colors ; i++ )
	  {
	     XColor Xcol;
	     Xcol.red = xlockimage.color[ i ]*257;
	     Xcol.green = xlockimage.color[ i + xlockimage.colors ]*257;
	     Xcol.blue = xlockimage.color[ i + 2 * xlockimage.colors ]*257;
	     XAllocColor( MI_DISPLAY(mi) , m_cmap , &Xcol);
	     pixel_num[i] = Xcol.pixel;
	  }

   tmpdata = rasdata;
   for ( y=0; y<xlockimage.height; y++ )
     {
	for ( x=0; x<xlockimage.width; x++)
	  {
	     XPutPixel( *image , x , y , pixel_num[ tmpdata[ 0 ] ] );
	     tmpdata++;
	  }
     }
   free( rasdata );
   free( pixel_num );

   return RasterSuccess;
}

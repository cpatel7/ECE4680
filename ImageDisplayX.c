#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main(int argc, char *argv[])
{
	Display			*Monitor;
	Window			ImageWindow;
	GC				ImageGC;
	XWindowAttributes		Atts;
	XImage			*Picture;
	int				ROWS,COLS, BYTES;
	int 			i,RGB;
	char			header[80];
	unsigned char		*displaydata;
	unsigned char 		*imagedata;
	FILE 				*fpt;

	/* ... */
	if(argc != 2)
	{
		printf("usage: ./prog [filename.ppm]");
		exit(0);
	}
	
	/* open image for reading */
	fpt = fopen(argv[1], "rb");
	if(fpt == NULL)
	{
		printf("ERROR! Unable to open %s", argv[1]);
		exit(0);
	}
	
	/* read image header */	
	i=fscanf(fpt,"%s %d %d %d ",header,&COLS,&ROWS,&BYTES);
	if (i != 4 || BYTES != 255)
	{
		if(strcmp(header,"P5") != 0 || strcmp(header,"P6") != 0)
		{
			printf("%s is not an 8-bit PPM greyscale (P5) OR a 24-bit PPM RGB (P6) image\n",argv[1]);
		}
		else
		{
			printf("%s is not a PPM image file\n",argv[1]);						
		}
		fclose(fpt);
		exit(0);
	}
	
	if(strcmp(header,"P5") == 0)
	{
		/* Greyscale Image */
		/* allocate dynamic memory for image */
		imagedata=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
		if (imagedata == NULL)
		{
			printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
			exit(0);
		}
	
		/* read image data from file */
		fread(imagedata,1,ROWS*COLS,fpt);
		fclose(fpt);

	
		/* allocate dynamic memory for display (16-bit) */
		displaydata=(unsigned char *)calloc(ROWS*COLS*2,sizeof(unsigned char));
		if (displaydata == NULL)
		{
			printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
			exit(0);
		}

		/* convert image to 16-bit Greyscale */

		for (i=0; i<ROWS*COLS; i++)
		{
			displaydata[i*2+1] = (imagedata[i] & 0xF8) + ((imagedata[i] & 0xE0) >> 5);
			displaydata[i*2+0] = ((imagedata[i] & 0x1C) << 3 ) + ((imagedata[i] & 0xF8) >> 3);

		}	
	}
	else
	{
		/* RGB Image */
		/* allocate dynamic memory for image */
		imagedata=(unsigned char *)calloc(ROWS*COLS*3,sizeof(unsigned char));
		if (imagedata == NULL)
		{
			printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
			exit(0);
		}
	
		/* read image data from file */
		fread(imagedata,2,ROWS*COLS*3,fpt);
		fclose(fpt);

	
		/* allocate dynamic memory for display (16-bit) */
		displaydata=(unsigned char *)calloc(ROWS*COLS*2,sizeof(unsigned char));
		if (displaydata == NULL)
		{
			printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
			exit(0);
		}

		/* convert image to 16-bit RGB */
		for (i=0; i<ROWS*COLS; i++)
		{
			displaydata[i*2+1] = (imagedata[i*3+0] & 0xF8) + ((imagedata[i*3+1] & 0xE0) >> 5);
			displaydata[i*2+0] = ((imagedata[i*3+1] & 0x1C) << 3) + ((imagedata[i*3+2] & 0xF8) >> 3);
		}
	} 

	

	Monitor=XOpenDisplay(NULL);
	if (Monitor == NULL)
	  {
	  printf("Unable to open graphics display\n");
	  exit(0);
	  }

	ImageWindow=XCreateSimpleWindow(Monitor,RootWindow(Monitor,0),
		50,10,		/* x,y on screen */
		COLS,ROWS,	/* width, height */
		2, 		/* border width */
		BlackPixel(Monitor,0),
		WhitePixel(Monitor,0));

	ImageGC=XCreateGC(Monitor,ImageWindow,0,NULL);

	XMapWindow(Monitor,ImageWindow);
	XFlush(Monitor);
	while(1)
	  {
	  XGetWindowAttributes(Monitor,ImageWindow,&Atts);
	  if (Atts.map_state == IsViewable /* 2 */)
	    break;
	  }

	Picture=XCreateImage(Monitor,DefaultVisual(Monitor,0),
			DefaultDepth(Monitor,0),
			ZPixmap,	/* format */
			0,		/* offset */
			displaydata,/* the data */
			COLS,ROWS,	/* size of the image data */
			16,		/* pixel quantum (8, 16 or 32) */
			0);		/* bytes per line (0 causes compute) */

	XPutImage(Monitor,ImageWindow,ImageGC,Picture,
			0,0,0,0,	/* src x,y and dest x,y offsets */
			COLS,ROWS);	/* size of the image data */

	XFlush(Monitor);
	sleep(7);
	XCloseDisplay(Monitor);
}

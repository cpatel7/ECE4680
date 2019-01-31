#include <windows.h>
#include <stdio.h>
#include <tchar.h>


LRESULT CALLBACK EventProcessor (HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{ return(DefWindowProc(hWnd,uMsg,wParam,lParam)); }

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,int nCmdShow)
{
	WNDCLASS		wc;
	HWND			WindowHandle;
	int				i,ROWS,COLS, BYTES, RGB;
	unsigned char	*displaydata, *imagedata, *flippedimage;
	unsigned char	header[80];
	BITMAPINFO		*bm_info;
	FILE			*fpt;
	HDC				hDC;

	fpt = fopen(lpCmdLine, "rb");
	if(fpt == NULL)
	{
		MessageBox(NULL, _T("Unable to open file"),_T(""), MB_OK | MB_APPLMODAL);
		exit(0);
	}

	fscanf(fpt, "%s %d %d %d ", header, &COLS, &ROWS, &BYTES); //read the last white space

	if (strcmp((const char*)header, "P5") == 0)
	{
		/* 8-bit greyscale */
		RGB = 0;
		displaydata = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
		if (displaydata == NULL)
		{
			MessageBox(NULL, _T("Unable to allocate required memory"),_T(""), MB_OK | MB_APPLMODAL);
			exit(0);
		}

		fread(displaydata, 1, ROWS*COLS, fpt);
	}
	else if (strcmp((const char*)header, "P6") == 0)
	{
		/* 24-bit RGB  */
		
		RGB = 1;
		imagedata = (unsigned char *)calloc(ROWS*COLS*3, sizeof(unsigned char));
		if (imagedata == NULL)
		{
			MessageBox(NULL, _T("Unable to allocate required memory"),_T(""), MB_OK | MB_APPLMODAL);
			exit(0);
		}

		fread(imagedata, 1, ROWS*COLS*3, fpt);
		flippedimage = (unsigned char *)calloc(ROWS*COLS*3, sizeof(unsigned char));
		//flip the image
		int c;
		for(i=ROWS; i>0; i--) //go from bottom of image to top
		{
			for(c=0; c<COLS; c++)
			{
				flippedimage[(i*COLS+c)*3] = imagedata[((ROWS-i)*COLS+c)*3];
				flippedimage[(i*COLS+c)*3+1] = imagedata[((ROWS-i)*COLS+c)*3+1];
				flippedimage[(i*COLS+c)*3+2] = imagedata[((ROWS-i)*COLS+c)*3+2];
			}
		}

		displaydata = (unsigned char *)calloc(ROWS*COLS*2, sizeof(unsigned char));
		if (displaydata == NULL)
		{
			MessageBox(NULL, _T("Unable to allocate required memory"),_T(""), MB_OK | MB_APPLMODAL);
			exit(0);
		}

		/* convert image from 24-bit to 16-bit */
		unsigned char red, green, blue;
		int j = 0;
		for (i=0; i<ROWS*COLS*3; i+=3)
		{
			
			red = flippedimage[i] & 0xF8; //get the first five
			green = flippedimage[i+1] & 0xF8; //get the first five
			blue = flippedimage[i+2] & 0xF8; //get the first five

			displaydata[j+1]	= (red >> 1) | (green >> 6);
			displaydata[j]		= (green << 2) | (blue >> 3);

			j+=2;
		}
	}
	else
	{
		MessageBox(NULL, _T("This is not a .ppm file"),_T(""), MB_OK | MB_APPLMODAL);
		fclose(fpt);
		exit(0);
	}

	fclose(fpt);

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=(WNDPROC)EventProcessor;
	wc.cbClsExtra=wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=NULL;
	wc.lpszMenuName=NULL;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName=_T("Image Window Class");
	if (RegisterClass(&wc) == 0)
	  exit(0);

	WindowHandle=CreateWindow(_T("Image Window Class"),_T("ECE468 Lab1"),
							  WS_OVERLAPPEDWINDOW,
							  10,10,COLS,ROWS,
							  NULL,NULL,hInstance,NULL);
	if (WindowHandle == NULL)
	{
		  MessageBox(NULL,_T("No window"),_T("Try again"),MB_OK | MB_APPLMODAL);
		  exit(0);
	}
	ShowWindow (WindowHandle, SW_SHOWNORMAL);

	bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
	hDC=GetDC(WindowHandle);

	/* ... set up bmiHeader field of bm_info ... */
	bm_info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bm_info->bmiHeader.biSizeImage = 0; 
	bm_info->bmiHeader.biBitCount = (RGB ? 16 : 8); 
	bm_info->bmiHeader.biWidth = COLS;
	bm_info->bmiHeader.biHeight = (RGB? ROWS : -ROWS); 
	bm_info->bmiHeader.biPlanes = 1;
	bm_info->bmiHeader.biCompression = BI_RGB;
	bm_info->bmiHeader.biClrUsed = 0;

	for (i=0; i<256; i++)	/* colormap */
	{
		  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
		  bm_info->bmiColors[i].rgbReserved=0;
	} 
	SetDIBitsToDevice(hDC,0,0,COLS,ROWS,0,0,
				  0, /* first scan line */
				  COLS, /* number of scan lines */
				  displaydata,bm_info,DIB_RGB_COLORS);
	
	free(bm_info);
	free(displaydata);	
	ReleaseDC(WindowHandle,hDC);
	Sleep(5000);
}
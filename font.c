#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define WIDTH 320
#define HEIGHT 240

unsigned int temp[WIDTH*HEIGHT];

void font_print( unsigned int *buffer, int xpos, int ypos, char *name, char* text, int size, float rot, int weight, BOOL italic, BOOL flipped, unsigned int color ){
	unsigned int *trasker=temp;
	int i;
	unsigned int alpha;

	HDC hdc_mem;
	HBITMAP bitmap;
	BITMAPINFO bi;
	LOGFONT lf;
	HFONT font;
	HDC hdc;

	hdc = GetDC( NULL );
	bitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
	hdc_mem = CreateCompatibleDC(hdc);
	SelectObject(hdc_mem, bitmap);

	lf.lfHeight = size;
	lf.lfWidth= 0;

	if( flipped==TRUE)
		lf.lfEscapement     = (long)((rot-3.1415f)*10*(180/3.1415f));
	else
		lf.lfEscapement     = (long)(rot*10*(180/3.1415f));

	lf.lfOrientation    = 0;
	lf.lfWeight         = weight;
	lf.lfItalic         = italic;
	lf.lfUnderline      = 0;
	lf.lfStrikeOut      = 0;
	lf.lfCharSet        = 0;
	lf.lfOutPrecision   = 0;
	lf.lfClipPrecision  = 0;
	lf.lfQuality        = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = 0;
	strcpy( lf.lfFaceName, name );

	font = CreateFontIndirect( &lf );

	memset( &bi, 0, sizeof(BITMAPINFO) );
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = WIDTH;

	if( flipped==TRUE)
		bi.bmiHeader.biHeight = HEIGHT;
	else
		bi.bmiHeader.biHeight = -HEIGHT;

	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	SetBkMode( hdc_mem, TRANSPARENT );
	memset(temp,0,WIDTH*HEIGHT*4);
	SetDIBits( hdc_mem, bitmap, 0, HEIGHT, temp, &bi, DIB_RGB_COLORS );

	SelectObject(hdc_mem, font);

	alpha = (color>>24);
	SetTextColor( hdc_mem, alpha|(alpha<<8)|(alpha<<16));

	TextOut(hdc_mem, xpos, ypos, text, strlen(text));
	GetDIBits( hdc_mem, bitmap, 0, HEIGHT, temp, &bi, DIB_RGB_COLORS );

	DeleteDC(hdc_mem);
	DeleteObject(bitmap);
	DeleteObject(font);

	for(i=WIDTH*HEIGHT;i;i--){
		unsigned int old_alpha = ((*buffer&0xFF000000)>>24);
		unsigned int new_color;
		unsigned int old_color = *buffer;

		alpha = ((*trasker&0xFF0000)>>16)+((*trasker&0xFF00)>>8)+(*trasker&0xFF);
		alpha /= 3;
		if(alpha>255) alpha = 255;

		new_color  = ( ((((old_color)&0xFF00FF)*(255-alpha))+(color&0xFF00FF)*alpha)>>8 )&0xFF00FF;
		new_color |= ( ((((old_color)&0x00FF00)*(255-alpha))+(color&0x00FF00)*alpha)>>8 )&0x00FF00;

		alpha += old_alpha;
		if(alpha>255) alpha = 255;

		*buffer = new_color|(alpha<<24);

		trasker++;
		buffer++;
	}

}

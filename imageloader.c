#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <ocidl.h>
#include <olectl.h>
#include <crtdbg.h>
#define HIMETRIC_INCH	2540

void load_bitmap( void *data, int data_len, unsigned int **pixels, unsigned int *w, unsigned int *h ){
	BITMAPINFO bi;
	HGLOBAL global;
	void* temp_data;
	LPSTREAM stream;
	HRESULT hr;
	LPPICTURE picture;
	long width, height;
	int width_, height_;

	HBITMAP bitmap;
	HDC hdc, hdc_mem;
	unsigned int* image_data;

	if(w)*w = 0;
	if(h)*h = 0;
	*pixels = NULL;

	if(!data) return;

	global = GlobalAlloc( GMEM_MOVEABLE, data_len );
	temp_data = GlobalLock( global );
	memcpy( temp_data, data, data_len );
	GlobalUnlock( global );
	stream = NULL;
	hr = CreateStreamOnHGlobal( global, TRUE, &stream );
	if(FAILED(hr)) return;

	hr = OleLoadPicture(
		stream,
		data_len,
		FALSE,
		&IID_IPicture,
		(LPVOID *)&picture
	);
	if(FAILED(hr)) return;

	stream->lpVtbl->Release(stream);

	picture->lpVtbl->get_Width( picture, &width );
	picture->lpVtbl->get_Height( picture, &height );

	hdc = GetDC( NULL );
	width_ = MulDiv(width, GetDeviceCaps(hdc, LOGPIXELSX), HIMETRIC_INCH);
	height_ = MulDiv(height, GetDeviceCaps(hdc, LOGPIXELSY), HIMETRIC_INCH);

	bitmap = CreateCompatibleBitmap(hdc, width_, height_ );
	hdc_mem = CreateCompatibleDC(hdc);
	SelectObject(hdc_mem, bitmap);

	picture->lpVtbl->Render( picture, hdc_mem, 0, 0, width_, height_, 0, height, width, -height, NULL );
	picture->lpVtbl->Release(picture);
	image_data = malloc(sizeof(unsigned int)*width_*height_);

	memset( &bi, 0, sizeof(BITMAPINFO) );
	bi.bmiHeader.biSize=sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width_;
	bi.bmiHeader.biHeight = -height_;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	GetDIBits( hdc_mem, bitmap, 0, height_, image_data, &bi, DIB_RGB_COLORS );
	DeleteDC(hdc_mem);
	DeleteObject(bitmap);

	if(w) *w = width_;
	if(h) *h = height_;
	*pixels = image_data;
	return;
}
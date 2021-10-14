#ifndef __IMAGELOADER_H__
#define __IMAGELOADER_H__

#ifdef __cplusplus
extern "C"{
#endif

void load_bitmap( void *data, int data_len, unsigned int **pixels, unsigned int *w, unsigned int *h );

#ifdef __cplusplus
}
#endif

#endif /*__IMAGELOADER_H__*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

#include "leepra.h"
#include "imageloader.h"
#include "font.h"
#include "file.h"

#include "minifmod.h"

void error(char* string){
	leepra_close();
	MessageBox( NULL, string, NULL, MB_OK );
	exit(1);
}

#define WIDTH 320
#define HEIGHT 240

unsigned int screen[WIDTH*HEIGHT];
unsigned int noicebuffer[WIDTH*HEIGHT];
unsigned int *pics[9];
unsigned int text[25][WIDTH*HEIGHT];

extern unsigned char img01[];
extern unsigned char img02[];
extern unsigned char img03[];
extern unsigned char img04[];
extern unsigned char img05[];
extern unsigned char img06[];
extern unsigned char img07[];
extern unsigned char img08[];
extern unsigned char img09[];

#include "samplegen.h"
void sampleloadcallback(void *buff, int lenbytes, int numbits, int instno, int sampno){
	int len;
	void *sampledata = gen_sample(instno,&len);
	if(lenbytes>(len*2)) lenbytes = len*2;

	memset(buff,0,lenbytes);
	memcpy(buff,sampledata,lenbytes);

	free(sampledata);
}

void multiply( unsigned int *bitmap1, unsigned int *bitmap2 ){
	__asm{
		mov edi, bitmap1
		mov esi, bitmap2
		mov ecx, WIDTH*HEIGHT
		pxor mm5, mm5

align 16
lup:
		movd mm0, [edi]
		movd mm1, [esi]
		punpcklbw mm0, mm5
		punpcklbw mm1, mm5

		pmullw mm0, mm1
		add esi, 4
		psrlw mm0, 8
		packuswb mm0, mm0
		movd eax, mm0

		mov [edi], eax
		add edi, 4

		dec ecx
		jnz lup

		emms
	}
}

void blend( unsigned int *bitmap1, unsigned int *bitmap2, unsigned char alpha ){
	unsigned short blend[4]={alpha,alpha,alpha,alpha};

	__asm{
		movq mm6, [blend]
		pxor mm5, mm5

		mov edi, bitmap1
		mov esi, bitmap2
		mov ecx, WIDTH*HEIGHT
		pxor mm5, mm5

align 16
lup:
		movd mm0, [edi]
		movd mm1, [esi]
		punpcklbw mm0, mm5
		punpcklbw mm1, mm5

		psubw mm1, mm0
		add esi, 4
		pmullw mm1, mm6
		psllw mm0, 8
		paddw mm0, mm1
		psrlw mm0, 8
		packuswb mm0, mm5

		movd [edi], mm0
		add edi, 4

		dec ecx
		jnz lup

		emms
	}
}

void colorfade( unsigned int *bitmap, unsigned int color, unsigned char alpha ){
	int len = WIDTH*HEIGHT;

	static short white[]= {0xff,0xff,0xff,0xff};
	__asm{

		xor eax, eax
		mov al, alpha
		mov ebx, eax
		shl ebx, 8
		add eax, ebx
		shl ebx, 8
		add eax, ebx
		shl ebx, 8
		add eax, ebx

		pxor mm5, mm5

		movd mm2, eax
		punpcklbw mm2, mm5

		movq mm3, white
		psubusw mm3, mm2
		
		mov edi, bitmap

		movd mm1, color
		punpcklbw mm1, mm5

		pmullw mm1, mm2
		psrlw mm1, 8

		mov ecx, len
align 16
lup:
		movd mm0, [edi]
		punpcklbw mm0, mm5
		pmullw mm0, mm3
		add esi, 4
		psrlw mm0, 8
		paddusw mm0, mm1
		packuswb mm0, mm0
		movd eax, mm0
		stosd

		dec ecx
		jnz lup

		emms
	}
}

void noice( unsigned int *bitmap, unsigned int random){
	int i;
	for(i=((WIDTH*HEIGHT)>>3);i;i--){
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
		*bitmap++ = (*bitmap + random)^random;
	}
}

void gamma( unsigned int *bitmap, float gamma ){
	int counter;
	static unsigned char gammatable[256];
	unsigned char *src = (unsigned char*)bitmap;

	for( counter=0; counter<256; counter++ ){
		int val = (int)pow(counter,gamma);
		if(val<0) val=0;
		if(val>255) val=255;
		gammatable[counter] = val;
	}

	for( counter=WIDTH*HEIGHT; counter; counter-- ){
		*src++ = gammatable[*src];
		*src++ = gammatable[*src];
		*src++ = gammatable[*src];
		src++;
	}
}

void main(){
	MSG msg;
	int w,h;
	float time = 0;
	FMUSIC_MODULE *mod;

	load_bitmap( img01,10000, &pics[0], &w, &h);
	load_bitmap( img02,10000, &pics[1], &w, &h);
	load_bitmap( img03,10000, &pics[2], &w, &h);
	load_bitmap( img04,10000, &pics[3], &w, &h);
	load_bitmap( img05,10000, &pics[4], &w, &h);
	load_bitmap( img06,10000, &pics[5], &w, &h);
	load_bitmap( img07,10000, &pics[6], &w, &h);
	load_bitmap( img08,10000, &pics[7], &w, &h);
	load_bitmap( img09,10000, &pics[8], &w, &h);

	memset( text[0], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[0], -10,0, "arial black", "arcane", 130, 0, 0, 0, 0, 0xFF000000 );

	memset( text[1], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[1], -10,50, "arial black", "won't", 160, 0, 0, 0, 0, 0xFF000000 );

	memset( text[2], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[2], -10,25, "arial black", "sleep", 165, 0, 0, 0, 0, 0xFF000000 );

	memset( text[3], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[3], -5,-5, "arial black", "with", 200, 0, 0, 0, 0, 0xFF000000 );

	memset( text[4], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[4], -20,-50, "arial black", "me", 300, 0, 0, 0, 0, 0xFF000000 );


	/* start-dilldall */

	memset( text[5], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[5], 0,0, "arial", "O", 100, 0, 0, 0, 0, 0xFF000000 );

	memcpy(text[6],text[5], WIDTH*HEIGHT*4);
	font_print( text[6], 20,-30, "times", "d", 170, 0.25f, 0, 0, 0, 0xFF000000 );

	memcpy(text[7],text[6], WIDTH*HEIGHT*4);
	font_print( text[7], 180,-30, "arial black", "d", 120, -0.15f, 0, 0, 0, 0xFF000000 );

	memcpy(text[8],text[7], WIDTH*HEIGHT*4);
	font_print( text[8], 5,120, "arial", "p    r    e    s    e    n    t    s", 14, 0, 0, 0, 0, 0xFF000000 );

	memset( text[9], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[9], 200,100, "times", "ARCANE", 32, -0.3f, 800, 0, 0, 0xFF000000 );

	memcpy(text[10],text[9], WIDTH*HEIGHT*4);
	font_print( text[10], 70,132, "arial black", "W   O   N   '   T", 30, 0, 0, 0, 0, 0xFF000000 );

	memcpy(text[11],text[10], WIDTH*HEIGHT*4);
	font_print( text[11], 70,132, "times", "sleep", 70, -0.3f, 0, TRUE, 0, 0xFF000000 );

	memcpy(text[12],text[11], WIDTH*HEIGHT*4);
	font_print( text[12], 165,185, "arial", "w i t h", 30, 0.3f, 0, FALSE, 0, 0xFF000000 );

	memcpy(text[13],text[12], WIDTH*HEIGHT*4);
	font_print( text[13], 210,185, "times", "Me", 70, 0, 800, FALSE, 0, 0xFF000000 );

	memcpy(text[14],text[13], WIDTH*HEIGHT*4);
	font_print( text[14], 295,175, "arial black", "!", 80, 0, 0, FALSE, 0, 0xFF000000 );


	memset( text[15], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[15], 30,40, "arial black", "LETS", 130, 0, 0, 0, 0, 0xFF000000 );

	memset( text[16], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[16], 55,40, "arial black", "EAT", 130, 0, 0, 0, 0, 0xFF000000 );

	memset( text[17], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[17], 10,40, "arial black", "SOME", 130, 0, 0, 0, 0, 0xFF000000 );

	memset( text[18], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[18], 2,65, "arial black", "WICKED", 100, 0, 0, 0, 0, 0xFF000000 );

	memset( text[19], 0xFFFFFFFF, WIDTH*HEIGHT*4);
	font_print( text[19], 5,65, "arial black", "DRUGS!", 100, 0, 0, 0, 0, 0xFF000000 );

	if(!leepra_open( "odd - arcane won't sleep with me!", WIDTH, HEIGHT, TRUE)) error( "pimp yo mama!" );

	FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell);

	if(!FSOUND_Init(44100,0)) error("SUCKY SOUNDCARD!!!");
	mod = FMUSIC_LoadSong(NULL, sampleloadcallback);
	if(!mod) error("YOU ARE NOT ALLOWED!");

	FMUSIC_PlaySong(mod);
	memset(screen,0,WIDTH*HEIGHT*sizeof(unsigned int));

	do{
		int ord = FMUSIC_GetOrder(mod);
		int row = FMUSIC_GetRow(mod);
		time = ((float)row)*(1.f/128.f);

//		ord = 18;

//		printf("time: %f row: %i\n",time,row);

		if(ord<3){
			memcpy(screen,pics[1],WIDTH*HEIGHT*sizeof(unsigned int));
			if(ord==0) colorfade(screen, 0, 255-row*2);
			if(ord==2){
				if(row<32){
					multiply(screen, text[5]);
				}else if(row<(32+12)){
					multiply(screen, text[6]);
				}else if(row<64+16){
					multiply(screen, text[7]);
				}else{
					multiply(screen, text[8]);
				}
			}
			gamma(screen, 1+(float)sin(-(M_PI/2)+time*M_PI*16)*0.05f);
		}else if(ord==3){
			memcpy(screen,pics[0],WIDTH*HEIGHT*sizeof(unsigned int));
			gamma(screen, 1+(float)sin(-(M_PI/2)+time*M_PI*16)*0.05f);
			if(row<32){
				multiply(screen, text[9]);
			}else if(row<(32+12)){
				multiply(screen, text[10]);
			}else if(row<32+24){
				multiply(screen, text[11]);
			}else if(row<64){
				multiply(screen, text[12]);
			}else if(row<64+16){
				multiply(screen, text[13]);
			}else{
				multiply(screen, text[14]);
			}
		}else if(ord<10){
			blend(screen,pics[((int)(time*5))%10], (int)(10+(1+sin(time))*3) );
			multiply(screen, text[(int)(time*150)%5]);
			if(ord>5) gamma(screen, 1+(float)sin(time*M_PI*32)*0.05f);
		}else if(ord<14){
			if(ord==10) memcpy(screen,pics[2],WIDTH*HEIGHT*sizeof(unsigned int));
			else if(ord==11) memcpy(screen,pics[3],WIDTH*HEIGHT*sizeof(unsigned int));
			else if(ord==12){
				if(row<64) memcpy(screen,pics[4],WIDTH*HEIGHT*sizeof(unsigned int));
				else memcpy(screen,pics[5],WIDTH*HEIGHT*sizeof(unsigned int));
			}
			else if(ord==13){
				memcpy(screen,pics[6],WIDTH*HEIGHT*sizeof(unsigned int));
				if(row>31 && row<64) multiply(screen, text[15]);
				else if(row>63 && row<80) multiply(screen, text[16]);
				else if(row>79 && row<96) multiply(screen, text[17]);
				else if(row>95 && row<104) multiply(screen, text[18]);
				else if(row>103 && row<112) multiply(screen, text[19]);
				else if(row>111) multiply(screen, text[15+(row%5)]);
			}
			gamma(screen, 1+(float)sin(time*M_PI*32)*0.05f);
		}else if(ord<18){
			blend(screen,pics[((int)(time*5))%10], (int)(10+(1+sin(time))*3) );
			gamma(screen, 1+(float)sin(time*M_PI*32)*0.05f);
			noice(screen,rand());
			multiply(screen, text[(int)(time*100)%5]);
		}else if(ord<23){
			if(ord==18){
				blend(screen,pics[((int)(time*5))%10], (int)(10+(1+sin(time))*3) );
				gamma(screen, 1+(float)sin(time*M_PI*32)*0.05f);
				noice(screen,rand());
				multiply(screen, text[(int)(time*100)%5]);

				blend(screen,pics[7], row*8 );
			}else if(ord<21){
				if(ord==20){
					memcpy(screen,pics[1],WIDTH*HEIGHT*sizeof(unsigned int));
				}
			}else{
				blend(screen,pics[((int)(time*5))%10], (int)(10+(1+sin(time))*3) );
				gamma(screen, 1+(float)sin(time*M_PI*32)*0.05f);
				noice(screen,rand());
				multiply(screen, text[(int)(time*100)%5]);
			}
		}else if(ord<33){
			if(ord==26){
					memcpy(screen,pics[4],WIDTH*HEIGHT*sizeof(unsigned int));
			}else if(ord==28){
					memcpy(screen,pics[5],WIDTH*HEIGHT*sizeof(unsigned int));
			}else{
				blend(screen,pics[((int)(time*5))%10], (int)(10+(1+sin(time))*3) );
				noice(screen,rand());
				gamma(screen, 1+(float)sin(time*M_PI*64)*0.05f);
				noice(screen,rand());
				gamma(screen, 1+(float)sin(time*M_PI*32)*0.05f);
				noice(screen,rand());
				gamma(screen, 1+(float)sin(time*M_PI*8)*0.05f);
				multiply(screen, text[(int)(time*100)%5]);
			}
		}else{
			memcpy(screen,pics[8],WIDTH*HEIGHT*sizeof(unsigned int));
			if(row<32) colorfade(screen, 0, 255-(row*8));
			else colorfade(screen, 0, (row-32)*4);
		}
				
		
		if(ord==33 && row>(31+63)){
			PostQuitMessage(0);
		}

		leepra_update(screen);
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){ 
		    TranslateMessage(&msg);
		    DispatchMessage(&msg); 
		    if (msg.message == WM_QUIT)
				break;
		}
	}while((msg.message!=WM_QUIT) && !GetAsyncKeyState(VK_ESCAPE));

	FMUSIC_FreeSong(mod);
	FSOUND_Close();
	leepra_close();
}
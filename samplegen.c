#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>

#pragma intrinsic( cos, sin )

#define SAMPLERATE 44100
#define LEN 65536

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795028876f
#endif

float f_rand(){
	static unsigned long rand_seed = 22222;
	rand_seed = (rand_seed * 196314165) + 907633515;
	return ((float)rand_seed/2147483648.f)-1;
}

void gen_sin(float* buffer, int len){
	int i;
	float rate = 2*M_PI*(440.f/(float)SAMPLERATE);
	for(i=0;i<len;i++) buffer[i] += (float)sin(i*rate);
}

void gen_saw(float* buffer, int len){
	int i;
	float rate = 440.f/(float)SAMPLERATE*2;
	for(i=0;i<len;i++) buffer[i] += (float)fmod(i*rate,2)-1;
}


void gen_square(float* buffer, int len){
	int i;
	const float wavelength = 440.f/(float)SAMPLERATE*2;
	for(i=0;i<len;i++) buffer[i] += ((int)(i*wavelength) & 1);
}

void gen_funk(float* buffer, int len){
	int i;
	float rate = 2*M_PI*(440.f/(float)SAMPLERATE);
	for(i=0;i<len;i++) buffer[i] += (float)sin(i*rate*sin(i*0.001f));
}

void gen_bassdrum(float* buffer, int len, int base){
	int i;
	for(i=0;i<len;i++){
		float hz = (float)pow(base, 1-(float)(i*4)/SAMPLERATE);
		float amp = 1-(float)(i-len);
		if(amp>1) amp = 1;
		buffer[i] += (float)sin(i*hz*(M_PI/44100))*amp;
	}
}

void gen_evil_bassdrum(float* buffer, int len){
	int i;
	for(i=0;i<len;i++){
		float temp;
		float hz = (float)pow(1000, 1-(float)(i*2)/SAMPLERATE);
		float amp = 1-(float)(i-len);
		temp = (float)sin(i*hz*(M_PI/44100))*amp;
		if(temp>1.f) temp = 1.f;
		if(temp<-1.f) temp = -1.f;
		buffer[i] += temp;
	}
}

void gen_disco(float* buffer, int len){
	int i;
	for(i=len; i; i--){
		float hz = 1-(float)log((i+0.001f)*0.00001f)*1000;
		float amp = 1-(float)(i-35280)*0.2f;
		if(amp>1) amp = 1;

		buffer[i] += ((float)sin(i*hz*(M_PI/44100))*amp);
	}
}

void gen_noise(float* buffer, int len){
	int i;
	float vol = 1.f;
	for( i=0; i<len; i++ ){
		float rnd = f_rand();
		vol = (float)tan(1-(float)i/len);
		buffer[i] += (rnd*vol);
	}
}

void gen_synth(float* buffer, int len){
	int i;
	float rate = 2*M_PI*(440.f/(float)SAMPLERATE);
	for(i=0;i<len;i++)
		buffer[i] += 
				(float)sin(i*rate+sin(i*rate*0.1f)+sin(i*rate*0.4f)+sin(i*rate*0.3f));
}

void gen_shjit(float* buffer, int len,  int delay_len){
	float *delay = malloc(sizeof(float)*delay_len);
	float old = 0;
	int i;
	float rate = 2*M_PI*(440.f/(float)SAMPLERATE);

	for(i=0;i<delay_len;i++) delay[i] = (float)sin(i*0.01f)*f_rand();

	for(i=0;i<len;i++){
		float feedback = delay[(i+delay_len)%delay_len];
		float mixed = (feedback+old)*0.5f;
		old = feedback;
		buffer[i] += mixed;
		delay[(i+delay_len)%delay_len] = mixed;
	}
	free(delay);
}

/* filtere */
void flt_lp(float *buffer, int len, float f, float r){
	int i;
	float c = 1.f / (float)tan(M_PI*f/(float)SAMPLERATE);
	float a1 = 1.f / (1.f+r*c+c*c);
	float a2 = 2*a1;
	float a3 = a1;
	float b1 = 2.f*(1.f-c*c)*a1;
	float b2 = (1.f-r*c+c*c)*a1;
	float out_n1 = 0;
	float out_n2 = 0;
	float in_n1 = 0;
	float in_n2 = 0;

	for( i=0; i<len; i++ ){
		float in = buffer[i];
		float out = a1 * in + a2 * in_n1 + a3 * in_n2 - b1*out_n1 - b2*out_n2;
		in_n2 = in_n1;
		in_n1 = in;
		out_n2 = out_n1;
		out_n1 = out;
		buffer[i] = out;
	}
}

void flt_hp(float *buffer, int len, float f, float r){
	int i;

	float c = (float)tan(M_PI*f/(float)SAMPLERATE);
	float a1 = 1.f/(1.f+r*c+c*c);
	float a2 = -2*a1;
	float a3 = a1;
	float b1 = 2.f*(c*c-1.f)*a1;
	float b2 = (1.f-r*c+c*c)*a1;

	float out_n1 = 0;
	float out_n2 = 0;
	float in_n1 = 0;
	float in_n2 = 0;

	for( i=0; i<len; i++ ){
		float in = buffer[i];
		float out = a1 * in + a2 * in_n1 + a3 * in_n2 - b1*out_n1 - b2*out_n2;
		in_n2 = in_n1;
		in_n1 = in;
		out_n2 = out_n1;
		out_n1 = out;

		buffer[i] = out;

	}
}

void flt_feedback(float* buffer, int len,  int delay_len, float feedback_level){
	float *delay = malloc(sizeof(float)*delay_len);
	int i;
	float rate = 2*M_PI*(440.f/(float)SAMPLERATE);

	for(i=0;i<delay_len;i++) delay[i] = 0;

	for(i=0;i<len;i++){
		float feedback = delay[(i+delay_len)%delay_len];
		float mixed = buffer[i]+feedback*feedback_level;
		buffer[i] = mixed;
		delay[(i+delay_len)%delay_len] = mixed;
	}
	free(delay);
}

void flt_distort(float *buffer, int len, float a){
	int i;
	for(i=0;i<len;i++){
		float x = buffer[i];
		buffer[i] = (float)(x*(abs((int)x) + a)/(pow(x,2) + (a-1)*abs((int)x) + 1));
	}
}

int lens[] = {
	LEN*2,
	LEN,
	LEN,
	LEN,
	LEN,
	LEN,
	LEN*2,
	LEN/4,
	LEN,
	LEN,
	LEN,
	LEN*4,
	LEN,
	LEN
};

#define MAX ((float)sqrt(2))
short *gen_sample(const int sample_num, int *len){
	int i;

	static float fsample[44100*10];
	short *sample;
	*len = lens[sample_num%(sizeof(lens)/sizeof(int))];

	sample = (short*)malloc(sizeof(short)*(*len));
	memset(sample,0,sizeof(short)*(*len));
	memset(fsample,0,sizeof(float)*(*len));

	switch(sample_num){
	case 0:
		gen_synth(fsample,*len);
		gen_sin(fsample,*len);
		flt_hp(fsample,*len,800,MAX*0.8f);
		flt_feedback(fsample,*len,1000,0.3f);
	break;
	case 1:
		gen_saw(fsample,*len);
		flt_feedback(fsample,*len,1000,0.9f);
		flt_lp(fsample,*len,400,MAX*0.2f);
		gen_sin(fsample,*len);
	break;
	case 2:
		gen_square(fsample,*len);
		flt_feedback(fsample,*len,1000,0.3f);
	break;
	case 3:
		gen_funk(fsample,*len);
		flt_lp(fsample,*len,400,MAX*0.2f);
		flt_hp(fsample,*len,400,MAX*0.3f);
	break;
	case 4:
		gen_disco(fsample,*len);
		flt_lp(fsample,*len,400,MAX*0.2f);
		flt_hp(fsample,*len,400,MAX*0.3f);
	break;
	case 5:
		gen_bassdrum(fsample,*len,1000);
		flt_lp(fsample,*len,40,MAX*0.2f);
		flt_hp(fsample,*len,40,MAX*0.3f);
		gen_bassdrum(fsample,*len,1000);
	break;
	case 6:
		gen_bassdrum(fsample,*len,1000);
		flt_feedback(fsample,*len,1000,0.98f);
	break;
	case 7:
		gen_evil_bassdrum(fsample,*len);
		flt_feedback(fsample,*len,1000,0.9f);
	break;
	case 8:
		gen_noise(fsample,*len);
	break;
	case 9:
		gen_noise(fsample,*len);
		flt_lp(fsample,*len,400,MAX*0.2f);
		flt_hp(fsample,*len,400,MAX*0.3f);
	break;
	case 10:
		gen_evil_bassdrum(fsample,*len);
		flt_lp(fsample,*len,400,MAX*0.2f);
		flt_hp(fsample,*len,400,MAX*0.3f);
	break;
	case 11:
		gen_shjit(fsample,*len,1000);
		flt_feedback(fsample,*len,100,0.3f);
	break;
	case 12:
		gen_shjit(fsample,*len,60);
	break;
	case 13:
		gen_bassdrum(fsample,*len,2000);
		gen_bassdrum(fsample,*len,2000);
		flt_distort(fsample,*len,3);
	break;
	default:
		gen_sin(fsample,*len);
	}

	for(i=0;i<*len;i++){
		float temp = fsample[i];
		if(temp>1.f) temp = 1.f;
		if(temp<-1.f) temp = -1.f;
		sample[i] = (short)(temp*32767.f);
	}

//	free(fsample);
	return sample;
}

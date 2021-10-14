#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "file.h"

extern unsigned char music[];
extern unsigned char music_end[];

unsigned int memopen(char *name){
	MEMFILE *memfile = malloc(sizeof(MEMFILE));
	memfile->length = music_end-music;
	memfile->data = (void*)music;
	memfile->pos = 0;

	return (unsigned int)memfile;
}

void memclose(unsigned int handle){
	MEMFILE *memfile = (MEMFILE *)handle;
	free(memfile);
}

int memread(void *buffer, int size, unsigned int handle){
	MEMFILE *memfile = (MEMFILE *)handle;
	if (memfile->pos + size >= memfile->length) size = memfile->length - memfile->pos;
	memcpy(buffer, (char *)memfile->data+memfile->pos, size);
	memfile->pos += size;
	return size;
}

void memseek(unsigned int handle, int pos, signed char mode){
	MEMFILE *memfile = (MEMFILE *)handle;
	if (mode == SEEK_SET) memfile->pos = pos;
	else if (mode == SEEK_CUR) memfile->pos += pos;
	else if (mode == SEEK_END) memfile->pos = memfile->length + pos;
	if (memfile->pos > memfile->length) memfile->pos = memfile->length;
}

int memtell(unsigned int handle){
	MEMFILE *memfile = (MEMFILE *)handle;
	return memfile->pos;
}

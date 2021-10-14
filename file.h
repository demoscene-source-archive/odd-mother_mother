typedef struct {
	int length;
	int pos;
	void *data;
}MEMFILE;


unsigned int memopen(char *name);
void memclose(unsigned int handle);
int memread(void *buffer, int size, unsigned int handle);
void memseek(unsigned int handle, int pos, signed char mode);
int memtell(unsigned int handle);

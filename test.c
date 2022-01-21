#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

void
readfp (char* filename)
{
	FILE * pFile;
  	long lSize;
  	char * buffer;
	unsigned int * buf;
  	size_t result;

  	pFile = fopen(filename , "rb");
  	if(pFile==NULL){
		fputs("File error",stderr); exit (1);
	}

  	buf = (int*)malloc(sizeof(int));
  	if(buf == NULL){
		fputs("Memory error",stderr); exit (2);
	}

	//pathlen
  	result = fread(buf,4,1,pFile);
  	if(result != 1){
		fputs("Reading error",stderr); exit (3);
	}
	printf("%08x\n", *buf);

  	// pathname
  	buffer = (char*)malloc(sizeof(char)*((*buf)+1));
  	if(buffer == NULL){
		fputs("Memory error",stderr); exit (2);
	}
  	result = fread(buffer,1,(*buf),pFile);
  	if(result != *buf){
		fputs("Reading error",stderr); exit (3);
	}
	buffer[(*buf)] = '\0';
	printf("%s\n", buffer);

  	//dsize 
  	result = fread(buf,4,1,pFile);
  	if(result != 1){
		fputs("Reading error",stderr); exit (3);
	}
	printf("%08x\n", *buf);

  	//mode
  	result = fread (buf,4,1,pFile);
  	if(result != 1){
		fputs ("Reading error",stderr); exit (3);
	}
	printf("%08x\n", *buf);

  	// terminate
  	fclose(pFile);
  	free(buffer);
	free(buf);
  	return ;
}

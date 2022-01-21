#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int
main(int argc, char* argv[]){
	FILE* fp = fopen(argv[1],"rb");
	char* buffer;
	long lsize;
	size_t buf;

	fseek(fp,0,SEEK_END);
	lsize = ftell(fp);
	rewind(fp);

	buffer = (char*)malloc(sizeof(char)*lsize);

	buf = fread(buffer,1,lsize, fp);
	if(buf != lsize) printf("err");

	for(long i=0; i<lsize; i+=2){
		if(i%16==0) printf("%08lx: ",i);
		printf("%02x%02x ", buffer[i], buffer[i+1]);
		//printf("%ld\n",i);
		if(i%16==14){
			//for(int j=i-16; j<16;i++){
			//	printf("%c",buffer[j]);
			//}
			printf("\n");
		}
	}
	printf("\n");
	
	fclose(fp);
	free(buffer);
}

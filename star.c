#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
	archive,
	list,
	extract,
	N_op
} op ;

char * op_str[N_op] = {
	"archive",
	"list",
	"extract",
} ;

op
get_op_code (char * s)
{
	for(int i=0; i<N_op; i++){
		if(strcmp(op_str[i],s)==0){
			return i;
		}
	}
}

int
endstr (char* dirname)
{
	int i=0;
	while(dirname[i]!='\0')
		i++;
	if(dirname[i-1] == '/')
		return 1;
	return 0;
}

char*
finddir (char* dirname)
{
	char* cp;
	char* cur = dirname;

	cp = strchr(dirname, '/');
	while(cp != NULL || cp+1 != NULL){
		cp++;
		cur = cp;
		cp = strchr(cp,'/');
	}
	return cur;
}

int
writefile(FILE* fp, char* dirname)
{
	//printf("mkfile-%s\n",dirname);
	
  	struct stat sfile;
	char* buffer;
	size_t res;

  	if(stat(dirname,&sfile)==-1){
    		printf("Error Occurred\n"); return 1;
  	}
	
	FILE * origin = fopen(dirname, "rb");
	if(origin == NULL){
		printf("no such file to read\n"); return 1;
	}

	//header
	unsigned int pathlen[1];// = {findpathlen(dirname)};
	pathlen[0] = strlen(dirname);
	char* path = dirname;
	unsigned int* dsize = (unsigned int*)malloc(sizeof(unsigned int));
  	if(dsize == NULL){
		fputs("Memory error(malloc)",stderr); fclose(origin); return 1;
	}
	dsize[0] = sfile.st_size;
	/*
	printf("pathlen: %08x\n", *pathlen);
	printf("path: ");
	for(int i=0; i<*pathlen; i++){
		printf("%02x", path[i]);
	}
	printf("\n");
	printf("size: %08x\n", *dsize);
	*/

	fwrite(pathlen, 4, 1, fp);
	fwrite(path, sizeof(char), pathlen[0], fp);
	fwrite(dsize, 4, 1, fp);
	free(dsize);

	int* mode = (int*)malloc(sizeof(int));
  	if(dsize == NULL){
		fputs("Memory error(malloc)",stderr); fclose(origin); return 1;
	}
	mode[0] = sfile.st_mode;
	//printf("mode: %08x\n",*mode);
	fwrite(mode, 4, 1, fp);
	free(mode);

	//data
	if(sfile.st_mode & S_IFDIR){
	}else{
		buffer = (char*)malloc(sizeof(char)*(sfile.st_size));
  		if(buffer == NULL){
			fputs("Memory error(malloc)",stderr); fclose(origin); return 1;

		}
  		res = fread(buffer,1,sfile.st_size,origin);
  		if(res != sfile.st_mode && sfile.st_mode & S_IFDIR==0){
			fputs("Reading error",stderr); fclose(origin); return 1;
		}
		fwrite(buffer, sizeof(char), sfile.st_size, fp);
	}
	
	fclose(origin);
}

void
archivefd(FILE* fp, char* dirname)
{
	DIR *dp;
	struct dirent *ep;

	dp = opendir(dirname);
	if(dp!=NULL){
		char n[228];
		//printf("mkdir -%s\n",dirname);
		if(writefile(fp, dirname)){
			return ;
		}
		while(ep = readdir(dp)){
			//puts(ep->d_name);
			if(strcmp(ep->d_name,"..")!=0 && strcmp(ep->d_name,".")){
				strcpy(n, dirname);
				if(!endstr(dirname)){
					strcat(n,"/");
				}
				strcat(n,ep->d_name);
				printf("next -");
				puts(n);
				archivefd(fp,n);
			}
		}
		(void)closedir(dp);
	}else{
		//printf("mkfile-%s\n",dirname);
		if(writefile(fp, dirname)){
			return ;
		}
	}
}

void
showpath(char* buffer){
	//printf("%s\n",buffer);
	char* chp;
	char* cur = buffer;
	chp = strchr(buffer,'/');
	while(chp!=NULL){
		printf("\t");
		cur = chp+1;
		chp = strchr(cur, '/');
	}
	printf("> %s\n", (cur));
}

void
listup (char* filename)
{
	FILE* fp = fopen(filename, "rb");
  	if(fp==NULL){
		fputs("File error",stderr); exit (1);
	}
	long lsize = 0;
	char* buffer;
	unsigned int* buf;
	size_t res;

	fseek(fp, 0, SEEK_END);
	lsize = ftell(fp);
	rewind(fp);
	
	buf = (int*)malloc(sizeof(int));
	if(buf == NULL){
		fputs("Memory Error(malloc)",stderr); fclose(fp); return;
	}

	while(ftell(fp)<lsize){
		//pathlen
		res = fread(buf,4,1,fp);
		if(res!=1){
			fputs("Reading error(pathlen)",stderr); break;
		}
		//pathname
		buffer = (char*)malloc(sizeof(char)*((*buf)+1));
		if(buffer == NULL){
			fputs("Memory error(malloc)",stderr); break;
		}
		res = fread(buffer, sizeof(char), *buf, fp);
		if(res!=*buf){
			fputs("Reading error(pathname)", stderr); free(buffer); break;
		}
		buffer[*buf] = '\0';
		showpath(buffer);
		free(buffer);
		//dsize
		res = fread(buf,4,1,fp);
		if(res!=1){
			fputs("Reading error(dsize)",stderr); break;
		}
		unsigned int dsize = *buf;
		//mode
		res = fread(buf,4,1,fp);
		if(res!=1){
			fputs("Reading error(mode)",stderr); break;
		}

		if((*buf)&S_IFDIR){
			continue;
		}else{
			fseek(fp, dsize, SEEK_CUR);
		}
	}

	free(buf);
	fclose(fp);
}

void
extractfd (char* filename)
{
	FILE* fp = fopen(filename, "rb");
  	if(fp==NULL){
		fputs("File error",stderr); exit (1);
	}
	long lsize = 0;
	char* buffer;
	char* pathname;
	unsigned int* buf;
	size_t res;
	unsigned int dsize = 0;
	unsigned int mode = 0;
	
	fseek(fp, 0, SEEK_END);
	lsize = ftell(fp);
	rewind(fp);
	
	buf = (int*)malloc(sizeof(int));
	if(buf == NULL){
		fputs("Memory Error(malloc)",stderr); fclose(fp); return;
	}

	while(ftell(fp)<lsize){
		//pathlen
		res = fread(buf,4,1,fp);
		if(res!=1){
			fputs("Reading error(pathlen)",stderr); break;
		}
		//pathname
		pathname = (char*)malloc(sizeof(char)*((*buf)+1));
		if(pathname == NULL){
			fputs("Memory error(malloc)",stderr); break;
		}
		res = fread(pathname, sizeof(char), *buf, fp);
		if(res!=*buf){
			fputs("Reading error(pathname)", stderr); free(pathname); break;
		}
		pathname[*buf] = '\0';
		//printf("%s\n",pathname);
		showpath(pathname);
	
		dsize = 0;
		res = fread(buf,4,1,fp);
		if(res!=1){
			fputs("Reading error(dsize)",stderr); break;
		}
		dsize = *buf;
		
		mode = 0;
		res = fread(buf,4,1,fp);
		if(res!=1){
			fputs("Reading error(mode)",stderr); break;
		}
		mode = *buf;

		if((*buf)&S_IFDIR){
			DIR* dp;
			dp = opendir(pathname);
			if(dp==NULL){
				mkdir(pathname, mode);
				//printf("suc dir\n");
				continue;
			}
			(void)closedir(dp);
			//fputs("already exist directory. try again\n",stderr);
			//break;
		}else{
			//if(access(pathname,F_OK)!= -1){
			//	fputs("file is already exist. try again.\n",stderr); //break;
			//}
			FILE* nfp = fopen(pathname, "wb");
			buffer = (char*)malloc(sizeof(char)*(dsize));
			if(buffer == NULL){
				fputs("Memory error(malloc)",stderr); break;
			}
			res = fread(buffer, 1, dsize, fp);
			if(res!=dsize){
				fputs("Reading error(buffer)", stderr); free(pathname); free(buffer); break;
			}
			//printf("suc file\n");
			fwrite(buffer,1, dsize, nfp);
			chmod(pathname, mode);

			fclose(nfp);
			free(buffer);
		}
		free(pathname);
	}
	free(buf);
	fclose(fp);

}

int
main (int argc, char* argv[])
{
	int op = N_op;
	FILE* fp;

	switch (op = get_op_code(argv[1])) {
		case archive:
			if(access(argv[2],F_OK)!= -1){
				fputs("file is already exist. try again.\n",stderr); return 1;
			}
		       	fp = fopen(argv[2],"wb");
			archivefd(fp,argv[3]);
			fclose(fp);
			//printf("%s %s %s\n", argv[1], argv[2], argv[3]);
			break;
		case list:
			listup(argv[2]);
			//printf("%s %s \n", argv[1], argv[2]);
			break;
		case extract:
			extractfd(argv[2]);
			//printf("%s %s \n", argv[1], argv[2]);
			break;
		default: 
			printf("invalid operation\n");
			break;
	}
	return 0;
}

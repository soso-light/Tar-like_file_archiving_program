#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

int
endstr (char* dirname){
	int i=0;
	while(dirname[i]!='\0')
		i++;
	if(dirname[i-1] == '/')
		return 1;
	return 0;
}

int
copyfl (char* dsdir, char* pathname)
{
	FILE* dsfp = fopen(dsdir,"rb");
	FILE* cpfp = fopen(pathname,"wb");
	char* buffer;
	long lsize;
	size_t res;

	struct stat sfile;
  	if(stat(dsdir,&sfile)==-1){
    		printf("Error Occurred\n");
  	}

	fseek(dsfp,0,SEEK_END);
	lsize = ftell(dsfp);
	rewind(dsfp);

	buffer = (char*)malloc(sizeof(char)*lsize);

	res = fread(buffer,1,lsize, dsfp);
	if(res != lsize){ 
		return 1;
	}

	//copy
	fwrite(buffer, sizeof(char), lsize, cpfp); 

	chmod(pathname, sfile.st_mode);

	fclose(dsfp);
	fclose(cpfp);
	free(buffer);
	return 0;
}

void
readdirs (char* dsdir, char* cpdir)
{
	DIR *dp;
	struct dirent *ep;
	char c[228];
	char* ds = dsdir;
	strcpy(c,cpdir);
	if(ds[0]=='.'){
		ds++;
		if(ds[0]=='.'){
			ds++;
		}
	}
	if(ds[0]!='/'){
		strcat(c,"/");
        }
	strcat(c,ds);

	dp = opendir(dsdir);
	if(dp!=NULL){
		char n[228];
		mkdir(c, S_IRWXU);
		while(ep = readdir(dp)){
			//puts(ep->d_name);
			if(strcmp(ep->d_name,"..")!=0 && strcmp(ep->d_name,".")){
				strcpy(n, dsdir);
				if(!endstr(dsdir)){
					strcat(n,"/");
				}
				strcat(n,ep->d_name);
				puts(n);
				readdirs(n,cpdir);
			}
		}
		(void)closedir(dp);
	}else{
		if(copyfl(dsdir, c)) printf("err\n");
		printf("file-%s\n",c);
	}
	return ;
}


int
main (int argc, char* argv[])
{
	readdirs(argv[1], argv[2]);
	return 0;
}

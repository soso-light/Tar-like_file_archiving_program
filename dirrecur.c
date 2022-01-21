#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include<string.h>

int
endstr (char* dirname){
	int i=0;
	while(dirname[i+1],'\0')
		i++;
	if(dirname[i] == '/')
		return 1;
	return 0;
}

void
readdirs (char* dirname){
	DIR *dp;
	struct dirent *ep;

	dp = opendir(dirname);
	if(dp!=NULL){
		while(ep = readdir(dp)){
			//puts(ep->d_name);
			char n[228];
			strcpy(n, dirname);
			strcat(n,"/");
			strcat(n,ep->d_name);
			puts(n);

			if(strcmp(ep->d_name,"..")!=0 && strcmp(ep->d_name,".")){
				readdirs(n);
			}
		}
		(void)closedir(dp);
	}
	return;
}

int
main (void)
{
  char buf[128] ;
  scanf("%s",buf);

  readdirs(buf);

  return 0;
}

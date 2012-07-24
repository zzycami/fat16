/*
 *		fs/fs/main.c
 *
 *		(C) zhou.zheyong
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#include "kernel.h"
#include "fat16.h"

void operate(){
	char order[20];
	strcpy(order,"ls");
	while(strcmp(order, "exit")){
		printf("zfs>>~%s$ ",currentdir);
		scanf("%s", order);
		if(!strcmp(order, "ls")){
			ls();
		}else if(!strcmp(order, "format")){
			format();
		}else if(!strcmp(order, "mkdir")){
			char name[11];
			//printf("directory name(the length of the name < 11):");
			scanf("%s", name);
			mkdir(name);
		}else if(!strcmp(order, "cd")){
			char dirname[110];
			//printf("directory:");
			scanf("%s",dirname);
			cd(dirname);
		}else if(!strcmp(order, "deldir")){
			char dirname[110];
			//printf("directory:");
			scanf("%s",dirname);
			deldir(dirname);
		}else if(!strcmp(order, "create")){
			char filename[110];
			//printf("directory:");
			scanf("%s",filename);
			create(filename);
		}else if(!strcmp(order, "open")){
			char filename[110];
			//printf("directory:");
			scanf("%s",filename);
			open(filename);
		}else if(!strcmp(order, "close")){
			close(fileopenptr);
		}else if(!strcmp(order, "delfile")){
			char filename[110];
			//printf("directory:");
			scanf("%s",filename);
			delfile(filename);
		}else if(!strcmp(order, "write")){
			//printf("directory:");
			write(fileopenptr);
		}else if(!strcmp(order, "read")){
			//printf("directory:");
			read(fileopenptr);
		}else if(!strcmp(order, "h")){
			printf("zzycami file system\n");
			printf("version: %1.2f\n", VERSION);
			printf("Useage:[order] -[option]\n\n");
			printf("    ls      --list-files        list files and directory on current directory\n");
			printf("    format  --format            format this file system\n");
			printf("    cd      --change-directory	change current directory, example cd ./fs/include \n");
			printf("    mkdir   --make-directory    make directory in current path, example mkdir fs\n");
			printf("    h       --help              give this help\n");
			printf("    close   --close             close current opened file\n");
			printf("    open    --open              open a file at current directory\n");
			printf("    write   --write             write data to opened file\n");
			printf("    read    --read              read data from a opened file\n");
			printf("    deldir  --delete directory  delete a directory at current directory example deldir fs\n");
			printf("    delfile --delete file       delete a file at current directory, example delfile fs.txt\n");
			printf("    exit    --exit              exit this file system\n");
			printf("\n");
		} else if(!strcmp(order, "exit")){
			continue;
		}else {
			printf("no such order!\nget help by input h\n");
		}
	}
	
}

int main(){
	startsys();
	operate();
	existsys();
	return 0;
}

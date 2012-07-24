/*
 *		fs/fs/sys.c
 *
 *		(C) zhou.zheyong
 */
#include "kernel.h"
#include "fat16.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>


void startsys(){
	FILE *myfsys;
	fcb *root;
	block0 *boot;
	
	vhard = (unsigned char*)malloc(SIZE);
	memset(vhard,0,SIZE);
	if(myfsys = fopen("myfsys","r")){
		fread(vhard,SIZE, 1, myfsys);
		boot = (block0 *)vhard;
		printf("\nWelcome to %s\nversion:%1.2f\nblock size:%d\ntotal size:%d\nmax opened files:%d\nroot block location:%u\n\n", boot->name, boot->version, boot->blocksize, boot->size, boot->maxopenfile, boot->root);
		fclose(myfsys);
	}else {
		printf("myfsys is not exist, now create it\n");
		format();
		boot = (block0 *)vhard;
	}
	
	root = (fcb *)(vhard + boot->root*BLOCKSIZE);
	strcpy(openfilelist[0].filename, "root");
	openfilelist[0].attribute = 0x4;
	openfilelist[0].time = ((fcb *)(vhard + 5*BLOCKSIZE))->time;
	openfilelist[0].date = ((fcb *)(vhard + 5*BLOCKSIZE))->date;
	openfilelist[0].first = ((fcb *)(vhard + 5*BLOCKSIZE))->first;
	/* the root is a directory, so the length means how many fcb this directory contained */
	openfilelist[0].length = ((fcb *)(vhard + 5*BLOCKSIZE))->length;
	/*while(strcmp(root->filename, "")){
		openfilelist[0].length ++;
		root ++;
	}*/
	openfilelist[0].count = 1;
	openfilelist[0].dirno = 5;
	openfilelist[0].diroff = 0;
	openfilelist[0].fcbstate = 0;
	strcpy(currentdir, "/root");
	strcpy(openfilelist[0].dir[0], currentdir);
	openfilelist[0].free = 1;
	openfilelist[0].topenfile = 0;
	ptrcurdir = openfilelist[0];
	fileopenptr = 0;
}

void existsys(){
	FILE *myfsys;
	myfsys = fopen("myfsys", "w");
	fwrite(vhard,SIZE,1,myfsys);
	free(vhard);
	fclose(myfsys);
	printf("exit sucessed! bye!\n");
}

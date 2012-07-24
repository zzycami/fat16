/*
 *		fs/fs/disk.c
 *
 *		(C) zhou.zheyong
 */
#include "kernel.h"
#include "fat16.h"

#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>


void format(){
	FILE * myfsys;
	time_t *now;
	struct tm *current;
	block0 *boot;
	fat *fat1, *fat2;
	fcb *root;
	unsigned char * p = vhard;
	int i=0;

	boot = ((block0 *)p);
	
	strcpy(boot->name, "zzycami file system\0");
	boot->version = VERSION;
	boot->blocksize = BLOCKSIZE;
	boot->size = SIZE;
	boot->maxopenfile = MAXOPENFILE;
	boot->root = 5;
	
	printf("\nWelcome to %s\nversion:%1.2f\nblock size:%d\ntotal size:%d\nmax opened files:%d\nroot block location:%u\n\n", boot->name, boot->version, boot->blocksize, boot->size, boot->maxopenfile, boot->root);

	//build two fat
	p += BLOCKSIZE;
	fat1 = ((fat *)p);
	fat2 = ((fat *)(p + 2*BLOCKSIZE));
	//first five block is used
	for(i=0;i<5;i++){
		fat1->id = END;
		fat2->id = END;
		fat1 ++;
		fat2 ++;
	}
	fat1->id = 5;
	fat2->id = 5;
	fat1 ++;
	fat2 ++;
	for(i = 6;i<SIZE/BLOCKSIZE;i++){
		fat1->id = FREE;
		fat2->id = FREE;
		fat1 ++;
		fat2 ++;
	}

	p += 4*BLOCKSIZE;
	root = (fcb *)p;
	strcpy(root->filename, ".");
	root->attribute = 0x4;
	
	now = (time_t*)malloc(sizeof(time_t));
	time(now);
	current = localtime(now);
	root->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	root->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	root->first = 5;
	root->length = 2;

	root ++;
	strcpy(root->filename, "..");
	root->attribute = 0x4;
	time(now);
	current = localtime(now);
	root->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	root->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	root->first = 5;
	root->length = 1;
	root ++;

	for(i=2;i<BLOCKSIZE/sizeof(fcb);i++){
		root->filename[0] = '\0';
	}
	
	myfsys = fopen("myfsys", "w");
	fwrite(vhard, SIZE, 1, myfsys);
	fclose(myfsys);
}

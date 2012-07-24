/*
 *		fs/fs/dir.c
 *
 *		(C) zhou.zheyong
 */
 
#include "kernel.h"
#include "fat16.h"
#include "stack.h"
#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

void ls(){
	fcb *file;
	char type[11];
	int i = BLOCKSIZE/sizeof(fcb);
	file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	while((i--) > 0){
		int year, mon, day;
		int hour, min, sec;
		int length;
		if(!strcmp(file->filename, "")){
			file ++;
			continue;
		}
		
		if(file->attribute == 0x4){
			length = sizeof(fcb) * file->length;
			strcpy(type, "dir");
		}else {
			int l = strlen(file->filename);
			length = file->length;
			while(l--){
				if(file->filename[l] == '.'){
					strcpy(type, (char *)(file->filename + l*sizeof(char) + 1));
					break;
				}
			}
		}
		
		year = ((file->date & 0xfe00) >> 9) + 80 + 1900;
		mon = ((file->date & 0x01e0) >> 5) + 1;
		day = file->date & 0x001f;
		
		hour = (file->time & 0xf800) >> 11;
		min = (file->time & 0x07e0) >> 5;
		sec = (file->time & 0x001f) << 1;
		printf("%8d %04d-%02d-%02d %02d:%02d:%02d %6s %s\n",length , year, mon, day, hour, min, sec, type, file->filename);
		file ++;
	}
}

void mkdir(char *dirname){
	fcb * dir, *temp;
	time_t *now;
	struct tm *current;
	fat *table = (fat *)(vhard + BLOCKSIZE);
	fat *table2 = (fat *)(vhard + 3*BLOCKSIZE);
	int i = 0;

	
	//check the dir name
	if((!strcmp(dirname, "")) || (!strcmp(dirname, ".")) || (!strcmp(dirname, ".."))){
		printf("please input directory name!\n");
		return ;
	}
	for(i = 0; i < (int)strlen(dirname) ; i ++){
		if(dirname[i] == '/' 
			|| dirname[i] == '\\' 
			|| dirname[i] == ':' 
			|| dirname[i] == '*'
			|| dirname[i] == '?'
			|| dirname[i] == '"'
			|| dirname[i] == '<'
			|| dirname[i] == '>'
			|| dirname[i] == '|'){
				printf("directopry name or file name can't contain / \\ : * ? \" < > | \n");
				return ;
		}
	}
	
	temp = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i < BLOCKSIZE/sizeof(fcb) ; i ++){
		if(!strcmp(temp->filename, dirname)){
			printf("already have the file or directory has the same name, please create a new name!\n");
			return ;
		}
		temp ++;
	}

	//create the new fcb
	dir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i< BLOCKSIZE/sizeof(fcb); i++){
		if(!strcmp(dir->filename, "")){
			break;
		}
		dir ++;
	}
	//dir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE + ptrcurdir.length * sizeof(fcb));
	strcpy(dir->filename, dirname);
	dir->attribute = 0x4;
	
	now = (time_t*)malloc(sizeof(time_t));
	time(now);
	current = localtime(now);
	dir->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	dir->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	
	for(i=0;i<(BLOCKSIZE*2)/sizeof(fat);i++){
		//printf("i:%d - fat:%d\n",i,(int)table->id);
		if(table->id == FREE){
			break;
		}
		table ++;
		table2 ++;
	}
	dir->first = i;
	//printf("alloc block:%d\n", i);
	table->id = END;
	table2->id = END;
	dir->length = 2;
	ptrcurdir.length ++;
	((fcb*)(vhard + ptrcurdir.dirno * BLOCKSIZE + ptrcurdir.diroff * sizeof(fcb)))->length ++;

	// create two fcb . and ..
	dir = (fcb*)(vhard + dir->first * BLOCKSIZE);

	now = (time_t*)malloc(sizeof(time_t));
	strcpy(dir->filename, ".");
	dir->attribute = 0x4;
	time(now);
	current = localtime(now);
	dir->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	dir->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	/* the fcb->first that filename is '.' point to the current block id */
	dir->first = i;
	dir->length = 1;

	dir ++;
	strcpy(dir->filename, "..");
	dir->attribute = 0x4;
	time(now);
	current = localtime(now);
	dir->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	dir->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	/* the fcb.first that filename is '..' point to the id of father of current dir*/
	dir->first = ptrcurdir.first;
	dir->length = 1;
}

void cd(char *dirname){
	char dirs[MAXOPENFILE][80];
	int dirslength = 0;
	char currentdirc[MAXOPENFILE][80];
	int currentdirclength = 0;
	int x = 0, y = 0;
	int i = 0;
	fat* table = (fat *)(vhard + BLOCKSIZE);
	fcb* cdir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	fcb* root = (fcb *)(vhard + 5*BLOCKSIZE);
	int off;
	int limit;
	int issort = 0;
	
	//divide the current path
	i = 1;
	while((currentdir[i] != '\0') && (i < 110)){
		if(currentdir[i] != '/'){
			currentdirc[x][y] = currentdir[i];
			y ++;
		}else {
			currentdirc[x][y] = '\0';
			y = 0;
			x ++;
		}
		i++;
	}
	currentdirc[x][y] = '\0';
	currentdirclength = x + 1;

	//divide the order path
	dirslength = strlen(dirname);
	if(dirname[dirslength - 1] == '/'){
		dirname[dirslength - 1] = '\0';
	}
	x = 0; y = 0;i = 0;
	while((dirname[i] != '\0') && (i < 110)){
		if(dirname[i] != '/'){
			dirs[x][y] = dirname[i];
			y ++;
		}else {
			dirs[x][y] = '\0';
			y = 0;
			x ++;
		}
		i++;
	}
	dirs[x][y] = '\0';
	dirslength = x + 1;
	
	//the method to change directory
	for(i = 0;i < dirslength;i ++){
		int isfind = 0;
		limit = BLOCKSIZE/sizeof(fcb);//avoid cdir over one blocksize
		while(limit--){
			if((!strcmp(cdir->filename, dirs[i])) && (cdir->attribute == 0x4)){
				isfind = 1;
				break;
			}
			cdir ++;
		}
		if(isfind == 0){
			printf("have no such directory!\n");
			return ;
		}else {
			cdir = (fcb*)(vhard + cdir->first * BLOCKSIZE);
		}
	}

	/* find the absolute directory of current directory */
	x = dirslength;i = 0;
	while((x--) > 0){
		if(!strcmp(dirs[i], ".")){
			//do nothing
		}else if(!strcmp(dirs[i], "..")){
			currentdirclength = currentdirclength > 1?currentdirclength - 1:currentdirclength;
		}else {
			if(!strcmp(dirs[i], "")){
				strcpy(currentdirc[currentdirclength ++], "");
			}else {
				strcpy(currentdirc[currentdirclength++], dirs[i]);
			}
		}
		i ++;
	}
	
	strcpy(currentdir, "");
	for(i=0;i<currentdirclength;i++){
		strcat(currentdir, "/");
		strcat(currentdir,currentdirc[i]);
	}
	

	/* fill the openfilelist */
	cdir ++;off = 0;
	cdir = (fcb *)(vhard + cdir->first * BLOCKSIZE);
	openfilelist[fileopenptr].dirno = cdir->first;
	if(strcmp(currentdirc[currentdirclength-1], "root")){
		limit = BLOCKSIZE/sizeof(fcb);//avoid cdir over one blocksize
		while(limit--){
			if(!strcmp(cdir->filename, currentdirc[currentdirclength-1])){
				break;
			}
			cdir ++;
			off ++;
		}
	}
	strcpy(openfilelist[fileopenptr].filename, cdir->filename);
	openfilelist[fileopenptr].attribute = 0x4;
	openfilelist[fileopenptr].time = cdir->time;
	openfilelist[fileopenptr].date = cdir->date;
	openfilelist[fileopenptr].first = cdir->first;
	openfilelist[fileopenptr].length = cdir->length;
	openfilelist[fileopenptr].count = 1;
	openfilelist[fileopenptr].diroff = off;
	openfilelist[fileopenptr].fcbstate = 0;
	for(i=0;i<currentdirclength;i++){
		strcpy(openfilelist[fileopenptr].dir[i], currentdirc[i]);
	}
	openfilelist[fileopenptr].free = 1;
	openfilelist[fileopenptr].topenfile = 0;
	ptrcurdir = openfilelist[fileopenptr];
}


void deldir(char *dirname){
	fat * fat1, *fat2;
	fcb *dir, *ddir;
	int i = 0;
	//check the dir name
	if((!strcmp(dirname, "")) || (!strcmp(dirname, ".")) || (!strcmp(dirname, ".."))){
		printf("please input directory name!\n");
		return ;
	}
	for(i = 0; i < (int)strlen(dirname) ; i ++){
		if(dirname[i] == '/' 
			|| dirname[i] == '\\' 
			|| dirname[i] == ':' 
			|| dirname[i] == '*'
			|| dirname[i] == '?'
			|| dirname[i] == '"'
			|| dirname[i] == '<'
			|| dirname[i] == '>'
			|| dirname[i] == '|'){
				printf("directopry name or file name can't contain / \\ : * ? \" < > | \n");
				return ;
		}
	}
	
	ddir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i < BLOCKSIZE/sizeof(fcb); i ++){
		if(!strcmp(ddir->filename, dirname)){
			break;
		}
		ddir ++;
	}
	if(i == BLOCKSIZE/sizeof(fcb)){
		printf("have no such directory!\n");
		return ;
	}else {
		//delete the directory
		int all = BLOCKSIZE/sizeof(fcb);

		((fcb*)(vhard + ptrcurdir.dirno * BLOCKSIZE + ptrcurdir.diroff * sizeof(fcb)))->length --;
		fat1 = (fat *)(vhard + BLOCKSIZE);
		fat2 = (fat *)(vhard + 3*BLOCKSIZE);

		// Traverse and delete
		clear();
		push(ddir);
		while(!isempty()){
			fcb* temp;
			int all = BLOCKSIZE/sizeof(fcb) - 2;
			dir = (fcb *)pop();
			temp = dir;
			dir = (fcb *)(vhard + dir->first * BLOCKSIZE + 2 * sizeof(fcb));
			memset(temp, 0, sizeof(fcb));
			while((all --) > 0){
				if(strcmp(dir->filename, "")){
					if(dir->attribute == 0x4){
						//if it is a directory
						fat1 = (fat *)(vhard + BLOCKSIZE);
						fat2 = (fat *)(vhard + 3*BLOCKSIZE);
						fat1 += dir->first;
						fat2 += dir->first;
						if(fat1->id == END){
							fat1->id = FREE;
							fat2->id = FREE;
						}else {
							printf("error ocurred when modify fat!\n");
						}
						push((void *)dir);
					}else if(dir->attribute == 0x5 ){//delete the file
						delfile(dir->filename);
					}//else if(dir->attribute == 0x5 )

				}//if(strcmp(dir->filename, ""))
				dir ++;
			}//while((all --) > 0)
		}//while((all --) > 0)
	}
}



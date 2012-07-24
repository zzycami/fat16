/*
 *		fs/fs/file.c
 *
 *		(C) zhou.zheyong
 */
 
#include "kernel.h"
#include "fat16.h"

#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>


int create(char *filename){
	int i = 0;
	fcb *temp, *file;
	time_t *now;
	fat *fat1, *fat2;
	struct tm *current;
	//check the dir name
		if((!strcmp(filename, "")) || (!strcmp(filename, ".")) || (!strcmp(filename, ".."))){
			printf("please input directory name!\n");
			return -1;
		}
		for(i = 0; i < (int)strlen(filename) ; i ++){
			if(filename[i] == '/'
				|| filename[i] == '\\'
				|| filename[i] == ':'
				|| filename[i] == '*'
				|| filename[i] == '?'
				|| filename[i] == '"'
				|| filename[i] == '<'
				|| filename[i] == '>'
				|| filename[i] == '|'){
					printf("file name or file name can't contain / \\ : * ? \" < > | \n");
					return -1;
			}
		}

		temp = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
		for(i = 0;i < BLOCKSIZE/sizeof(fcb) ; i ++){
			if(!strcmp(temp->filename, filename)){
				printf("already have the file or directory has the same name, please create a new name!\n");
				return -1;
			}
			temp ++;
		}

		//create the new fcb
		file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
		for(i = 0;i< BLOCKSIZE/sizeof(fcb); i++){
			if(!strcmp(file->filename, "")){
				break;
			}
			file ++;
		}
		if(i == BLOCKSIZE/sizeof(fcb)){
			printf("current directory is full, please change a directory and create new file!\n");
		}else {
			int j = 0;
			strcpy(file->filename, filename);
			file->attribute = 0x5;
			now = (time_t *)malloc(sizeof(time_t));
			time(now);
			current = localtime(now);
			file->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
			file->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);

			fat1 = (fat *)(vhard + BLOCKSIZE);
			fat2 = (fat *)(vhard + 3*BLOCKSIZE);
			for(j = 0;j < (2*BLOCKSIZE)/sizeof(fat);j++){
				if(fat1->id == FREE){
					break;
				}
				fat1 ++;
				fat2 ++;
			}

			file->first = j;
			fat1->id = END;
			fat2->id = END;
			file->length = BLOCKSIZE;
		}

		return 0;
}


int open(char *filename){
	fcb *file;
	fat *fat1;
	int limit, isfind;
	int l, i, off;
	char name[81];
	char exname[10];
	
	limit = BLOCKSIZE/sizeof(fcb);
	file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	isfind = 0;off = 0;
	while((limit --) > 0){
		// deal the file name
		l = strlen(file->filename);
		strcpy(name, file->filename);
		while(l --){
			if(name[l] == '.'){
				strcpy(exname, (char *)(name + l*sizeof(char) + 1));
				name[l] = '\0';
				break;
			}
		}

		// find 
		if((!strcmp(file->filename, filename) || !strcmp(name, filename)) && file->attribute == 0x5){
			isfind = 1;
			break;
		}
		file ++;
		off ++;
	}
	if(!isfind){
		printf("file doesn't exist!\n");
		return -1;
	}else {
		int j =0;
		//check free user open file list
		for(j = 0 ; j< MAXOPENFILE; j ++){
			if(!strcmp(openfilelist[j].filename, "")){
				fileopenptr = j;
				break;
			}else if(!strcmp(openfilelist[j].filename, filename)){
				fileopenptr = j;
				printf("you have already opened this file, file id is %d", fileopenptr);
				return j;
			}
		}
		strcpy(openfilelist[fileopenptr].filename, file->filename);
		openfilelist[fileopenptr].attribute = file->attribute;
		openfilelist[fileopenptr].date = file->date;
		openfilelist[fileopenptr].time = file->time;
		openfilelist[fileopenptr].dirno = ptrcurdir.first;
		openfilelist[fileopenptr].first = ((fcb*)(vhard + ptrcurdir.first * BLOCKSIZE + off * sizeof(fcb)))->first;
		openfilelist[fileopenptr].diroff = off;
		openfilelist[fileopenptr].fcbstate = 0;
		openfilelist[fileopenptr].length = file->length;
		openfilelist[fileopenptr].topenfile = 1;
		openfilelist[fileopenptr].fcbstate = 0;
		printf("open file suceed, resource id %d\n", fileopenptr);
		return fileopenptr;
	}
	return 0;
}

void close(int fid){
	if(fid < 0 || fid > MAXOPENFILE){
		printf("illegal file id(%d)!\n", fid);
		return ;
	}

	if(!strcmp(openfilelist[fid].filename, "")){
		printf("you havn't open file whose fid is %d!", fid);
		return ;
	}

	strcpy(openfilelist[fid].filename, "");
	openfilelist[fid].attribute = 0x0;
	openfilelist[fid].date = 0;
	openfilelist[fid].time = 0;
	openfilelist[fid].dirno = 0;
	openfilelist[fid].first = 0;
	openfilelist[fid].diroff = 0;
	openfilelist[fid].fcbstate = 0;
	openfilelist[fid].length = 0;
	openfilelist[fid].topenfile = 0;
	openfilelist[fid].fcbstate = 0;
	printf("close file suceed\n");
	fileopenptr = -1;
}

int write(int fid){
	char text[BLOCKSIZE + 1];
	fcb * file = (fcb *)(vhard + openfilelist[fid].dirno * BLOCKSIZE + openfilelist[fid].diroff * sizeof(fcb));
	int point;
	int len;
	char c;
	if(openfilelist[fid].attribute != 0x5){
		printf("have no opened file or the file you opened can't write!\n");
		return -1;
	}
	printf("Edit Mode:\n");
	point = 0;
	openfilelist[fid].length = 0;
	getchar();//get one char
	while((c = getchar()) != EOF){
		if(point == BLOCKSIZE){
			if(dowrite(fid, text, BLOCKSIZE, 3) == -1){
				printf("write file error!\n");
				return -1;
			}
			openfilelist[fid].length += point;
			point = 0;
		}
		text[point ++] = c;
	}
	//len = strlen(text);
	openfilelist[fid].length += point;
	file->length = openfilelist[fid].length;
	if(dowrite(fid, text, point, 3) == -1){
		printf("write file error!\n");
		return -1;
	}
	printf("\n");

}

int dowrite(int fid, char *text, int len, char wstyle){
	char *p;
	fat * fat1, *fat2;
	if(len >= BLOCKSIZE){//if previous block is used up
		int i = 0;
		fat *temp = (fat *)(vhard + BLOCKSIZE);
		// find the last block
		fat1 = (fat *)(vhard + BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		i = openfilelist[fid].first;
		while(fat1->id != END){
			if(fat1->id == FREE){
				printf("fat error!\n");
				return -1;
			}
			i = fat1->id;
			fat1 = (fat *)(vhard + BLOCKSIZE + fat1->id * sizeof(fat));
			fat2 = (fat *)(vhard + 3 * BLOCKSIZE + fat1->id * sizeof(fat));
		}
		//fill the end block
		p = (char *)(vhard + i * BLOCKSIZE);
		strcpy(p, text);
		// find a free block
		for(i = 0; i < 2*BLOCKSIZE/sizeof(fat); i++){
			if(temp->id == FREE){
				break;
			}
			temp ++;
		}
		fat1->id = i;
		fat2->id = i;
		fat1 = (fat *)(vhard + BLOCKSIZE + i * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + i * sizeof(fat));
		fat1->id = END;
		fat2->id = END;
	}else{// the text length is shorter than one block
		int i = 0;
		fat *temp = (fat *)(vhard + BLOCKSIZE);
		// find the last block
		fat1 = (fat *)(vhard + BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		i = openfilelist[fid].first;
		while(fat1->id != END){
			if(fat1->id == FREE){
				printf("fat error!\n");
				return -1;
			}
			i = fat1->id;
			fat1 = (fat *)(vhard + BLOCKSIZE + fat1->id * sizeof(fat));
			fat2 = (fat *)(vhard + 3 * BLOCKSIZE + fat1->id * sizeof(fat));
		}
		//fill the end block
		p = (char *)(vhard + i * BLOCKSIZE);
		strcpy(p, text);
	}
	return 0;
}


int read(int fid){
	char *text = (char *)malloc(2*BLOCKSIZE);
	fcb *file;
	fat *fat1;
	int i = 0;
	int len = openfilelist[fileopenptr].length;

	if(fid > MAXOPENFILE){
		if(fileopenptr > MAXOPENFILE){
			printf("file resrouce id is error , may be you have not open it");
		}
		fid = fileopenptr;
	}
	if(openfilelist[fileopenptr].attribute != 0x5){
		printf("have no opened file or the file you opened can't write!\n");
		return -1;
	}

	if(openfilelist[fileopenptr].length < BLOCKSIZE){
		len = openfilelist[fileopenptr].length;
		text = (char *)(vhard + openfilelist[fileopenptr].first * BLOCKSIZE);
		i = 0;
		printf("Read Mode | file length:%d\n", len);
		while((len --) > 0){
			putchar(text[i ++]);
		}
		printf("\n");
	}else {
		int id = 0;
		len = openfilelist[fileopenptr].length;
		fat1 = (fat*)(vhard + BLOCKSIZE + openfilelist[fileopenptr].first * sizeof(fat));
		text = (char *)(vhard + openfilelist[fileopenptr].first * BLOCKSIZE);
		for(i = 0;i <= BLOCKSIZE; i++){
			putchar(text[i]);
		}
		while(fat1->id != END){
			int limit;
			if(fat1->id == FREE){
				printf("fat error!\n");
				return -1;
			}
			len -= BLOCKSIZE;
			limit = len < BLOCKSIZE?len:BLOCKSIZE;
			text = (char *)(vhard + fat1->id * BLOCKSIZE);
			for(i = 0;i<= limit; i++){
				putchar(text[i]);
			}
			fat1 = (fat *)(vhard + BLOCKSIZE + fat1->id * sizeof(fat));
		}
	}
	return 0;
}


void delfile(char *filename){
	fat * fat1, *fat2;
	fcb *file1;
	char *file2, name[100], exname[20];
	int isfind = 0;
	int i = 0;
	//check the file name
	if((!strcmp(filename, "")) || (!strcmp(filename, ".")) || (!strcmp(filename, ".."))){
		printf("please input file name!\n");
		return ;
	}
	for(i = 0; i < (int)strlen(filename) ; i ++){
		if(filename[i] == '/' 
			|| filename[i] == '\\' 
			|| filename[i] == ':' 
			|| filename[i] == '*'
			|| filename[i] == '?'
			|| filename[i] == '"'
			|| filename[i] == '<'
			|| filename[i] == '>'
			|| filename[i] == '|'){
				printf("directopry name or file name can't contain / \\ : * ? \" < > | \n");
				return ;
		}
	}
	
	file1 = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i < BLOCKSIZE/sizeof(fcb); i ++){
		int l = strlen(file1->filename);
		strcpy(name, file1->filename);
		while(l --){
			if(name[l] == '.'){
				strcpy(exname, (char *)(name + l*sizeof(char) + 1));
				name[l] = '\0';
				break;
			}
		}
		// find 
		if((!strcmp(file1->filename, filename) || !strcmp(name, filename)) && file1->attribute == 0x5){
			isfind = 1;
			break;
		}
		file1 ++;
	}
	//delete data in fat
	fat1 = (fat *)(vhard + BLOCKSIZE + file1->first * sizeof(fat));
	fat2 = (fat *)(vhard + 3 * BLOCKSIZE + file1->first * sizeof(fat));
	while(fat1->id != END){
		int id = fat1->id;
		if(fat1->id == FREE){
			printf("fat error!\n");
			return ;
		}
		file2 = (char *)(vhard + fat1->id * BLOCKSIZE);
		memset(file2,0,BLOCKSIZE);
		fat1->id = FREE;
		fat2->id = FREE;
		fat1 = (fat *)(vhard + BLOCKSIZE + id * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + id * sizeof(fat));
	}
	fat1->id = FREE;
	fat2->id = FREE;
	file2 = (char *)(vhard + file1->first * BLOCKSIZE);
	memset(file2,0,BLOCKSIZE);
	// delete fcb
	memset(file1, 0, sizeof(fcb));
}
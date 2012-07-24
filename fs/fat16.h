#include "const.h"
#ifndef _FAT16_H
#define _FAT16_H

/* file control block*/
#ifndef __FS_FAT16_FCB__
#define __FS_FAT16_FCB__
typedef struct FCB{
	char filename[11];/*file name include extend file name*/
	unsigned char attribute;/*file attribution*/
	unsigned char other[10];
	unsigned short time;/*file create time*/
	unsigned short date;/*file create data*/
	unsigned short first;/*file start disk block*/
	unsigned int length;/*file length*/
}fcb;
#endif//__FS_FAT16_FCB__

/* file allocation table */
#ifndef __FS_FAT16_FAT__
#define __FS_FAT16_FAT__
typedef struct FAT{
	unsigned short id;
}fat;
#endif//__FS_FAT16_FAT__

/* user open table*/
#ifndef __FS_FAT16_USEROPEN__
#define __FS_FAT16_USEROPEN__
typedef struct USEROPEN{
	char filename[11];/*file name include extend file name*/
	unsigned char attribute;/*file attribution*/
	unsigned short time;/*file create time*/
	unsigned short date;/*file create data*/
	unsigned short first;/*file start disk block*/
	unsigned short length;/*file length*/
	char free;/*mark whether this directory is empty, 0 means empty, 1 means assigned*/
	/* previous content is fcb, in the next recode the directory of it's parent's that opened */
	int dirno; /*the disk block id of the directory of the opened file in it's parent's */
	int diroff; /*the index id of the disk block id of the directory of the opened file in it's parent's*/
	char dir[MAXOPENFILE][80]; /* the directory of opened file, so as to check whether the file is opened faster*/
	int count; /* the position of pointer in the file */
	char fcbstate; /* whether modified the content of fcb, if modified set 1, or set 0*/
	char topenfile; /* wheather opened table item is empty, if the value is 0 empty, or occupied */
}useropen;
#endif//__FS_FAT16_USEROPEN__

#ifndef __FS_FAT16_BLOCK0__
#define __FS_FAT16_BLOCK0__
typedef struct BLOCK0 {
	char name[20];/* name of the file system */
	float version;
	int blocksize;
	int size;
	int maxopenfile;
	unsigned short root;
}block0;
#endif //__FS_FAT16_BLOCK0__

unsigned char *vhard; /* pointer to the start of vertual disk*/
useropen openfilelist[MAXOPENFILE]; /* array of user open file list*/
int fileopenptr;
useropen ptrcurdir; /*pointer to current user open list*/
char currentdir[80]; /*recode cureent dir (include path)*/
unsigned char* startp; /* recode the start position in data area of vertual disk*/

#endif //_FAT16_H

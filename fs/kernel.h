/* kernel.h contain all the function prototypes */
#ifndef _KERNEL_H
#define _KERNEL_H
/* system function prototypes */
void startsys();
void existsys();

/* disk function prototypes */
void format();

/* directory function prototypes */
void mkdir(char *dirname);
void cd(char *dirname);
void deldir(char *dirname);
void ls();

/* file function prototypes */
int create(char *filename);
void delfile(char *filename);
int open(char *filename);
void close(int fid);
int write(int fid);
int dowrite(int fid, char *text, int len, char wstyle);
int read(int fid);
#endif

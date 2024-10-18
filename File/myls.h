#pragma once

#include <dirent.h>
#include <sys/stat.h>

extern int CAPACITY;
extern int terminalWidth;

int runLs(const int argc, const char **argv);
int getFileList(char ***files, const char *path, const int isHidden);
DIR* getDir(const char *path);
int ascend(const void *a, const void *b);
int getRows(const char **files, const int fileCount);
int getMaxCols(const char **files, const int fileCount, const int rows);
int getMaxLen(const char **files, int st, int ed);
void printFiles(const char **files, const int fileCount);
void recursiveDir(const char *path, const int isHidden, const int isLong);
void printFilePermissions(mode_t mode);
void printInfo(const char **files, const char *path, const int fileCount);
int getCountInt(int a);
char* allocatePath(const char *path, const char *file);
void getFileStat(const char *buf, struct stat *st);

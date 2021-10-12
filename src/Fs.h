// Interface to the File System ADT

// !!! DO NOT MODIFY THIS FILE !!!

#ifndef FS_H
#define FS_H

// Chiro: TODO: Remove stdbool
#include <stdbool.h>

// Chiro: TODO: Remove FS_
#define FS_PATH_MAX 4096

typedef struct FsRep *Fs;

Fs FsNew(void);

void FsGetCwd(Fs fs, char cwd[FS_PATH_MAX + 1]);

void FsFree(Fs fs);

void FsMkdir(Fs fs, char *path);

void FsMkfile(Fs fs, char *path);

void FsCd(Fs fs, char *path);

void FsLs(Fs fs, char *path);

void FsPwd(Fs fs);

void FsTree(Fs fs, char *path);

void FsPut(Fs fs, char *pathStr, char *content);

void FsCat(Fs fs, char *path);

void FsDldir(Fs fs, char *path);

void FsDl(Fs fs, bool recursive, char *path);

void FsCp(Fs fs, bool recursive, char *src[], char *dest);

void FsMv(Fs fs, char *src[], char *dest);

// TODO: remove this
void FsFilPrint(FIL *file);

FsErrors FsPathParse(PATH *pathRoot, const char *pathStr, PATH **path);

void FsPrint(Fs fs, char *pathStr);

#endif

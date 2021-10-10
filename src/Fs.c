// Implementation of the File System ADT
// Written by:
// Date:

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FileType.h"
#include "Fs.h"
#include "utility.h"

struct FsRep {
    
};

Fs FsNew(void) {
    // TODO
    return NULL;
}

void FsGetCwd(Fs fs, char cwd[FS_PATH_MAX + 1]) {
    // TODO
}

void FsFree(Fs fs) {
    // TODO
}

void FsMkdir(Fs fs, char *path) {
    // TODO
}

void FsMkfile(Fs fs, char *path) {
    // TODO
}

void FsCd(Fs fs, char *path) {
    // TODO
}

void FsLs(Fs fs, char *path) {
    // TODO
}

void FsPwd(Fs fs) {
    // TODO
}

void FsTree(Fs fs, char *path) {
    // TODO
}

void FsPut(Fs fs, char *path, char *content) {
    // TODO
}

void FsCat(Fs fs, char *path) {
    // TODO
}

void FsDldir(Fs fs, char *path) {
    // TODO
}

void FsDl(Fs fs, bool recursive, char *path) {
    // TODO
}

void FsCp(Fs fs, bool recursive, char *src[], char *dest) {
    // TODO
}

void FsMv(Fs fs, char *src[], char *dest) {
    // TODO
}


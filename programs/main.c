//
// Created by Chiro on 2021/10/10.
//

#include "FileType.h"
#include "Fs.h"
#include "utility.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void bash(Fs fs_) {
  Fs fs = fs_;
  puts("======= WHERECOME TO BASH ========");
  if (!fs) {
    fs = FsNew();
  }
  char input[FS_PATH_MAX];
  char cwd[FS_PATH_MAX];
  int to_exit = 0;
  while (!to_exit) {
    FsGetCwd(fs, cwd);
    printf("\n%s > ", cwd);
    fflush(stdout);
    gets(input);
    char *arg = input;
    if (!*input)
      continue;
    while (arg < input + FS_PATH_MAX && *arg && *arg != ' ') {
      arg++;
    }
    if (*arg == ' ') {
      *arg = '\0';
      arg++;
    }
    char *name = input;
    // printf("\tname: %s, arg: %s\n", name, arg);
    if (!*arg)
      arg = NULL;

    if (strcmp(name, "exit") == 0) {
      to_exit = 1;
    } else if (strcmp(name, "mkdir") == 0) {
      FsMkdir(fs, arg);
    } else if (strcmp(name, "cd") == 0) {
      FsCd(fs, arg);
    } else if (strcmp(name, "pwd") == 0) {
      FsPwd(fs);
    } else if (strcmp(name, "tree") == 0) {
      FsTree(fs, arg);
    } else if (strcmp(name, "ls") == 0) {
      FsLs(fs, arg);
    } else if (strcmp(name, "cat") == 0) {
      FsCat(fs, arg);
    } else if (strcmp(name, "rmdir") == 0) {
      FsDldir(fs, arg);
    } else if (strcmp(name, "mkfile") == 0) {
      FsMkfile(fs, arg);
    } else {
      printf("Unknown command: %s\n", name);
    }
  }
  if (!fs_)
    FsFree(fs);
  puts("======= BYE ========");
}

int main(int argc, char **argv) {
  char buf[FS_PATH_MAX];
  Fs fs = FsNew();
  FsMkdir(fs, "dir1");
  FsCd(fs, "dir1");
  FsGetCwd(fs, buf);
  // printf("cwd: %s\n", buf);
  // FsTree(fs, "/");
  FsMkdir(fs, "dir2");
  // FsCd(fs, "/dir1/dir2");
  FsGetCwd(fs, buf);
  printf("cwd: %s\n", buf);
  FsTree(fs, "/");
  FsCd(fs, "/");
  FsMkdir(fs, "dir3");
  FsTree(fs, "/");
  FsTree(fs, "/dir1");

  puts("=================");
  FsCd(fs, "");
  FsGetCwd(fs, buf);
  printf("cwd: %s\n", buf);

  puts("=================");
  // FsCd(fs, "/dir1/dir2/");
  FsCd(fs, "");
  FsLs(fs, NULL);


  printf("ALL DONE\n");
  // FsFree(fs);

  bash(fs);
  return 0;
}

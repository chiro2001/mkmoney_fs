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
    } else if (strcmp(name, "put") == 0) {
      char *content = arg;
      while (*content && *content != ' ')
        content++;
      if (*content == ' ') {
        *content = '\0';
        content++;
      } else {
        content = "";
      }
      FsPut(fs, arg, content);
    } else if (strcmp(name, "rmdir") == 0) {
      FsDldir(fs, arg);
    } else if (strcmp(name, "mkfile") == 0) {
      FsMkfile(fs, arg);
    } else if (strcmp(name, "rm") == 0) {
      bool recursive = false;
      if (*arg == '-' && *(arg + 1) == 'r' && *(arg + 2) == ' ' && *(arg + 3)) {
        recursive = true;
        arg += 3;
      }
      FsDl(fs, recursive, arg);
    } else if (strcmp(name, "cp") == 0) {
      bool recursive = false;
      if (*arg == '-' && *(arg + 1) == 'r' && *(arg + 2) == ' ' && *(arg + 3)) {
        recursive = true;
        arg += 3;
      }
      char *arg2 = arg;
      while (*arg && *arg != ' ')
        arg++;
      if (*arg == ' ')
        *(arg++) = '\0';
      char *srcFiles[] = {arg2, NULL};
      FsCp(fs, recursive, srcFiles, arg);
    } else if (strcmp(name, "mv") == 0) {
      char *arg2 = arg;
      while (*arg && *arg != ' ')
        arg++;
      if (*arg == ' ')
        *(arg++) = '\0';
      char *srcFiles[] = {arg2, NULL};
      FsMv(fs, srcFiles, arg);
    } else if (strcmp(name, "print") == 0) {
      FsPrint(fs, arg);
    } else {
      printf("Unknown command: %s\n", name);
    }
  }
  if (!fs_)
    FsFree(fs);
  puts("======= BYE ========");
}

int main(int argc, char **argv) {
  printf("########### TEST START ##########\n");

  printf("########### TEST 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "/tmp");
    FsMkdir(fs, "tmp");
    FsMkdir(fs, "./tmp");
    FsFree(fs);
  }
  printf("########### TEST 1 DONE ##########\n");

  printf("########### TEST 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsMkfile(fs, "hello/world");
    FsMkdir(fs, "html");
    FsMkfile(fs, "html/index.html");
    FsMkfile(fs, "html/index.html/hi");
    FsTree(fs, NULL);
  }
  printf("########### TEST 2 DONE ##########\n");

  printf("########### TEST 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "tmp");
    FsCd(fs, "tmp");
    FsCd(fs, NULL);
    FsMkfile(fs, "hello.txt");
    FsTree(fs, NULL);
  }
  printf("########### TEST 3 DONE ##########\n");

  printf("########### TEST 4 START ##########\n");
  {
    Fs fs = FsNew();
    printf("---\n"); // marker to separate output
    FsLs(fs, "/");
    printf("---\n");
    FsMkfile(fs, "hello.txt");
    FsMkdir(fs, "tmp");
    FsLs(fs, "/");
  }
  printf("########### TEST 4 DONE ##########\n");

  printf("########### TEST 5 START ##########\n");
  {
    Fs fs = FsNew();
    FsPwd(fs);
    FsMkdir(fs, "home");
    FsCd(fs, "home");
    FsPwd(fs);
    FsMkdir(fs, "tim");
    FsCd(fs, "tim");
    FsPwd(fs);
  }
  printf("########### TEST 5 DONE ##########\n");

  printf("########### TEST 6 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsTree(fs, "hello");
    FsTree(fs, "./hello/world");
    FsFree(fs);
  }
  printf("########### TEST 6 DONE ##########\n");

  printf("########### TEST 7 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsPut(fs, "hello.txt", "hello\n");
    FsPut(fs, "./hello.txt", "world\n"); // overwrites existing content
    FsFree(fs);
  }
  printf("########### TEST 7 DONE ##########\n");

  printf("########### TEST 8 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsPut(fs, "hello/world", "random-message\n");
    FsFree(fs);
  }
  printf("########### TEST 8 DONE ##########\n");

  printf("########### TEST 9 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsCat(fs, "hello");
    FsCat(fs, ".");
    FsCat(fs, "/");
    FsFree(fs);
  }
  printf("########### TEST 9 DONE ##########\n");

  printf("########### TEST 10 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsMkdir(fs, "hello/world");
    FsDldir(fs, "hello");
    FsTree(fs, NULL);
    FsFree(fs);
  }
  printf("########### TEST 10 DONE ##########\n");

  printf("########### TEST 11 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsDl(fs, false, "hello");
    FsFree(fs);
  }
  printf("########### TEST 11 DONE ##########\n");

  printf("########### TEST 12 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsPut(fs, "hello.txt", "hello\n");
    FsMkfile(fs, "world.txt");
    FsPut(fs, "world.txt", "world\n");
    FsCat(fs, "world.txt");
    printf("---\n");
    char *src[] = {"hello.txt", NULL};
    FsCp(fs, false, src, "world.txt");
    FsCat(fs, "world.txt");
    printf("---\n");
    FsTree(fs, NULL);
    FsFree(fs);
  }
  printf("########### TEST 12 DONE ##########\n");

  printf("########### TEST 13 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsPut(fs, "hello.txt", "hello\n");
    FsTree(fs, NULL);
    printf("---\n");
    char *src[] = {"hello.txt", NULL};
    FsMv(fs, src, "world.txt");
    FsTree(fs, NULL);
    printf("---\n");
    FsCat(fs, "world.txt");
    FsFree(fs);
  }
  printf("########### TEST 13 DONE ##########\n");

  printf("########### ALL DONE ##########\n");
  bash(NULL);
  return 0;
}

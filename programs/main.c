//
// Created by Chiro on 2021/10/10.
//

#include "FileType.h"
#include "utility.h"
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#include "Fs.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void bash(Fs fs_) {
  Fs fs = fs_;
  puts("======= WHERECOME TO BASH ========");
  if (!fs) {
    fs = FsNew();
  }
  char input[PATH_MAX];
  char cwd[PATH_MAX];
  int to_exit = 0;
  while (!to_exit) {
    FsGetCwd(fs, cwd);
    printf("\n%s > ", cwd);
    fflush(stdout);
    gets(input);
    char *arg = input;
    if (!*input)
      continue;
    while (arg < input + PATH_MAX && *arg && *arg != ' ') {
      arg++;
    }
    if (*arg == ' ') {
      *arg = '\0';
      arg++;
    }
    char *name = input;
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
    } else {
      printf("Unknown command: %s\n", name);
    }
  }
  if (!fs_)
    FsFree(fs);
  puts("======= BYE ========");
}

void TestExamples() {
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
    FsTree(fs, NULL);
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

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();

    // this is equivalent to FsMkdir(fs, "tmp"); because initially,
    // the current working directory is the root directory
    FsMkdir(fs, "/tmp");

    FsMkdir(fs, "/tmp/tmp.123");
    FsMkdir(fs, "/usr");
    FsMkdir(fs, "/bin");

    // see the section for FsTree for details
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "/tmp");
    FsMkdir(fs, "tmp");
    FsMkdir(fs, "./tmp");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();

    // see the section for FsMkfile for details
    FsMkfile(fs, "hello.txt");

    FsTree(fs, NULL);
    FsMkdir(fs, "hello.txt/world");
    FsMkdir(fs, "html");
    FsMkfile(fs, "html/index.html");
    FsMkdir(fs, "html/index.html/hi");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello/world");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 5 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, ".");
    FsMkdir(fs, "..");
  }
  printf("########### TEST EX 5 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    // this is equivalent to FsMkdir(fs, "hello.c"); because initially,
    // the current working directory is the root directory
    FsMkfile(fs, "/hello.c");

    FsMkfile(fs, "world.c");
    FsMkdir(fs, "/bin");
    FsMkfile(fs, "bin/mkdir");
    FsMkfile(fs, "bin/mkfile");

    // see the section for FsTree for details
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "/tmp");
    FsMkfile(fs, "tmp");
    FsMkfile(fs, "./tmp");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsTree(fs, NULL);
    FsMkfile(fs, "hello/world");
    FsMkdir(fs, "html");
    FsMkfile(fs, "html/index.html");
    FsMkfile(fs, "html/index.html/hi");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello/world");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 5 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, ".");
    FsMkfile(fs, "..");
  }
  printf("########### TEST EX 5 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "/home");
    FsCd(fs, "home");
    FsMkdir(fs, "jas");
    FsCd(fs, "jas");
    FsMkdir(fs, "cs2521");
    FsCd(fs, "cs2521");
    FsMkdir(fs, "lectures");
    FsMkdir(fs, "tutes");
    FsMkdir(fs, "labs");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsCd(fs, "."); // does nothing
    FsCd(fs, "..");
    // does nothing, since the parent of the root directory is itself
    FsCd(fs, "./.././.././"); // also does nothing
    FsMkdir(fs, "tmp");
    FsCd(fs, "tmp");
    FsMkfile(fs, "random.txt");
    FsMkdir(fs, "../bin");
    FsMkdir(fs, "./../home");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "tmp");
    FsCd(fs, "tmp");
    FsCd(fs, NULL);
    FsMkfile(fs, "hello.txt");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsCd(fs, "hello");
    FsCd(fs, "hello/world");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 5 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "tmp");
    FsCd(fs, "bin");
    FsCd(fs, "tmp/dir123");
  }
  printf("########### TEST EX 5 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    printf("---\n"); // marker to separate output
    FsLs(fs, "/");
    printf("---\n");
    FsMkfile(fs, "hello.txt");
    FsMkdir(fs, "tmp");
    FsLs(fs, "/");
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsMkdir(fs, "tmp");
    FsMkfile(fs, "tmp/world.txt");
    FsLs(fs, "hello.txt");
    FsLs(fs, "tmp/world.txt");
    FsLs(fs, "tmp/.././hello.txt");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "tmp");
    FsMkfile(fs, "tmp/hello.txt");
    FsMkfile(fs, "tmp/world.txt");
    FsCd(fs, "tmp");
    FsLs(fs, NULL);
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsLs(fs, "hello/world");
    FsLs(fs, "hello/.");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 5 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "tmp");
    FsLs(fs, "hello");
    FsLs(fs, "tmp/world");
  }
  printf("########### TEST EX 5 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsPwd(fs);
    FsMkdir(fs, "home");
    FsCd(fs, "home");
    FsPwd(fs);
    FsMkdir(fs, "jas");
    FsCd(fs, "jas");
    FsPwd(fs);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsPwd(fs);
    FsMkdir(fs, "home");
    FsCd(fs, "home");
    FsPwd(fs);
    FsMkdir(fs, "jas");
    FsCd(fs, "jas");
    FsPwd(fs);
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsMkfile(fs, "world.txt");
    FsMkdir(fs, "bin");
    FsMkfile(fs, "bin/ls");
    FsMkfile(fs, "bin/pwd");
    FsMkdir(fs, "home");
    FsMkdir(fs, "home/jas");
    FsMkfile(fs, "home/jas/todo.txt");
    FsMkfile(fs, "home/jas/mail.txt");
    FsTree(fs, "/home/jas");
    printf("---\n"); // marker to separate output
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsTree(fs, "hello");
    FsTree(fs, "./hello/world");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "tmp");
    FsTree(fs, "hello");
    FsTree(fs, "tmp/world");
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsPut(fs, "hello.txt", "hello\n");
    FsPut(fs, "./hello.txt", "world\n"); // overwrites existing content
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsPut(fs, "hello", "random-message\n");
    FsPut(fs, ".", "random-message\n");
    FsPut(fs, "/", "random-message\n");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsPut(fs, "hello/world", "random-message\n");
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsPut(fs, "hello/world", "random-message\n");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsPut(fs, "hello.txt", "hello\n");
    FsCat(fs, "hello.txt");
    FsPut(fs, "./hello.txt", "world\n"); // overwrites existing content
    FsCat(fs, "/hello.txt");
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsCat(fs, "hello");
    FsCat(fs, ".");
    FsCat(fs, "/");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsCat(fs, "hello/world");
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsCat(fs, "hello/world");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsMkdir(fs, "hello/world");
    FsTree(fs, NULL);
    printf("---\n"); // marker to separate output
    FsDldir(fs, "hello/world");
    FsDldir(fs, "hello");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsMkdir(fs, "hello/world");
    FsDldir(fs, "hello");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsDldir(fs, "hello/world");
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsDldir(fs, "hello/world");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsMkfile(fs, "hello/world.txt");
    FsMkfile(fs, "abc.txt");
    FsTree(fs, NULL);
    printf("---\n"); // marker to separate output
    FsDl(fs, true, "abc.txt");
    FsTree(fs, NULL);
    printf("---\n"); // marker to separate output
    FsDl(fs, true, "hello");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsDl(fs, false, "hello");
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello");
    FsDl(fs, false, "hello/world");
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsDl(fs, false, "hello/world");
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
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
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### TEST EX 2 START ##########\n");
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
  }
  printf("########### TEST EX 2 DONE ##########\n");

  printf("########### TEST EX 3 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkfile(fs, "hello.txt");
    FsPut(fs, "hello.txt", "hello\n");
    FsMkdir(fs, "world");
    FsMkfile(fs, "world/hello.txt");
    FsTree(fs, NULL);
    printf("---\n");
    FsCat(fs, "world/hello.txt");
    printf("---\n");
    char *src[] = {"hello.txt", NULL};
    FsCp(fs, false, src, "world");
    FsTree(fs, NULL);
    printf("---\n");
    FsCat(fs, "world/hello.txt");
  }
  printf("########### TEST EX 3 DONE ##########\n");

  printf("########### TEST EX 4 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsMkfile(fs, "hello/a.txt");
    FsTree(fs, NULL);
    printf("---\n");
    char *src[] = {"hello", NULL};
    FsCp(fs, true, src, "world");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 4 DONE ##########\n");

  printf("########### TEST EX 5 START ##########\n");
  {
    Fs fs = FsNew();
    FsMkdir(fs, "hello");
    FsMkfile(fs, "hello/a.txt");
    FsMkdir(fs, "world");
    FsTree(fs, NULL);
    printf("---\n");
    char *src[] = {"hello", NULL};
    FsCp(fs, true, src, "world");
    FsTree(fs, NULL);
  }
  printf("########### TEST EX 5 DONE ##########\n");

  printf("########### TEST EX 1 START ##########\n");
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
  }
  printf("########### TEST EX 1 DONE ##########\n");

  printf("########### ALL DONE ##########\n");
}

int main(int argc, char **argv) {
  // 启动测试
  TestExamples();
  // 启动交互程序
  bash(NULL);
  return 0;
}

/***
 * 说明
 * 1. 编译时加入宏 `COLORED` 可以将文件夹显示为蓝色，
 *    加入 `DEBUG` 宏可以使命令报错加上代码位置。
 * 2. 注意复制、移动、删除等情况下不能包括操作父目录，
 *    这一点在文档中说明了不要求实现。
 * 3. 编译方法：使用 CMake 编译，或者将文件直接复制出来编译都行。
 * 4. 标注保持原样的文件都没有动。
 * 5. 没想好说什么，就请你访问 https://space.bilibili.com/672328094 点个关注吧。
 */
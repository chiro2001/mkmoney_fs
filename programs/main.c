//
// Created by Chiro on 2021/10/10.
//

#include "FileType.h"
#include "Fs.h"
#include "utility.h"
#include <stdbool.h>
#include <stdio.h>

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
  FsFree(fs);
  printf("ALL DONE\n");
  return 0;
}

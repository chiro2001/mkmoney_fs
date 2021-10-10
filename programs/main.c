//
// Created by Chiro on 2021/10/10.
//

#include "FileType.h"
#include "Fs.h"
#include "utility.h"
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char **argv) {
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
  FsFree(fs);
}

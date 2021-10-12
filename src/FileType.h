// !!! DO NOT MODIFY THIS FILE !!!

#ifndef FILE_TYPE_H
#define FILE_TYPE_H

typedef enum {
  REGULAR_FILE,
  DIRECTORY,
} FileType;

#include <stdint.h>

typedef enum {
  FS_OK = 0,
  FS_ERROR,
  FS_FILE_EXISTS,
  FS_NO_SUCH_FILE,
  FS_IS_A_DIRECTORY,
  FS_NOT_A_DIRECTORY,
  FS_DIRECTORY_NOT_EMPTY
} FsErrors;

struct FIL_t {
  // 文件类型：文件夹 / 文件
  FileType type;
  // 文件名
  char *name;
  // 文件名长度
  size_t name_length;
  // 当 link != NULL 的时候表示这个文件是某文件的链接
  struct FIL_t *link;
  // 上层文件
  struct FIL_t *parent;
  // 子文件列表
  struct FIL_t **children;
  // 子文件数量大小
  size_t size_children;
  // 文件大小
  size_t size_file;
  // 文件内容
  char *content;
};

typedef struct FIL_t FIL;

struct PATH_t {
  struct FIL_t *file;
  struct PATH_t *forward;
  struct PATH_t *next;
};

typedef struct PATH_t PATH;

// 分隔符
#define FS_SPLIT '/'
#define FS_SPLIT_STR "/"

void listFile(char *name, FileType type);

#endif

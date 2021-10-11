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

#define BLUE "\033[34m"
#define RESET_COLOR "\033[0m"

const char FsErrorMessages[][64] = {"Fs OK",
                                    "Fs Error",
                                    "File exists",
                                    "No such file or directory",
                                    "Is a directory",
                                    "Not a directory",
                                    "Directory not empty"};

#define DEBUG

#ifdef DEBUG
#define PERROR(code, prefix)                                                   \
  printf("[Line:%-4d] " prefix ": %s\n", __LINE__, FsErrorMessages[code]);

#define PERRORD(code, prefix, ...)                                             \
  printf("[Line:%-4d] " prefix ": %s\n", __LINE__, __VA_ARGS__,                \
         FsErrorMessages[code]);
#else
#define PERROR(code, prefix) printf(prefix ": %s\n", FsErrorMessages[code]);

#define PERRORD(code, prefix, ...)                                             \
  printf(prefix ": %s\n", __VA_ARGS__, FsErrorMessages[code]);
#endif

#define FS_MAX_CHILDREN 64

// 储存文件系统相关信息
struct FsRep {
  // 根文件目录
  FIL *root;
  // 当前目录（双向链表尾部）
  PATH *current;
  // 当前路径（双向链表头部）
  PATH *pathRoot;
};

FIL *FsFilFindByName(FIL *dir, const char *name) {
  // TODO: 优化查找算法
  // 先用线性查找
  for (size_t i = 0; i < dir->size; i++) {
    if (strcmp(dir->children[i]->name, name) == 0) {
      return dir->children[i];
    }
  }
  // 找不到文件
  return NULL;
}

/// 按照字典序排序文件夹中的文件排列顺序
/// \param dir
/// \param reverse
void FsFilSort(FIL *dir, int reverse) {
  if (!dir || dir->size == 0 || dir->type == REGULAR_FILE || dir->link)
    return;
  FIL *tmp = NULL;
  // 使用简单的冒泡排序
  for (int i = 0; i < dir->size; i++) {
    if (dir->children[i]->link)
      continue;
    for (int j = i + 1; j < dir->size; j++) {
      if (dir->children[j]->link)
        continue;
      if (0 < (strcmp(dir->children[i]->name, dir->children[j]->name) *
               (reverse ? -1 : 1))) {
        // printf("%s <==> %s\n", dir->children[i]->name,
        // dir->children[j]->name);
        tmp = dir->children[i];
        dir->children[i] = dir->children[j];
        dir->children[j] = tmp;
      }
    }
  }
}

void FsFilPrint(FIL *file) {
  if (!file) {
    printf("[EMPTY FILE]\n");
    return;
  }
  if (file->link) {
    printf("[link] %s -> %s\n", file->name, file->link->name);
    return;
  }
  if (file->type == DIRECTORY) {
    printf("[dir ] %s: ", file->name);
    FsFilSort(file, 0);
    for (int i = 0; i < file->size; i++) {
      printf("%s%s", file->children[i]->name,
             i + 1 == file->size ? "\n" : ", ");
    }
  } else {
    printf("[file] %s\n", file->name);
  }
}

PATH *FsPathGetTail(PATH *path) {
  while (path->next)
    path = path->next;
  return path;
}

void FsPathFree(PATH *path) {
  if (!path)
    return;
  FsPathFree(path->next);
  free(path);
}

/// 在 PATH 链表后插入一个 file
/// \param tail
/// \param file
/// \return 插入后的 path
PATH *FsPathInsert(PATH *tail, FIL *file) {
  assert(tail);
  assert(file);
  tail->next = malloc(sizeof(PATH));
  assert(tail->next);
  memset(tail->next, 0, sizeof(PATH));
  tail->next->file = file;
  tail->next->forward = tail;
  return tail->next;
}

void FsInitFil(FIL *parent, FIL **file, const char *name) {
  if (!name)
    return;
  // 分配储存内存
  *file = (FIL *)malloc(sizeof(FIL));
  assert(*file);
  // 初始化内存
  memset(*file, 0, sizeof(FIL));
  // 分配文件名字内存空间，并且复制名字内容
  // 注意文件名字包含最后结束符\\0，所以多分配一个字节
  (*file)->name_length = strlen(name);
  (*file)->name = (char *)malloc(sizeof(char) * ((*file)->name_length + 1));
  assert((*file)->name);
  strcpy((*file)->name, name);
  (*file)->parent = parent;
  (*file)->type = REGULAR_FILE;
}

/// 新建文件链接
/// \param parent
/// \param name
void FsMkLink(FIL *parent, FIL *link_to, const char *name) {
  FIL *file = NULL;
  FsInitFil(parent, &file, name);
  file->link = link_to;
  file->type = DIRECTORY;
  parent->children[parent->size++] = file;
  if (parent->size == FS_MAX_CHILDREN) {
    PERROR(FS_ERROR, "Children pool full!")
  }
}

/// 初始化文件夹结构
/// \param file
void FsInitDir(FIL *parent, FIL **file, const char *name) {
  FsInitFil(parent, file, name);
  (*file)->type = DIRECTORY;
  // 初始化文件列表空间
  (*file)->children = malloc(sizeof(FIL *) * FS_MAX_CHILDREN);
  assert((*file)->children);
  memset((*file)->children, 0, sizeof(FIL *) * FS_MAX_CHILDREN);
  // 新建两个文件夹：.和..，指向自己或者上层
  FsMkLink(*file, *file, ".");
  FsMkLink(*file, parent, "..");
  // 关于"."和".."文件夹：
  // 1. FileType 为 目录
  // 2. name == "." or ".."
  // 3. size == 0 and children == NULL
  // 4. link == parent or self
}

// 此功能应分配和初始化新的 struct FsRep，创建文件系统的根目录，
// 使根目录成为当前的工作目录。然后，它应返回指
// 向分配的 struct FsRep 的指针。
Fs FsNew(void) {
  // 为文件系统分配内存
  Fs fs = malloc(sizeof(struct FsRep));
  assert(fs);
  memset(fs, 0, sizeof(struct FsRep));
  // 初始化根目录
  // 根目录的 parent 是 NULL
  FsInitDir(NULL, &(fs->root), FS_SPLIT_STR);
  // 把 `/../` -> `/`
  fs->root->children[1]->link = fs->root;
  // 初始化当前访问路径
  // fs->last = malloc(sizeof(PATH));
  // assert(fs->last);
  // memset(fs->last, 0, sizeof(PATH));
  // 指向根目录，forward不用，next暂时留空
  fs->pathRoot = malloc(sizeof(PATH));
  assert(fs->pathRoot);
  memset(fs->pathRoot, 0, sizeof(PATH));
  fs->current = fs->pathRoot;
  fs->current->file = fs->root;
  return fs;
}

/// 复制路径结构
/// \param src
/// \param dst
PATH *FsPathClone(PATH *src) {
  PATH *dst = malloc(sizeof(PATH));
  assert(dst);
  memset(dst, 0, sizeof(PATH));
  PATH *p = dst;
  PATH *s = src;
  p->file = s->file;
  while (s->next) {
    p = FsPathInsert(p, s->next->file);
    s = s->next;
  }
  return dst;
}

char *FsPathGetStr(PATH *path) {
  size_t length = 0;
  PATH *p = path;
  while (p) {
    length += p->file->name_length + 1;
    p = p->next;
  }
  char *pathStr = malloc(sizeof(char) * (length + 1));
  char *str = pathStr;
  p = path;
  while (p) {
    strcpy(str, p->file->name);
    str += p->file->name_length;
    if (strcmp(p->file->name, FS_SPLIT_STR) != 0) {
      *(str++) = FS_SPLIT;
    }
    p = p->next;
  }
  *str = '\0';
  return pathStr;
}

/// 简化路径，`/a/../b` -> `/b`
/// \param path
void FsPathSimplify(PATH **path) {
  // 清理 Path 中的 link 链接类型
  PATH *p = *path;
  if (!p)
    return;
  // Path 最顶层一定不是 link
  while (p && p->next) {
    // printf("path now: ");
    // char *pathStrAbs = FsPathGetStr(*path);
    // puts(pathStrAbs);
    // free(pathStrAbs);
    // printf("p: ");
    // FsFilPrint(p->file);
    // printf("p->next: ");
    // FsFilPrint(p->next->file);
    if (p->next->file->link) {
      // 遇到 link，则开始收缩
      if (p->next->file->link == p->next->file &&
          p->next->file->link != (*path)->file) {
        // "." 路径
        // 直接跳过这个 Node
        PATH *tmp = p->next;
        tmp->next = NULL;
        p->next = p->next->next;
        FsPathFree(tmp);
      } else {
        // ".." 路径
        // 跳过 p、.. 两个 Node
        PATH *tmp = p->forward;
        if (p->next) {
          if (!p->forward) {
            // p: `/`
            // `/..` -> `/`
            FsPathFree(p->next);
            p->next = NULL;
            break;
          } else {
            p->forward->next = p->next->next;
          }
          p->next->next = NULL;
          FsPathFree(p->next);
        } else
          p->forward->next = p->next;
        p->next = NULL;
        FsPathFree(p);
        p = tmp;
      }
    }
    if (!p)
      break;
    p = p->next;
  }
}

FsErrors FsPathParse(PATH *pathRoot, const char *pathStr, PATH **path) {
  if (!path)
    return FS_ERROR;
  const char *p = pathStr;
  if (!pathStr || !*pathStr) {
    // 空串，返回 pwd
    *path = FsPathClone(pathRoot);
    return FS_OK;
  }
  if (*p != FS_SPLIT) {
    // 不以'/'开头，是相对目录
    if (!pathRoot)
      return FS_ERROR;
    // 就先转换为绝对目录
    // 复制路径结构然后简化路径
    *path = FsPathClone(pathRoot);
  } else {
    *path = malloc(sizeof(PATH));
    assert(*path);
    memset(*path, 0, sizeof(PATH));
    (*path)->file = pathRoot->file;
  }
  // 去除开头连续的 `//`
  while (*pathStr && *(pathStr + 1) == FS_SPLIT)
    pathStr++;
  // 现在 path 已经是绝对路径，接下来拼接 pathStr，
  PATH *pathTail = FsPathGetTail(*path);
  // 拼接过程中检查文件是否存在
  char buf[FS_PATH_MAX];
  char *p2 = NULL;
  // 特殊处理 '/' 开头：去掉
  while (*p == FS_SPLIT)
    p++;
  while (*p) {
    // 遇到'/' || 是相对路径开头 || 是绝对路径的'/'
    if (*p == FS_SPLIT || (*pathStr != FS_SPLIT && p == pathStr) ||
        (*pathStr == FS_SPLIT && p == pathStr + 1)) {
      while (*p == FS_SPLIT && *p)
        p++;
      if (!*p)
        return FS_OK;
      // 向下加一层文件夹
      // 找到文件名
      p2 = buf;
      while (*p != FS_SPLIT && *p)
        *(p2++) = *(p++);
      *p2 = '\0';
      // 查找对应文件是否存在
      FIL *target = FsFilFindByName(pathTail->file, buf);
      if (!target) {
        return FS_NO_SUCH_FILE;
      }
      // path 尾部处理：判断有无 '/' 结尾
      if (target->type == REGULAR_FILE) {
        // 找到文件的话，如果 pathStr 后面已经没有东西了
        // 那么就是正确的，否则是错误 FS_NOT_A_DIRECTORY
        if ((*p == FS_SPLIT && *(p + 1) == '\0') || *p == '\0') {
          pathTail = FsPathInsert(pathTail, target);
          FsPathSimplify(path);
          pathTail = FsPathGetTail(*path);
          // printf("After insert [file]: ");
          // char *pathAbs = FsPathGetStr(*path);
          // puts(pathAbs);
          // free(pathAbs);
          return FS_OK;
        } else {
          return FS_NOT_A_DIRECTORY;
        }
      } else {
        // target->type == DIRECTORY
        pathTail = FsPathInsert(pathTail, target);
        FsPathSimplify(path);
        // printf("After insert [dir]: ");
        // char *pathAbs = FsPathGetStr(*path);
        // puts(pathAbs);
        // free(pathAbs);
        pathTail = FsPathGetTail(*path);
      }
    }
    if (!*p)
      break;
    // p++;
  }
  return FS_OK;
}

// 这个函数应该在给定的 cwd 数组中存储当前工作目录的规范路径。
// 它可以假设当前工作目录的规范路径不超过
// PATH_MAX 字符。
/// 得到当前目录的路径
/// \param fs
/// \param cwd
void FsGetCwd(Fs fs, char cwd[FS_PATH_MAX + 1]) {
  // char *p = cwd;
  // PATH *path = fs->pathRoot;
  // char *p2 = NULL;
  // while (path) {
  //   p2 = path->file->name;
  //   while (*p2)
  //     *(p++) = *(p2++);
  //   path = path->next;
  // }
  // *p = '\0';
  char *pathAbs = FsPathGetStr(fs->pathRoot);
  strcpy(cwd, pathAbs);
  free(pathAbs);
}

/// 清理文件树
/// \param file
void FsFilFree(FIL *file) {
  if (!file)
    return;
  if (file->link) {
    // 链接文件不需要删除
    return;
  }
  while (file->size)
    FsFilFree(file->children[--file->size]);
  free(file->children);
  free(file->name);
  free(file);
}

// 这个函数应该释放与给定Fs 关联的所有内存。在处理每个阶段时，
// 您可能需要更新这个函数，以释放您创建的任何新数
// 据结构。
void FsFree(Fs fs) {
  FsFilFree(fs->root);
  free(fs);
}

char *FsPathStrGetName(char *pathStr) {
  size_t length = strlen(pathStr);
  char *p = pathStr + length - 1;
  while (*p && p >= pathStr) {
    if (*p == FS_SPLIT && p != pathStr + length - 1) {
      return p + 1;
    }
    p--;
  }
  // return NULL;
  return pathStr;
}

/// 复制一份 pathStr，并且切断到最后一个 '/' 之前
/// \param pathStr
/// \return
char *FsPathStrShift(char *pathStr) {
  size_t length = strlen(pathStr);
  char *dst = malloc(sizeof(char) * (length + 1));
  strcpy(dst, pathStr);
  // char *p = dst + length - 1;
  // while (*p && p >= dst) {
  //   if (*p == FS_SPLIT && p != dst + length - 1) {
  //     *(p + 1) = '\0';
  //     return dst;
  //   }
  // }
  char *name = FsPathStrGetName(dst);
  if (name) {
    *name = '\0';
  } else {
    // 没找到QAQ
    PERROR(FS_ERROR, "No " FS_SPLIT_STR " found!");
  }
  return dst;
}

// 该函数接受一个路径，并在给定文件系统中的该路径上创建一个新目录。
// FsMkdir 执行的功能与Linux 中的mkdir 命令 大致相同。
// 文件已存在于指定路径
// mkdir: cannot create directory 'path': File exists
// 路径的前缀是一个常规文件
// mkdir: cannot create directory 'path': Not a directory
// 路径的正确前缀不存在
// mkdir: cannot create directory 'path': No such file or
// directory
// 错误消息(包括其余函数中的错误消息)都应该打印到标准输出，这意味着应该使用printf
// 打印它们。还要注意，当出现这些错误之一时，程序不应该退出—函数应该简单地返回
// 文件系统，保持不变。
void FsMkdir(Fs fs, char *pathStr) {
  PATH *path;
  char *pathParentStr = FsPathStrShift(pathStr);
  char *name = FsPathStrGetName(pathStr);
  FsErrors res = FsPathParse(fs->pathRoot, pathParentStr, &path);
  if (res != FS_OK) {
    FsPathFree(path);
    free(pathParentStr);
    PERRORD(res, "mkdir: cannot create directory '%s'", pathStr);
    return;
  }
  PATH *targetPath = FsPathGetTail(path);
  FIL *found = FsFilFindByName(targetPath->file, pathStr);
  if (!found) {
    // 正常情况
    FIL *dirFile = NULL;
    FsInitDir(targetPath->file, &dirFile, name);
    targetPath->file->children[targetPath->file->size++] = dirFile;
    if (targetPath->file->size == FS_MAX_CHILDREN) {
      PERROR(FS_ERROR, "Children pool full!");
    }
    FsFilSort(targetPath->file, 0);
  } else {
    PERRORD(FS_FILE_EXISTS, "mkdir: cannot create directory '%s'", pathStr);
  }
  FsPathFree(path);
  free(pathParentStr);
}

// 该函数接受一个路径，并在给定文件系统中的该路径上创建一个新的空常规文件。
// 这个函数在Linux 中没有直接等效的命令，但最接近的命令是touch，它可以用来创建空
// 的常规文件，但也有其他用途，如更新时间戳。
void FsMkfile(Fs fs, char *path) {
  // TODO
}

// 该函数的路径可能为 NULL。
// 如果路径不为 NULL，函数应该将当前工作目录更改为该路径。
// 如果该路径为 NULL，则默认为 root directory (而不是主目录（home directory），
// 因为在这次任务中我们没有主目录)。 该函数大致相当于 Linux 中的 cd 命令。
// 路径的前缀是一个常规文件 cd: 'path': Not a directory
// 路径的前缀不存在 cd: 'path': No such file or directory
void FsCd(Fs fs, char *pathStr) {
  if (!pathStr || !*pathStr) {
    FsPathFree(fs->pathRoot->next);
    fs->pathRoot->file = fs->root;
    fs->pathRoot->next = NULL;
    fs->current = fs->pathRoot;
    return;
  }
  PATH *path = NULL;
  FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
  if (res) {
    PERRORD(res, "cd: '%s'", pathStr);
  } else {
    FsPathFree(fs->pathRoot);
    fs->pathRoot = FsPathClone(path);
    fs->current = FsPathGetTail(fs->pathRoot);
  }

  FsPathFree(path);
}

// 该函数的路径可能为NULL。
// 如果路径不是NULL 并且指向一个目录，那么函数应该打印该目录中所有文件的名称
// (除了. and . . )，按照ASCII 顺序，
// 每行一个。如果路径指向一个文件，那么该函数应该只打印文件系统中存在的给定路径。
// 如果路径为NULL，则默认为当前工作目录。
// 这个函数大致相当于Linux 中的ls 命令。
// 路径的正确前缀(proper prefix)是
// 一个常规文件
// ls: cannot access 'path': Not a directory
// 路径的前缀不存在 ls: cannot access 'path': No such file or
// directory
void FsLs(Fs fs, char *pathStr) {
  FIL *target = NULL;
  if (!pathStr || (pathStr && !(*pathStr))) {
    target = fs->current->file;
  } else {
    PATH *path = NULL;
    FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
    if (res) {
      FsPathFree(path);
      PERRORD(res, "ls: cannot access '%s'", pathStr);
      return;
    }
    target = FsPathGetTail(path)->file;
    FsPathFree(path);
    if (target->type == REGULAR_FILE) {
      PERRORD(FS_NOT_A_DIRECTORY, "ls: cannot access '%s'", pathStr);
      return;
    }
  }
  for (int i = 0; i < target->size; i++) {
    FIL *f = target->children[i];
    if (f->link)
      continue;
#ifdef COLORED
    printf("%s%s%s", (f->type == REGULAR_FILE ? RESET_COLOR : BLUE), f->name,
           RESET_COLOR);
#else
    printf("%s", f->name);
#endif
    if (f->type == DIRECTORY)
      printf(FS_SPLIT_STR "\n");
    else
      printf("\n");
    return;
  }
}

// 该函数打印当前工作目录的规范路径。
// 该函数大致相当于 Linux 下的 pwd 命令。
void FsPwd(Fs fs) {
  char *pathStrAbs = FsPathGetStr(fs->pathRoot);
  printf("%s\n", pathStrAbs);
  free(pathStrAbs);
}

FsErrors FsTreeInner(FIL *file, int layer) {
  if (!file)
    return FS_OK;
  // printf("Tree: [%d] %s\n", layer, file->name);
  if (file->type == REGULAR_FILE) {
    return FS_NOT_A_DIRECTORY;
  }
  // printf("FsTreeInner: %s\n", file->name);
  // FsFilPrint(file);
  FsFilSort(file, 0);
  for (size_t i = 0; i < file->size; i++) {
    FIL *f = file->children[i];
    if (f->link)
      continue;
    for (int j = 0; j < layer; j++)
      printf("    ");
#ifdef COLORED
    printf("%s%s%s", (f->type == REGULAR_FILE ? RESET_COLOR : BLUE), f->name,
           RESET_COLOR);
#else
    printf("%s", f->name);
#endif
    if (f->type == DIRECTORY)
      puts("/");
    else
      puts("");
    if (f->type == DIRECTORY) {
      FsErrors res = FsTreeInner(f, layer + 1);
      if (res) {
        return res;
      }
    }
  }
  return FS_OK;
}
// 该函数的路径可能为 NULL。
// 如果路径为 NULL，则默认为根目录。
// 该函数以结构化的方式打印给定路径的目录层次结构(见下面)。
// 这个函数大致相当于 Linux 中的 tree 命令。
// 输出的第一行应该包含给定的路径，如果它存在并指向一个目录。下面的行应该
// 按照 ASCII 顺序显示给定目录下的所有
// 文件，每行一个，用缩进显示哪些文件包含在哪些目录下。
// 每一级缩进增加 4 个空格。请参阅用法示例。
// 路径的前缀是一个常规文件 tree: 'path': Not a directory
// 路径的前缀不存在 tree: 'path': No such file or directory
void FsTree(Fs fs, char *pathStr) {
  PATH *path = NULL;
  FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
  if (res) {
    FsPathFree(path);
    PERRORD(res, "tree: '%s'", pathStr);
    return;
  }
  char *pathStrAbs = FsPathGetStr(path);
  PATH *pathTail = FsPathGetTail(path);
  // printf("tree %s:\n", pathStrAbs);
  res = FsTreeInner(pathTail->file, 0);
  if (res) {
    FsPathFree(path);
    PERRORD(res, "tree: '%s'", pathStr);
    return;
  }
  FsPathFree(path);
  free(pathStrAbs);
}

// ========== Task 1 ↑ | ↓ Task 2 ==========

// 该函数接受一个路径和一个字符串，并将该路径上的常规文件的内容设置为
// 给定的字符串。如果文件已经有一些内容，那么它将被覆盖。
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

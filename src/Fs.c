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

// #define DEBUG

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
// 是否在列出文件时在文件夹末尾加上分隔符
// #define FS_SHOW_DIR_SPLIT

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
  for (size_t i = 0; i < dir->size_children; i++) {
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
  if (!dir || dir->size_children == 0 || dir->type == REGULAR_FILE || dir->link)
    return;
  FIL *tmp = NULL;
  // 使用简单的冒泡排序
  for (int i = 0; i < dir->size_children; i++) {
    if (dir->children[i]->link)
      continue;
    for (int j = i + 1; j < dir->size_children; j++) {
      if (dir->children[j]->link)
        continue;
      if ((strcmp(dir->children[i]->name, dir->children[j]->name) *
           (reverse ? -1 : 1)) > 0) {
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
    for (int i = 0; i < file->size_children; i++) {
      printf("%s%s", file->children[i]->name,
             i + 1 == file->size_children ? "\n" : ", ");
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

void FsFilFree(FIL *file) {
  if (!file)
    return;
  if (file->link) {
    // Link 文件只释放本身
    free(file);
    return;
  }
  if (file->type == DIRECTORY) {
    for (int i = 0; i < file->size_children; i++) {
      FsFilFree(file->children[i]);
    }
    free(file->children);
  } else {
    if (file->content)
      free(file->content);
  }
  free(file->name);
  free(file);
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

void FsFilInit(FIL *parent, FIL **file, const char *name) {
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
  FsFilInit(parent, &file, name);
  file->link = link_to;
  file->type = DIRECTORY;
  parent->children[parent->size_children++] = file;
  if (parent->size_children == FS_MAX_CHILDREN) {
    PERROR(FS_ERROR, "Children pool full!")
  }
}

/// 初始化文件夹结构
/// \param file
void FsInitDir(FIL *parent, FIL **file, const char *name) {
  FsFilInit(parent, file, name);
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
  // 3. size_children == 0 and children == NULL
  // 4. link == parent or self
}

/// 初始化文件结构
/// \param parent
/// \param file
/// \param name
void FsInitFile(FIL *parent, FIL **file, const char *name) {
  FsFilInit(parent, file, name);
  (*file)->type = REGULAR_FILE;
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
  // 指向根目录
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
    if (p->next->file->link) {
      // 遇到 link，则开始收缩
      if (p->next->file->link == p->file &&
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
  while (*pathStr == FS_SPLIT && *(pathStr + 1) == FS_SPLIT)
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
          return FS_OK;
        } else {
          return FS_NOT_A_DIRECTORY;
        }
      } else {
        // target->type == DIRECTORY
        pathTail = FsPathInsert(pathTail, target);
        FsPathSimplify(path);
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
  char *pathAbs = FsPathGetStr(fs->pathRoot);
  strcpy(cwd, pathAbs);
  free(pathAbs);
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
  char *newName = malloc(sizeof(char) * (length + 1));
  strcpy(newName, pathStr);
  char *p = newName + length - 1;
  while (*p == '/' && *p) {
    *p = '\0';
    p--;
  }
  while (*p && p >= newName) {
    if (*p == FS_SPLIT && p != newName + length - 1) {
      size_t length2 = strlen(p + 1);
      char *newNewName = malloc(sizeof(char) * (length2 + 1));
      strcpy(newNewName, p + 1);
      free(newName);
      return newNewName;
    }
    p--;
  }
  if (!*p) {
    // return p;
    // return "";
    *p = '\0';
  }
  return newName;
}

/// 复制一份 pathStr，并且切断到最后一个 '/' 之前
/// \param pathStr
/// \return
char *FsPathStrShift(char *pathStr) {
  size_t length = strlen(pathStr);
  char *dst = malloc(sizeof(char) * (length + 1));
  strcpy(dst, pathStr);
  // // char *p = dst + length - 1;
  // // while (*p && p >= dst) {
  // //   if (*p == FS_SPLIT && p != dst + length - 1) {
  // //     *(p + 1) = '\0';
  // //     return dst;
  // //   }
  // // }
  // char *name = FsPathStrGetName(dst);
  // char *name = FsPathStrGetName(pathStr);

  char *p = dst + length - 1;
  while (*p && p > dst) {
    if (*p == FS_SPLIT && p != dst + length - 1) {
      *(p + 1) = '\0';
      return dst;
    }
    p--;
  }
  if (p == dst)
    *p = '\0';
  // if (name) {
  //   *name = '\0';
  // } else {
  //   // 没找到QAQ
  //   PERROR(FS_ERROR, "No " FS_SPLIT_STR " found!");
  // }
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
  PATH *path = NULL;
  char *pathParentStr = FsPathStrShift(pathStr);
  char *name = FsPathStrGetName(pathStr);
  FsErrors res = FsPathParse(fs->pathRoot, pathParentStr, &path);
  if (res != FS_OK) {
    free(name);
    FsPathFree(path);
    free(pathParentStr);
    PERRORD(res, "mkdir: cannot create directory '%s'", pathStr);
    return;
  }
  PATH *targetPath = FsPathGetTail(path);
  if (targetPath->file->type == REGULAR_FILE) {
    free(name);
    FsPathFree(path);
    free(pathParentStr);
    PERRORD(FS_NOT_A_DIRECTORY, "mkfile: cannot create file '%s'", pathStr);
    return;
  }
  PATH *findingPath = NULL;
  FsErrors resFinding = FsPathParse(fs->pathRoot, pathStr, &findingPath);
  if (resFinding == FS_OK) {
    // 找到了文件，错误。
    PERRORD(FS_FILE_EXISTS, "mkdir: cannot create directory '%s'", pathStr);
  } else {
    // 正常情况
    FIL *dirFile = NULL;
    FsInitDir(targetPath->file, &dirFile, name);
    targetPath->file->children[targetPath->file->size_children++] = dirFile;
    if (targetPath->file->size_children == FS_MAX_CHILDREN) {
      PERROR(FS_ERROR, "Children pool full!");
    }
    FsFilSort(targetPath->file, 0);
  }
  FsPathFree(findingPath);
  FsPathFree(path);
  free(name);
  free(pathParentStr);
}

// 该函数接受一个路径，并在给定文件系统中的该路径上创建一个新的空常规文件。
// 这个函数在Linux 中没有直接等效的命令，但最接近的命令是touch，它可以用来创建空
// 的常规文件，但也有其他用途，如更新时间戳。
void FsMkfile(Fs fs, char *pathStr) {
  PATH *path = NULL;
  char *pathParentStr = FsPathStrShift(pathStr);
  char *name = FsPathStrGetName(pathStr);
  FsErrors res = FsPathParse(fs->pathRoot, pathParentStr, &path);
  if (res != FS_OK) {
    free(name);
    FsPathFree(path);
    free(pathParentStr);
    PERRORD(res, "mkfile: cannot create file '%s'", pathStr);
    return;
  }
  PATH *targetPath = FsPathGetTail(path);
  if (targetPath->file->type == REGULAR_FILE) {
    FsPathFree(path);
    free(name);
    free(pathParentStr);
    PERRORD(FS_NOT_A_DIRECTORY, "mkfile: cannot create file '%s'", pathStr);
    return;
  }
  PATH *findingPath = NULL;
  FsErrors resFinding = FsPathParse(fs->pathRoot, pathStr, &findingPath);
  if (resFinding == FS_OK) {
    // 找到了文件，错误。
    PERRORD(FS_FILE_EXISTS, "mkfile: cannot create directory '%s'", pathStr);
  } else {
    // 正常情况
    FIL *file = NULL;
    FsInitFile(targetPath->file, &file, name);
    targetPath->file->children[targetPath->file->size_children++] = file;
    if (targetPath->file->size_children == FS_MAX_CHILDREN) {
      PERROR(FS_ERROR, "Children pool full!");
    }
    FsFilSort(targetPath->file, 0);
  }
  FsPathFree(path);
  free(pathParentStr);
  free(name);
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
    PATH *pathTail = FsPathGetTail(path);
    if (pathTail->file->type == REGULAR_FILE) {
      PERRORD(FS_NOT_A_DIRECTORY, "cd: '%s'", pathStr);
    } else {
      FsPathFree(fs->pathRoot);
      fs->pathRoot = FsPathClone(path);
      fs->current = FsPathGetTail(fs->pathRoot);
    }
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
  if (!pathStr || !*pathStr) {
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
  for (int i = 0; i < target->size_children; i++) {
    FIL *f = target->children[i];
    if (f->link)
      continue;
#ifdef COLORED
    printf("%s%s%s", (f->type == REGULAR_FILE ? RESET_COLOR : BLUE), f->name,
           RESET_COLOR);
#else
    printf("%s", f->name);
#endif
#ifdef FS_SHOW_DIR_SPLIT
    if (f->type == DIRECTORY)
      printf(FS_SPLIT_STR "\n");
    else
      printf("\n");
#else
    puts("");
#endif
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
  if (layer == 0)
    printf("%s\n", file->name);
  for (size_t i = 0; i < file->size_children; i++) {
    FIL *f = file->children[i];
    if (f->link)
      continue;
    for (int j = 0; j < layer + 1; j++)
      printf("    ");
#ifdef COLORED
    printf("%s%s%s", (f->type == REGULAR_FILE ? RESET_COLOR : BLUE), f->name,
           RESET_COLOR);
#else
    printf("%s", f->name);
#endif
#ifdef FS_SHOW_DIR_SPLIT
    if (f->type == DIRECTORY)
      puts(FS_SPLIT_STR);
    else
      puts("");
#else
    puts("");
#endif
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
void FsPut(Fs fs, char *pathStr, char *content) {
  PATH *path = NULL;
  FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
  if (res) {
    PERRORD(res, "put: '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  PATH *pathTail = FsPathGetTail(path);
  if (pathTail->file->type != REGULAR_FILE) {
    PERRORD(FS_IS_A_DIRECTORY, "put: '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  FIL *target = pathTail->file;
  if (target->size_file) {
    if (target->content)
      free(target->content);
    target->content = NULL;
    target->size_file = 0;
  }
  size_t length = strlen(content) + 1;
  target->content = malloc(sizeof(char) * length);
  memcpy(target->content, content, length);
  target->size_file = length;
  FsPathFree(path);
}

// 该函数接受一个路径，并在该路径上打印常规文件的内容。
// 这个函数大致相当于Linux 中的cat 命令。
void FsCat(Fs fs, char *pathStr) {
  PATH *path = NULL;
  FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
  if (res) {
    PERRORD(res, "put: '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  PATH *pathTail = FsPathGetTail(path);
  if (pathTail->file->type != REGULAR_FILE) {
    PERRORD(FS_IS_A_DIRECTORY, "put: '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  FIL *target = pathTail->file;
  if (target->size_file) {
    printf("%s", target->content);
  }
  FsPathFree(path);
}

void FsFilDlTree(FIL *file) {
  FIL *parent = file->parent;
  int found = -1;
  for (int i = 0; i < parent->size_children; i++) {
    FIL *f = parent->children[i];
    if (f == file) {
      found = i;
      break;
    }
  }
  if (found < 0) {
    PERROR(FS_ERROR, "Internal Error!");
    exit(1);
  }
  FsFilFree(file);
  if (parent->size_children == 1) {
    parent->size_children = 0;
  } else {
    parent->children[found] = parent->children[parent->size_children - 1];
    parent->size_children--;
    FsFilSort(parent, 0);
  }
}

// 该函数接受一个指向目录的路径，当且仅当该路径为空时删除该目录。
// 这个函数大致相当于Linux 中的rmdir 命令。
// 为简单起见，可以假设给定路径不包含当前工作目录。
// 注意，这意味着给定的路径永远不会是根目录。如果您愿意(为了
// 完整性起见)，您可以处理这种情况，但是不会对它进行测试。
void FsDldir(Fs fs, char *pathStr) {
  PATH *path = NULL;
  FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
  if (res) {
    PERRORD(res, "dldir: failed to remove '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  PATH *pathTail = FsPathGetTail(path);
  if (pathTail->file->type != DIRECTORY) {
    PERRORD(FS_NOT_A_DIRECTORY, "dldir: failed to remove '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  FIL *dirFile = pathTail->file;
  if (dirFile->size_children > 2) {
    PERRORD(FS_DIRECTORY_NOT_EMPTY, "dldir: failed to remove '%s'", pathStr);
  } else {
    // TODO: check root
    FsFilDlTree(dirFile);
  }
  FsPathFree(path);
}

// 该功能采取路径并删除该路径上的文件。
// 默认情况下，该功能拒绝删除目录：它只会删除目录（及其所有内容递归），如
// 果递归是真实的。如果路径指常规文件，则递归参数无关紧要。
// 此函数大致对应于 Linux 中的 rm 命令，递归真实性与 rm 命令中使用的 -r
// 选项相对应。
void FsDl(Fs fs, bool recursive, char *pathStr) {
  PATH *path = NULL;
  FsErrors res = FsPathParse(fs->pathRoot, pathStr, &path);
  if (res) {
    PERRORD(res, "dl: cannot remove '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  PATH *pathTail = FsPathGetTail(path);
  FIL *target = pathTail->file;
  if (target->type == DIRECTORY && !recursive) {
    PERRORD(FS_IS_A_DIRECTORY, "dl: failed to remove '%s'", pathStr);
    FsPathFree(path);
    return;
  }
  if (target->type == REGULAR_FILE) {
    FsFilDlTree(target);
  } else {
    if (!recursive && target->size_children > 2) {
      PERRORD(FS_DIRECTORY_NOT_EMPTY, "dl: failed to remove '%s'", pathStr)
    } else {
      FsFilDlTree(target);
    }
  }
  FsPathFree(path);
}

FsErrors FsFilCopy(FIL *src, FIL *dst) {
  if (!src || !dst)
    return FS_ERROR;
  if (src->link)
    return FS_OK;
  if (dst->type != DIRECTORY) {
    return FS_NOT_A_DIRECTORY;
  }
  // 检查是否有同名文件
  FIL *found = FsFilFindByName(dst, src->name);
  if (found) {
    return FS_FILE_EXISTS;
  }
  FIL *data = NULL;
  if (src->type == DIRECTORY) {
    FsInitDir(dst, &data, src->name);
  } else {
    FsInitFile(dst, &data, src->name);
  }
  dst->children[dst->size_children++] = data;
  // 整理顺序
  FsFilSort(dst, 0);
  // 默认递归复制
  if (src->type == DIRECTORY) {
    for (int i = 0; i < src->size_children; i++) {
      FsFilCopy(src->children[i], data);
    }
  } else {
    data->content = malloc(sizeof(char) * src->size_file);
    memcpy(data->content, src->content, src->size_file);
  }
  return FS_OK;
}

FsErrors FsFilMove(FIL *src, FIL *dst) {
  if (!src || !dst)
    return FS_ERROR;
  if (dst->type != DIRECTORY)
    return FS_NOT_A_DIRECTORY;
  int parentIndex = -1;
  for (int i = 0; i < src->parent->size_children; i++) {
    if (src->parent->children[i] == src) {
      parentIndex = i;
      break;
    }
  }
  if (parentIndex < 0)
    return FS_ERROR;
  if (src->parent->size_children == 1) {
    src->parent->size_children = 0;
  } else {
    src->parent->children[parentIndex] =
        src->parent->children[--src->parent->size_children];
    FsFilSort(src->parent, 0);
  }
  src->parent = dst;
  dst->children[dst->size_children++] = src;
  FsFilSort(dst, 0);
  return FS_OK;
}

// 该函数接受一个以NULL 结尾的路径数组src 和路径dest。
// 如果src 数组恰好包含一个路径，那么它应该将位于src 的 文件复制到dest。
// 如果src 数组包含多个路径，那么dest 应该指向一个目录，
// 函数应该将src 数组中所有路径下的文 件复制到dest 目录下。
// 默认情况下，函数不复制目录-只有当递归为true 时，它才应该复制目录。
// 这个函数大致相当于Linux 中的cp 命令。
void FsCp(Fs fs, bool recursive, char *src[], char *dest) {
  // TODO: 检查路径包含
  char **pathStrPointer = src;
  if (!*pathStrPointer) {
    PERROR(FS_NO_SUCH_FILE, "cp");
    return;
  }
  PATH *pathDst = NULL;
  FsErrors resDst = FsPathParse(fs->pathRoot, dest, &pathDst);
  if (resDst) {
    if (resDst == FS_NO_SUCH_FILE) {
      // 找不到 Dist 则新建这个文件
      // 取 dst 的上层parent
      PATH *dstPathParent = NULL;
      char *pathParentStr = FsPathStrShift(dest);
      FsErrors res = FsPathParse(fs->pathRoot, pathParentStr, &dstPathParent);
      if (res != FS_OK) {
        PERRORD(res, "cp: '%s'", dest);
      } else {
        PATH *pathParent = NULL;
        res = FsPathParse(fs->pathRoot, *pathStrPointer, &pathParent);
        if (res) {
          PERRORD(res, "cp: '%s'", *pathStrPointer);
        } else {
          PATH *dstPathParentTail = FsPathGetTail(dstPathParent);
          PATH *pathParentTail = FsPathGetTail(pathParent);
          if (pathParentTail->file->type == DIRECTORY && !recursive) {
            PERRORD(FS_IS_A_DIRECTORY, "cp: '%s'", *pathStrPointer);
          } else {
            if (pathParentTail->file->type == DIRECTORY) {
              FsFilCopy(pathParentTail->file, dstPathParentTail->file);
            } else {
              FIL *newFile = NULL;
              char *name = FsPathStrGetName(dest);
              FsInitFile(dstPathParentTail->file, &newFile, name);
              free(name);
              FsFilCopy(newFile, dstPathParentTail->file);
              FsFilFree(newFile);
            }
          }
        }
        FsPathFree(pathParent);
      }
      free(pathParentStr);

    } else {
      PERRORD(resDst, "cp: '%s'", dest);
    }
    FsPathFree(pathDst);
    return;
  }
  PATH *pathDstTail = FsPathGetTail(pathDst);
  if (pathDstTail->file->type != DIRECTORY) {
    // 目标是个文件，则覆盖这个文件
    // 先删除之
    FIL *dstParent = pathDstTail->file->parent;
    // 只取最上面的文件
    PATH *pathParent = NULL;
    FsErrors res = FsPathParse(fs->pathRoot, *pathStrPointer, &pathParent);
    if (res) {
      PERRORD(res, "cp: '%s'", *pathStrPointer);
      FsPathFree(pathParent);
      return;
    }
    char *nameOld = malloc(sizeof(char) * (pathDstTail->file->name_length + 1));
    strcpy(nameOld, pathDstTail->file->name);
    // printf("To del: (%s) ", nameOld);
    // FsFilPrint(pathDstTail->file);
    FsFilDlTree(pathDstTail->file);

    PATH *pathParentTail = FsPathGetTail(pathParent);
    if (pathParentTail->file->link) {
      PERRORD(FS_NO_SUCH_FILE, "cp: '%s'", *pathStrPointer);
    } else {
      // 复制到内存
      FIL *newFile = NULL;
      FsInitFile(pathParentTail->file->parent, &newFile, nameOld);
      newFile->size_file = pathParentTail->file->size_file;
      newFile->content = malloc(sizeof(char) * newFile->size_file);
      memcpy(newFile->content, pathParentTail->file->content,
             sizeof(char) * newFile->size_file);
      // 复制该文件
      if (pathParentTail->file->link) {
        PERRORD(FS_NO_SUCH_FILE, "cp: '%s'", *pathStrPointer);
      } else {
        // printf("COPY (file->file): (%s->%s)\n", newFile->name, dstParent->name);
        // FsFilPrint(newFile);
        // FsFilPrint(dstParent);
        res = FsFilCopy(newFile, dstParent);
        if (res) {
          PERRORD(res, "cp: '%s'", *pathStrPointer);
        }
      }
      FsFilFree(newFile);
    }
    free(nameOld);
    FsPathFree(pathParent);
  } else {
    // 目标是一个路径
    // 源文件仅包含一个路径
    if (*pathStrPointer && !*(pathStrPointer + 1)) {
      PATH *pathParent = NULL;
      FsErrors res = FsPathParse(fs->pathRoot, *pathStrPointer, &pathParent);
      if (res) {
        PERRORD(res, "cp: '%s'", *pathStrPointer);
        FsPathFree(pathParent);
        return;
      }
      PATH *pathParentTail = FsPathGetTail(pathParent);
      if (pathParentTail->file->link) {
        PERRORD(FS_NO_SUCH_FILE, "cp: '%s'", *pathStrPointer);
      } else {
        if (pathParentTail->file->type == REGULAR_FILE) {
          // 仅仅复制该文件
          if (pathParentTail->file->link) {
            PERRORD(FS_NO_SUCH_FILE, "cp: '%s'", *pathStrPointer);
          } else {
            res = FsFilCopy(pathParentTail->file, pathDstTail->file);
            if (res) {
              PERRORD(res, "cp: '%s'", *pathStrPointer);
            }
          }
        } else {
          // 复制该路径下的所有文件
          for (int i = 0; i < pathParentTail->file->size_children; i++) {
            FIL *f = pathParentTail->file->children[i];
            // 不recursive 的时候不复制目录
            if (!recursive && f->type == DIRECTORY) {
              PERRORD(FS_IS_A_DIRECTORY, "cp: '%s'", f->name);
              continue;
            }
            res = FsFilCopy(f, pathDstTail->file);
            if (res) {
              PERRORD(res, "cp: '%s'", f->name);
            }
          }
        }
      }
      FsPathFree(pathParent);
    } else {
      // 包含多个路径，则复制这些路径的文件
      while (*pathStrPointer) {
        PATH *path = NULL;
        FsErrors res = FsPathParse(fs->pathRoot, *pathStrPointer, &path);
        if (res) {
          PERRORD(res, "cp: '%s'", *pathStrPointer);
          FsPathFree(path);
          continue;
        }
        PATH *pathTargetTail = FsPathGetTail(path);
        // 不recursive 的时候不复制目录
        if (!recursive && pathTargetTail->file->type == DIRECTORY) {
          PERRORD(FS_IS_A_DIRECTORY, "cp: '%s'", *pathStrPointer);
        } else {
          FsFilCopy(pathTargetTail->file, pathDstTail->file);
        }
        FsPathFree(path);
        pathStrPointer++;
      }
    }
  }

  FsPathFree(pathDst);
}

void FsPrint(Fs fs, char *pathStr) {
  PATH *path = NULL;
  // FsErrors res =
  FsPathParse(fs->pathRoot, pathStr, &path);
  FsFilPrint(FsPathGetTail(path)->file);
  FsPathFree(path);
}

// 该函数接受以null 结尾的src 路径数组和dest 路径。
// 它应该将src 中所有路径所指向的文件移动到dest。
// 该函数大致相当于Linux 中的mv 命令。
void FsMv(Fs fs, char *src[], char *dest) {
  // TODO: 检查路径包含
  char **pathStrPointer = src;
  if (!*pathStrPointer) {
    PERROR(FS_NO_SUCH_FILE, "mv");
    return;
  }
  PATH *pathDst = NULL;
  FsErrors resDst = FsPathParse(fs->pathRoot, dest, &pathDst);
  if (resDst) {
    if (resDst == FS_NO_SUCH_FILE) {
      // 找不到 dist 则新建文件
      // 取 dst 的上层parent
      PATH *dstPathParent = NULL;
      char *pathParentStr = FsPathStrShift(dest);
      FsErrors res = FsPathParse(fs->pathRoot, pathParentStr, &dstPathParent);
      if (res != FS_OK) {
        PERRORD(res, "mv: '%s'", dest);
      } else {
        PATH *pathParent = NULL;
        res = FsPathParse(fs->pathRoot, *pathStrPointer, &pathParent);
        if (res) {
          PERRORD(res, "mv: '%s'", *pathStrPointer);
        } else {
          // 改名字然后移动
          PATH *dstPathParentTail = FsPathGetTail(dstPathParent);
          PATH *pathParentTail = FsPathGetTail(pathParent);
          free(pathParentTail->file->name);
          char *name = FsPathStrGetName(dest);
          size_t length = strlen(name);
          pathParentTail->file->name = malloc(sizeof(char) * (length + 1));
          strcpy(pathParentTail->file->name, name);
          pathParentTail->file->name_length = length;
          free(name);
          res = FsFilMove(pathParentTail->file, dstPathParentTail->file);
          if (res) {
            PERRORD(res, "mv: '%s'", dest);
          }
        }
        FsPathFree(pathParent);
      }
      free(pathParentStr);
    } else {
      PERRORD(resDst, "mv: '%s'", dest);
    }
    FsPathFree(pathDst);
    return;
  }
  PATH *pathDstTail = FsPathGetTail(pathDst);
  if (pathDstTail->file->type != DIRECTORY) {
    // 目标是个文件，则覆盖这个文件
    // 先删除之
    FIL *dstParent = pathDstTail->file->parent;
    // 只取最上面的文件
    PATH *pathParent = NULL;
    FsErrors res = FsPathParse(fs->pathRoot, *pathStrPointer, &pathParent);
    if (res) {
      PERRORD(res, "mv: '%s'", *pathStrPointer);
      FsPathFree(pathParent);
      return;
    }
    FsFilDlTree(pathDstTail->file);

    PATH *pathParentTail = FsPathGetTail(pathParent);
    // 移动到目标处
    FsFilMove(pathParentTail->file, dstParent);
    FsPathFree(pathParent);
  } else {
    // 移动这些路径的文件
    while (*pathStrPointer) {
      PATH *path = NULL;
      FsErrors res = FsPathParse(fs->pathRoot, *pathStrPointer, &path);
      if (res) {
        PERRORD(res, "mv: '%s'", *pathStrPointer);
        FsPathFree(path);
        continue;
      }
      PATH *pathTargetTail = FsPathGetTail(path);
      FsFilMove(pathTargetTail->file, pathDstTail->file);
      FsPathFree(path);
      pathStrPointer++;
    }
  }
  FsPathFree(pathDst);
}

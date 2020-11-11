#include "user.h"
#include "/home/180110123/xv6-riscv-fall19/kernel/types.h"
#include "/home/180110123/xv6-riscv-fall19/kernel/stat.h"
#include "/home/180110123/xv6-riscv-fall19/kernel/fs.h"

void find(char *path, char *re) 
{
  char buf[512], *p;
  int fd;
  struct dirent de;   // 为获取文件目录内容
  struct stat st;

  // open the dir
  if((fd = open(path, 0)) < 0){
  fprintf(2, "文件打开失败%s\n", path);
  return;
  }
  // fstat：将fd所指向的文件状态复制到参数st所指向的结构中
  if(fstat(fd, &st) < 0 || T_DIR != st.type){
    fprintf(2, "第一个参数必须是dir path\n");
    close(fd);
    return;
  }
  // descent into sub-dir
  while(read(fd, &de, sizeof(de)) == sizeof(de)) {
    // splice current path
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    if(de.inum == 0) {
        continue;
    }
    // 将de中文件名赋值到buf，进行递归
    memmove(p, de.name, DIRSIZ);    
    p[DIRSIZ] = 0;
    // 提供文件名字，获取文件对应属性。
    if(stat(buf, &st) < 0){       
      printf("find: cannot stat %s\n", buf);
      continue;
    }
    switch(st.type) {
      case T_FILE:
        // 找当相同文件名路径
        if (strcmp(re, de.name) == 0) {
        printf("%s\n", buf);
        }
        break;
      case T_DIR:
      // 不能递归进入'.','..'
        if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
            find(buf, re);
        }
        break;
    }
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "参数太少\n");
    exit();
  }
  find(argv[1], argv[2]);
  exit();
}
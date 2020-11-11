#include "/home/180110123/xv6-riscv-fall19/kernel/types.h"
#include "/home/180110123/xv6-riscv-fall19/kernel/param.h"
#include "user.h"


int main(int argc, char *argv[])
{
    if(argc < 2) {
        fprintf(2, "No less than 2 parameters\n"); 
        exit();
    }

    char buf_argv[MAXARG][MAXARG]; 
    char buf;   // 读文件0
    char *p[MAXARG];
    int i,j;
    int blank = 0;  // 1:行中第一列为空格   


    while (1)
    {
        memset(buf_argv,0,MAXARG * MAXARG);   // 初始化

        i = 0;
        blank = 0;
        for(j = 1; j <argc ; j++) {
          strcpy(buf_argv[i++],argv[j]);    // 赋值buf_argv
        }
        j = 0;
        while (i < MAXARG-1) 
        {
            if (read(0, &buf, 1) <= 0) {    // 读取文件stdin
            // CTRL+D = EOF
                wait(); 
                exit();
            }
            if (buf == '\n') {  // 新一行
                break;
            }
            if(buf == ' ') {    // 空格
                if(blank) {   // exam: hello ' ',进入新一列
                    i = i+1;  // row
                    j = 0;    // column
                    blank = 0;
                }
                continue;
            }
            buf_argv[i][j++] = buf;
            blank = 1;
        }
        for(int k = 0;k < MAXARG-1; k++) {
            p[k] = buf_argv[k];
        }

        p[MAXARG-1] = 0;

        if(!fork()) {
          exec(argv[1], p);
          exit();
        }
    }
    exit();
}
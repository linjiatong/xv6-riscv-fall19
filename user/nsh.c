
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 10
#define MAXWORD 30
#define MAXLINE 100
 
void execPipe(char*argv[],int argc);
char whitespace[] = " \t\r\n\v";

int getcmd(char *buf, int nbuf)
{
    fprintf(2, "@ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}


void setargs(char *cmd, char* argv[],int* argc)
{
    int i = 0; // 表示第i个word
    int j = 0;
    // 让argv[i]指向每个命令中的一个word，并在word后面的空格设为\0
    for (; cmd[j] != '\n' && cmd[j] != '\0'; j++)
    {
        // 跳过前面的空格
        while (strchr(whitespace,cmd[j])){
            j++;
        }
        argv[i++]=cmd+j;
        // 找到下一个空格
        while (!strchr(whitespace,cmd[j])){
            j++;
        }
        cmd[j]='\0';
    }
    argv[i]=0;
    *argc=i;
}
 

void runcmd(char*argv[],int argc)
{
    for(int i = 1 ; i < argc ; i++){
        if(!strcmp(argv[i],"|")){
            execPipe(argv,argc);
        }
    }
    // 判断是否需要重定向
    for(int i = 1 ; i < argc ; i++){
        // 遇到 > ，说明需要执行输出重定向，首先需要关闭stdout
        if(!strcmp(argv[i],">")){
            close(1);
            // O_CREAT 若欲打开的文件不存在则自动建立该文件
            // O_WRONLY 以只写方式打开文件
            open(argv[i+1],O_CREATE|O_WRONLY); 
            argv[i]=0;
        }
        if(!strcmp(argv[i],"<")){
            // 遇到< ,需要执行输入重定向，关闭stdin
            close(0);
            // O_RDONLY 以只读方式打开文件
            open(argv[i+1],O_RDONLY);       // ox000
            argv[i]=0;
        }
    }
    exec(argv[0],argv);
}
 
void execPipe(char*argv[],int argc){
    int i=0;
    // 找到命令中的"|",替换成'\0'
    for(;i < argc ; i++){
        if(!strcmp(argv[i],"|")){
            argv[i]=0;
            break;
        }
    }
    int fd[2];
    pipe(fd);
    if(fork() == 0){
        // 执行左边的命令 把自己的标准输出关闭
        close(1);
        dup(fd[1]);
        close(fd[0]);
        close(fd[1]);
        runcmd(argv,i);
    }else{
        // 执行右边的命令 把自己的标准输入关闭
        close(0);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        runcmd(argv+i+1,argc-i-1);
    }
}

int main()
{
    char buf[MAXLINE];
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
        if (fork() == 0)
        {
            char* argv[MAXARGS];
            int argc=-1;
            setargs(buf, argv,&argc);
            runcmd(argv,argc);
        }
        wait(0);
    }
    exit(0);
}
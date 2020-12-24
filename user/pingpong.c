#include "user.h"


/*
    父进程将“ping”写入parent_pipe，发送给子进程
    子进程接受后，将“pong”写入child_pipe，发送给父进程
*/
int main(int argc, char const *argv[])
{
    int fd[2];      // f[1]写入端，f[0]:读出端 ， parent_pipe
    int fd_second[2];   // chile_pipe
    int k = pipe(fd);
    if(k < 0) {
        exit();
    }
    pipe(fd_second);
    //close(fd[0]);
    char buf[15];
    char pa_buf[15];
    if(!fork()) {
        // 子进程开始
        close(fd[1]);       // 关闭写端
        read(fd[0],buf,sizeof(buf));
        printf("%d: received %s",getpid(),buf);
        write(fd_second[1],"pong\n",5);
        exit();
    }
    else
    {
        // 父进程开始
        write(fd[1],"ping\n",5);
        wait();
        close(fd_second[1]);   
        read(fd_second[0],pa_buf,sizeof(pa_buf));
        printf("%d: received %s",getpid(),pa_buf);
        exit();
    }
    
    return 0;
    
}

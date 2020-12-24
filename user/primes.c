#include "user.h"

// 质数筛选，筛出2~35中所有质数
int main(int argc, char const *argv[]) {
    int number[34];     // number[0]表示数字2的标识符
    int len = 0;

    while (len < 33)
    {
        //int parent_fd[2];    // 父传子
        //pipe(parent_fd);
        int child_fd[2];    // 子传父
        pipe(child_fd);

        if(!fork()) {   // 子进程
            printf("prime %d\n",len+2);
            for(int i = len+1 ; i < 34 ; i++) {
                if((i+2) % (len+2) == 0) {    // 能被整除
                    number[i] = 1;     // 1:不是质数
                }  
            }
            write(child_fd[1],number,sizeof(number));
            exit();
        }
        else {
            wait();
            close(child_fd[1]);
            read(child_fd[0],number,sizeof(number));
            len = len+1;        // 找到下一个质数
            while (number[len] == 1 && len < 33)
            {
                len = len + 1;
            }
        }

    }
    exit();
    return 0;
}
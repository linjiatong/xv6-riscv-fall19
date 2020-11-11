// #include "user.h"

// void screen(int num[],int n) {
//     int prime = num[0];
//     printf("prime %d\n",prime);
//     if(n == 1) exit();
//     int p[2];
//     pipe(p);

//     int i;
//     for(i = 0;i <n;i++) {
//         write(p[1],&num[i],1);
//     }

//     close(p[1]);
//     if(fork() == 0) {
//         int buf[1];
//         i = 0;
//         while (read(p[0],buf,1) != 0)
//         {
//             if(buf[0] % prime != 0) {
//                 num[i] = buf[0];
//                 i++;
//             }
//         }
//         screen(num,i);
//         exit();
//     }
//     wait();
// }



// int main(){
//     int num[34];
//     for(int i = 0;i <= 33 ;i++) {
//         num[i] = i+2;
//     }
//     screen(num,34);
//     exit();
// }


#include "user.h"


int main(int argc, char const *argv[]) {
    int number[34];
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
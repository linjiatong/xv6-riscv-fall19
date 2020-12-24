#include "user.h"

int main(int argc, char const *argv[])
{
    /* code */
    if(argc == 2) {
        int time = atoi(argv[1]);   // 字符串转整数
        sleep(time);
        exit();
    }
    else{
        sleep(1.6);
        exit();
    }
    return 0;
}

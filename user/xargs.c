#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#define MAXLENGTH 32
int main(int argc, char *argv[])
{   
    //存储命令行参数
    char args[MAXARG][MAXLENGTH];
    char *ptrArgs[MAXARG];
    char c;
    int i;
    while(1){
        //命令行数组置零
        memset(args, 0,  MAXARG * MAXLENGTH);
        //将除了第一个以外的命令行参数拷贝到args中,第一个命令行参数是"xargs"
        for(i = 1; i < argc; i++){
            strcpy(args[i -1], argv[i]);
            //printf("%s\n", args[i-1]);
        }
        //命令行参数从args[argc-1]开始填入
        int row = argc - 1, col = 0, count, hasPreArgs = 0;

        while( (count = read(0, &c, sizeof(c))) > 0 && c != '\n'){
            //每次读取一个字符，如果是空格，使用hasPreArgs判断是不是第一个空格
            if(c == ' ' && hasPreArgs){
                args[row][col] = '\0';
                row++;
                col = 0;
                hasPreArgs = 0;
            }
            //如果两个参数之间有多个空格，则第一个空格之后的空格都省略
            if(c != ' '){
                args[row][col] = c;
                hasPreArgs = 1;
                col++;
                if(col >= MAXLENGTH){
                    fprintf(2, "argument is too long\n");
                    exit(1);
                }
            }
        }
        //如果读完，则退出循环
        if(count == 0){
            break;
        }
        //如果hasPraArgs为0，说明遇到换行符之前还存在空格，row多加了一次
        if(hasPreArgs == 0){
            row--;
        }
        //否则就是遇到了换行符，执行一次对应的命令
        for(i = 0; i <= row; i++){
            ptrArgs[i] = args[i];
        }
        //最后以NULL结尾
        ptrArgs[row + 1] = 0;
        if(fork() == 0){
            exec(ptrArgs[0], ptrArgs);
            exit(0);
        }else{
            wait((int*) 0);
        }
    }
    exit(0);
}

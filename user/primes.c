#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READEND 0
#define WRITEEND 1
int childProcess(int fd[]){
    int prime, four, number, newfd[2], pid;
    close(fd[WRITEEND]);
    four = read(fd[READEND], &prime, sizeof(int));
    //父进程已经全部筛完，子进程直接退出
    if(four == 0){
        close(fd[READEND]);
        exit(0);
    }
    //fork之前先输出，从而保证输出顺序
    printf("prime %d\n", prime);
    pipe(newfd);
    pid = fork();
    if(pid < 0){
        fprintf(2, "fork error!");
        exit(1);
    }else if(pid == 0){
        //关闭管道fd，子进程的子进程不需要该管道，不关闭会导致管道越来越多，耗尽xv6的资源
        //现在任一时刻最多只有两个管道
        close(fd[READEND]);
        childProcess(newfd);
    }else{
        close(newfd[READEND]);
        while(read(fd[READEND], &number, sizeof(int))){
                if(number % prime != 0){
                    write(newfd[WRITEEND], &number, sizeof(int));
                }
        }
        close(newfd[WRITEEND]);
        close(fd[READEND]);
        wait((int*) 0);
    }
    return 0;
}

int main(int argc, char *argv[]){
    int fd[2];
    int pid, index = 3;
    pipe(fd);
    printf("prime 2\n");
    pid = fork();
    if(pid < 0){
        fprintf(2, "fork error!");
        exit(1);
    }else if(pid == 0){
        childProcess(fd);
    }else{
        close(fd[READEND]);
        while(index <= 35){
            write(fd[WRITEEND], &index, sizeof(int));
            index += 2;
        }
        //父进程的管道的写端必须得关闭，否则管道为空时子进程read读端不会返回0而是阻塞
        close(fd[WRITEEND]);
        //等待子进程结束
        wait((int*) 0);
    }
    exit(0);
    
}

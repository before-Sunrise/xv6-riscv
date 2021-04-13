#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READEND 0
#define WRITEEND 1
int main(int argc, char *argv[])
{
  int pid, parentFd[2], childFd[2];
  char buf[10];
  pipe(parentFd);
  pipe(childFd);
  pid = fork();
  if(pid < 0){
    fprintf(2, "fork error!\n");
    exit(1);
  }
  // child process
  if(pid == 0){
    //子管道只能写
    close(childFd[READEND]);
    //父管道只能读
    close(parentFd[WRITEEND]);
    //从父管道读5个字节
    read(parentFd[READEND], buf, 5);
    //将父管道的5个字节读出
    printf("%d: received %s\n", getpid(), buf);
    //向子管道写入pong
    write(childFd[WRITEEND], "pong", 5);
    //关闭子管道的写端口
    close(childFd[WRITEEND]);
    //关闭父管道的读端口
    close(parentFd[READEND]);
  }else{
    //parent process
    //子管道只能读
    close(childFd[WRITEEND]);
    //父管道只能写
    close(parentFd[READEND]);
    //向父管道写入ping
    write(parentFd[WRITEEND], "ping", 5);
    //将子管道的5个字节读出
    read(childFd[READEND], buf, 5);
    printf("%d: received %s\n", getpid(), buf);
    close(childFd[READEND]);
    close(parentFd[WRITEEND]);
  }
  exit(0);
 
}

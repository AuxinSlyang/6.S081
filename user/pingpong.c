/* pingpong Shell Command */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc >= 2){
    fprintf(2, "Usage: pingpong\n");
    exit(1);
  }

  int p1[2];
  int p2[2];
  pipe(p1);
  pipe(p2);

  int pid = fork();
  if (pid == 0) {
    // child.
    close(p1[1]);
    close(p2[0]);
    char buf[64] = {1};
    int size = 0;
    size = read(p1[0], buf, 64);

    if (size < 0) {
      fprintf(2, "Child Pipe receive error.\n");
      exit(1);
    }

    pid = getpid();
    printf("%d: received ping\n", pid);
    write(p2[1], "world\n", 6);
    close(p1[0]);
    close(p2[1]);
  } else {
    // parent.
    close(p1[0]);
    close(p2[1]);
    write(p1[1], "hello ", 6);
    wait(0);

    int size = 0;
    char buf[64] = {0};
    size = read(p2[0], buf, 12);

    if (size < 0) {
      fprintf(2, "Parent Pipe receive error.\n");
      exit(1);
    }
    pid = getpid();
    printf("%d: received pong\n", pid);
    close(p1[1]);
    close(p2[0]);
  }
  exit(0);
}

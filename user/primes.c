/* primes pipe Shell Command */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "stdbool.h"

int
main(int argc, char *argv[])
{
  if(argc >= 2){
    fprintf(2, "Usage: primes\n");
    exit(1);
  }
  
  int p[2];
  pipe(p);
  int pid = fork();
  if (pid < 0)
  {
    fprintf(2, "Error build a new process!\n");
    exit(-1);
  }
  else if (pid > 0)
  {
    // parent process just generate number
    close(p[0]);
    for (int i = 2; i <= 35; i++) {
      write(p[1], &i, 4);
    }
    close(p[1]);

    wait(0);
  }
  else
  {
    // child
    close(p[1]);
    int divide = 2;
    bool flag = false;

    // send usage pipe.
    int p1[2];
    int num;
    int size = read(p[0], &num, 1);
    while (size) {
      if (num == divide)
      {
        printf("prime %d\n", divide);
      } 
      else if (num % divide != 0)
      {
        if (!flag)
        {
          pipe(p1);
          int pid = fork();
          if (pid == 0)
          {
            // child.
            divide = num;
            p[0] = p1[0];
            close(p1[1]);
          }
          else
          {
            // parent.
            close(p1[0]);
            flag = true;
            write(p1[1], &num, 4);
          }
        }
        else
        {
          write(p1[1], &num, 4);
        }
      }
      size = read(p[0], &num, 1);
    }

    if (flag) {
      close(p1[1]);
      close(p[0]);
      wait(0);
    }
  }


  exit(0);
}

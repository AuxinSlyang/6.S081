#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2
#define BUFFER_SIZE 512

char*
xagrs_gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    if(c == '\n' || c == '\r')
      break;
    buf[i++] = c;
  }
  buf[i] = '\0';
  return buf;
}

int
getline(char *buf, int nbuf)
{
  memset(buf, 0, nbuf);
  xagrs_gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
        fprintf(STD_ERR, "Usage: xargs command\n");
        exit(1);
  }

  char *_argv[MAXARG];
  int _argc = argc - 1;
  for (int i = 0; i < _argc; i++) {
    _argv[i] = argv[i + 1];
  }

  char buf[BUFFER_SIZE];
  while (!getline(buf, BUFFER_SIZE)) {
    // get line
    if (fork() == 0) {
        _argv[_argc] = buf;
        exec(_argv[0], _argv);
        fprintf(STD_ERR, "exec %s failed\n", argv[1]); // 执行失败
        exit(0);
    }
    wait(0);
  }

  exit(0);
}
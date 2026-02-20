#include <stdio.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
  pid_t childpid[3];
  char *messages[] = {"aaa", "bbbbbbbb", "ccccc"};
  char buffer[15] = { 0 };
  int status;
  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
  {
    perror("socketpair");
    exit(1);
  }
  for (size_t i = 0; i < 3; i++)
  {
    childpid[i] = fork();
    if (childpid[i] == -1)
    {
      perror("fork");
      exit(1);
    }
    else if (childpid[i] == 0)
    {
      if (close(sockets[0]) == -1) {
        perror("close");
        exit(1);
      }
      if (write(sockets[1], messages[i], strlen(messages[i]) + 1) == -1) {
        perror("write");
        exit(1);
      }
      printf("child %d send: %s\n", i, messages[i]);
      if (read(sockets[1], buffer, sizeof(buffer)) == -1) {
        perror("read");
        exit(1);
      }
      if (close(sockets[1]) == -1) {
        perror("close");
        exit(1);
      }
      printf("child %d recieve: %s\n", i, buffer);
      exit(0);
    }
    else
    {
      if (read(sockets[0], buffer, sizeof(buffer)) == -1) {
        perror("read");
        exit(1);
      }
      printf("parent recieve: %s from child %d\n", buffer, i);
      sprintf(buffer, "%s+ok", buffer);
      if (write(sockets[0], buffer, strlen(buffer) + 1) == -1) {
        perror("write");
        exit(1);
      }
      printf("parent send: %s to child %d\n", buffer, i);
    }
  }
  close(sockets[0]);
  for (size_t i = 0; i < 3; i++)
  {
    pid_t childpid = wait(&status);
    if (WIFEXITED(status))
      printf("Child %d exit with code: %d\n", i, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      printf("Child %d finish from signal with code: %d\n", i, WTERMSIG(status));
    else if (WIFSTOPPED(status))
      printf("Child %d stop with code: %d\n", i, WSTOPSIG(status));
  }
  exit(0);
}

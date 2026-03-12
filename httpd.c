#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
#include <sys/stat.h>
#include <sys/wait.h>


void sigchld_han(int signum)
{
  int status;
  while (waitpid(-1, &status, WNOHANG) > 0)
  printf("child process terminated\n");
}





void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r+"); 
   char *line = NULL;
   size_t size =0;
   ssize_t num;
 

   

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }
   num = getline(&line, &size, network);
   if (num == -1)
   {
      perror("getline");
      free(line);
      fclose(network);
      return;
   }
   char method[10];
   char path[100];
   char protocol[10];
   sscanf(line, "%s %s %s", method, path, protocol);
    



   while ((num = getline(&line, &size, network)) >0){
      if (strcmp(line, "\r\n") == 0|| strcmp(line, "\n") == 0)
        break;
    }
      char *filepath = path+1;
      FILE *file = fopen(filepath, "r");
     
      
      if (file == NULL){

        fprintf(network, "HTTP/1.0 404 Not Found\r\n\r\n");
      }else{
        struct stat st;
        stat(filepath, &st);
        long file_size = st.st_size;
        
        fprintf(network, "HTTP/1.0 200 OK\r\n");
        fprintf(network, "Content-Type: text/html\r\n");
        fprintf(network, "Content-Length: %ld\r\n", file_size);
        fprintf(network, "\r\n");
         if (strcmp(method, "GET") == 0){
            char buffer[1024];
            while(fgets(buffer, sizeof(buffer), file) != NULL){
             fprintf(network, "%s", buffer);
        }
      }
        fclose(file);

   
   fflush(network);
   free(line);
   fclose(network);
}

void build_response(){


}

void run_service(int fd)
{
   signal(SIGCHLD, sigchld_han);
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
        pid_t pid = fork();
        if (pid == 0)
        {
         close (fd);
         handle_request(nfd);
         close(nfd);
         exit(0);
        
      
         }
         else if (pid > 0)
         {
            close(nfd);
         }
         else
         {
            perror("fork");
            close(nfd);
         }
      }
   }
}


int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);

    if(port < 1024 || port > 65535)
    {
        fprintf(stderr, "Invalid port number: %d\n", port);
        return 1;
    }
    int fd = create_service(port);
    if (fd == -1)
   {
      perror("create_service");
      exit(1);
   }

   printf("listening on port: %d\n", port);
   run_service(fd);
   close(fd);

   return 0;
}

/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}

void * read_file_and_send_result(void *arg)
{

  //buffer for reading and writing the messages
  char buffer[1024]; 
  int newsockfd = *(int *)(arg);
  printf("Start\t%d\t%ld\n", newsockfd, pthread_self());
  
  //Variables to store C file, compilation error, runtime error and output error.
  char pthread_sel[100]="";
  char c_file_name[100]="";
  char executable_file_name[100] = "";
  char compilation_file_name[100]="";
  char runtime_error_file_name[100]="";
  char output_file_name[100]="";
  char pthread_selff[100] = "";

  sprintf(pthread_sel,"%ld", pthread_self()%10000);

  strcat(pthread_selff, pthread_sel);
  strcat(c_file_name, strcat(pthread_sel, "_C_CODE.c"));
  pthread_sel[0] = '\0';
  strcat(pthread_sel, pthread_selff);
  strcat(executable_file_name, strcat(pthread_sel, "_C_CODE"));
  pthread_sel[0] = '\0';
  strcat(pthread_sel, pthread_selff);
  strcat(compilation_file_name, strcat(pthread_sel, "_COMPILATION_ERROR"));
  pthread_sel[0] = '\0';
  strcat(pthread_sel, pthread_selff);
  strcat(runtime_error_file_name, strcat(pthread_sel, "_RUNTIME_ERROR"));
  pthread_sel[0] = '\0';
  strcat(pthread_sel, pthread_selff);
  strcat(output_file_name, strcat(pthread_sel, "_OUTPUT.txt"));


  //printf("%s\t%s\t%s\t%s\t%s\n",c_file_name, executable_file_name, compilation_file_name, runtime_error_file_name, output_file_name);

  //Various Commands that will be used
  char compilation_command[100] = "gcc ";
  char execute_command[100] = "./";
  char diff_command[100] = "diff actual_output.txt ";
  strcat(strcat(strcat(compilation_command, c_file_name), " -o "), executable_file_name);
  strcat(strcat(strcat(strcat(strcat(execute_command, executable_file_name), " 1>"), output_file_name), " 2>"), runtime_error_file_name);
  strcat(strcat(diff_command, output_file_name), " >diff.txt");
  //printf("%s\n%s\n%s\n", compilation_command, execute_command, diff_command);

  //Reading C file from client
  memset(buffer, 0, 1024);
  int characters_read = read(newsockfd, buffer, 1024);
  int fd = open(c_file_name , O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
  write(fd, buffer, characters_read);
  close(fd);
  printf("Mid\t%d\t%ld\n", newsockfd, pthread_self());
  //Compile and Run
  char message[5] = {0};

  if(characters_read == 0)
  {
    printf("File could not be read\n");
  }
  else if(system(compilation_command))
  {
    strcpy(message, "CERR");
    write(newsockfd, message, strlen(message)+1);
    memset(buffer, 0, 1024);
    int fdc = open("compilation_error", O_RDONLY);
    write(newsockfd, buffer, read(fdc, buffer, 1024));
    close(fdc);
    //system("rm c_code.c compilation_error");
  }
  else if(system(execute_command))
  {
    strcpy(message, "RERR");
    write(newsockfd, message, strlen(message)+1);
    memset(buffer, 0, 1024);
    int fdr = open("runtime_error", O_RDONLY);
    write(newsockfd, buffer, read(fdr, buffer, 1024));
    close(fdr);
    //system("rm c_code.c c_code output.txt runtime_error compilation_error");
  }
  else if(system(diff_command))
  {
    strcpy(message, "OERR");
    write(newsockfd, message, strlen(message)+1);
    memset(buffer, 0, 1024);
    int fdd = open("diff.txt", O_RDONLY);
    write(newsockfd, buffer, read(fdd, buffer, 1024));
    close(fdd);
    //system("rm c_code.c c_code output.txt runtime_error compilation_error diff.txt");
  }
  else
  {
    strcpy(message, "PASS");
    write(newsockfd, message, strlen(message)+1);
    //system("rm c_code.c c_code output.txt runtime_error compilation_error diff.txt");
  }
  
  printf("Done\t%d\t%ld\n", newsockfd, pthread_self());
  close(newsockfd);
  
}

int main(int argc, char *argv[]) {
  int sockfd,  portno, newsockfdodd, newsockfdeven, newsockfd[300] = {0}; 

  
  
  socklen_t clilen; 
  struct sockaddr_in serv_addr, cli_addr; 
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  

  if (sockfd < 0)
    error("ERROR opening socket");


  bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros

  serv_addr.sin_family = AF_INET; // Address Family of INTERNET
  serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address. 

  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  listen(sockfd, 1); 

  clilen = sizeof(cli_addr);  

  int index = 0;
  //char flag = 'e';
  while (1)
  {
    newsockfd[index] = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd[index] < 0)
      error("ERROR on accept");
    pthread_t t1 ;
    int rv = pthread_create(&t1, NULL, &read_file_and_send_result, (void *)&newsockfd[index]);
    if(rv != 0)
      printf("Thread could not be created\n");
    index = (index+1)%300;
    //sleep(10);

    /*if(flag == 'e')
    {
      newsockfdeven = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
      if (newsockfdeven < 0)
        error("ERROR on accept");
      pthread_t t1 ;
      int rv = pthread_create(&t1, NULL, &read_file_and_send_result, (void *)&newsockfdeven);
      if(rv != 0)
        printf("Thread could not be created\n");
      flag = 'o';
    }
    else
    {
      newsockfdodd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
      if (newsockfdodd < 0)
        error("ERROR on accept");
      pthread_t t1 ;
      int rv = pthread_create(&t1, NULL, &read_file_and_send_result, (void *)&newsockfdodd);
      if(rv != 0)
        printf("Thread could not be created\n");
      flag = 'e';
    }*/
  }
  return 0;
}

/* run using ./server <port> <thread_pool_size>*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>


int autograding_request[1000];
int queue_size = 0;
int queue_front = 0;
int queue_rear = -1;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void error(char *msg) {
  perror(msg);
  exit(1);
}

void * read_file_and_send_result(void *arg)
{ 
  char buffer[1024];
  int newsockfd;
  int counter = 0;
  while(1)
  {
    counter++;
    pthread_mutex_lock(&queue_mutex);
    while(queue_size == 0)
      pthread_cond_wait(&empty, &queue_mutex);
    newsockfd = autograding_request[queue_front];
    queue_size--;
    queue_front = (queue_front + 1) % 1000;
    pthread_mutex_unlock(&queue_mutex);

    printf("Start\t%d\t%ld\n", newsockfd, pthread_self());

    //Variables to store C file, compilation error, runtime error and output error.
    char intermediate_name[100]="";
    char c_file_name[100]="";
    char executable_file_name[100] = "";
    char compilation_file_name[100]="";
    char runtime_error_file_name[100]="";
    char output_file_name[100]="";
    char diff_file_name[100]="";
    char thread_id[100] = "";
    char count[30]; 

    sprintf(thread_id,"%ld", pthread_self());
    sprintf(count, "%d", counter);

    strcat(intermediate_name, thread_id);
    strcat(intermediate_name, count);
    strcat(c_file_name, strcat(intermediate_name, "_C_CODE.c"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, thread_id);
    strcat(intermediate_name, count);
    strcat(executable_file_name, strcat(intermediate_name, "_C_CODE"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, thread_id);
    strcat(intermediate_name, count);
    strcat(compilation_file_name, strcat(intermediate_name, "_COMPILATION_ERROR"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, thread_id);
    strcat(intermediate_name, count);
    strcat(runtime_error_file_name, strcat(intermediate_name, "_RUNTIME_ERROR"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, thread_id);
    strcat(intermediate_name, count);
    strcat(output_file_name, strcat(intermediate_name, "_OUTPUT.txt"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, thread_id);
    strcat(intermediate_name, count);
    strcat(diff_file_name, strcat(intermediate_name, "_diff.txt"));
    


    //printf("%s\n%s\n%s\n%s\n%s\n%s\n",c_file_name, executable_file_name, compilation_file_name, runtime_error_file_name, output_file_name, diff_file_name);

    //Various Commands that will be used
    char compilation_command[100] = "gcc ";
    char execute_command[100] = "./";
    char diff_command[100] = "diff actual_output.txt ";
    char remove_command[100] = "rm ";
    strcat(strcat(strcat(strcat(strcat(compilation_command, c_file_name), " -o "), executable_file_name)," 2>"),compilation_file_name);
    strcat(strcat(strcat(strcat(strcat(execute_command, executable_file_name), " 1>"), output_file_name), " 2>"), runtime_error_file_name);
    strcat(strcat(strcat(diff_command, output_file_name), " >"), diff_file_name);
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
      int fdc = open(compilation_file_name, O_RDONLY);
      write(newsockfd, buffer, read(fdc, buffer, 1024));
      close(fdc);
      strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name);
      system(remove_command);
    }
    else if(system(execute_command))
    {
      strcpy(message, "RERR");
      write(newsockfd, message, strlen(message)+1);
      memset(buffer, 0, 1024);
      int fdr = open(runtime_error_file_name, O_RDONLY);
      write(newsockfd, buffer, read(fdr, buffer, 1024));
      close(fdr);
      strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name);
      system(remove_command);
    }
    else if(system(diff_command))
    {
      strcpy(message, "OERR");
      write(newsockfd, message, strlen(message)+1);
      memset(buffer, 0, 1024);
      int fdd = open(diff_file_name, O_RDONLY);
      write(newsockfd, buffer, read(fdd, buffer, 1024));
      close(fdd);
      strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name), " "), diff_file_name);
      system(remove_command);
    }
    else
    {
      strcpy(message, "PASS");
      write(newsockfd, message, strlen(message)+1);
      strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name), " "), diff_file_name);
      system(remove_command);
    }
    printf("Done\t%d\t%ld\n", newsockfd, pthread_self());
    close(newsockfd);
  }
}

int main(int argc, char *argv[]) {
  int sockfd,  portno, newsockfdodd, newsockfdeven, newsockfd; 
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 3) {
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

  int number_of_threads = atoi(argv[2]);
  pthread_t thread[number_of_threads];

  //char flag = 'e';

  for(int i = 0; i < number_of_threads; i++)
  {
    thread[i] = pthread_create(&thread[i], NULL, &read_file_and_send_result, NULL);
    if(thread[i] != 0)
      printf("Thread could not be created\n");   
  }

  while(1)
  {
    printf("Waiting for a new connection\n");
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    pthread_mutex_lock(&queue_mutex);
    /*autograding_request[rear_index] = newsockfd;
    printf("Main Thread inserted a value\t:%d\n", autograding_request[rear_index]);
    rear_index++;
    if(rear_index == 1)
      front_index = 0;
    pthread_cond_signal(&empty);*/
    if (queue_size < 1000) {
        queue_size++;
        queue_rear = (queue_rear + 1) % 1000;
        autograding_request[queue_rear] = newsockfd;
        pthread_cond_signal(&empty);
    } else {
        printf("Queue is full. Connection dropped.\n");
        close(newsockfd);
    }
    pthread_mutex_unlock(&queue_mutex);
  }
  return 0;
}
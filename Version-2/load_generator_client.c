/* run client using: ./client localhost <server_port> c_file numberofiteration sleeptime timeout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>

long int response_time, response_time_sum = 0, response_time_avg=0, total_time;
int successful_response = 0;
int timeout;
struct timeval Throughput_start, Throughput_end;

void error(char *msg) {
  perror(msg);
  exit(0);
}

void alarm_handler(int signum) {
  printf("Time out\n");
  if(successful_response != 0){
    gettimeofday(&Throughput_end, NULL);
    total_time = ((Throughput_end.tv_sec * 1000000) + Throughput_end.tv_usec) - ((Throughput_start.tv_sec * 1000000) + Throughput_start.tv_usec);
    printf("Successful responses=%d\n", successful_response);
    printf("Total response time in microseconds=%ld\n", response_time_sum+timeout*1000000);
    response_time_avg = (response_time_sum+timeout*1000000)/successful_response ;
    printf("Average response time in microseconds=%ld\n", response_time_avg);
    float total_time_sec=total_time/1000000.0;
    printf("Throughput=%f\n", successful_response/total_time_sec);
    exit(0);
  }
  else{
    printf("I have 0 successful responses\n");
    printf("Successful responses=%d\n", successful_response);
    printf("Total response time in microseconds=%d\n", timeout);
    printf("Average response time in microseconds=%d\n", timeout);
    printf("Throughput=%f\n", (float)successful_response);
    exit(0);
  }
}

int main(int argc, char *argv[]) {

  signal(SIGALRM, alarm_handler);
  timeout = atoi(argv[6]);
  int sockfd, portno, n;
  
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname

  char buffer[1024] = {0}; //buffer for message

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[2]); // 2nd argument of the command is port number

  /* create socket, get sockfd handle */

  server = gethostbyname(argv[1]);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }


  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  serv_addr.sin_family = AF_INET; // Address Family is IP

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  /*Copy server IP address being held in h_addr field of server variable
  to sin_addr.s_addr of serv_addr structure */

  //convert host order port number to network order
  serv_addr.sin_port = htons(portno);
  
  gettimeofday(&Throughput_start, NULL);

  int l;
  for(l=0; l<atoi(argv[4]); l++)
  {
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
    //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
    //printf("%d\n", getpid());
    //printf("%d\n", sockfd);
    if (sockfd < 0)
      error("ERROR opening socket");

    /* fill in server address in sockaddr_in datastructure */  

    /* connect to server 
    First argument is the half-socket, second is the server address structure
    which includes IP address and port number, third is size of 2nd argument
    */
    alarm(atoi(argv[6]));

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      error("ERROR connecting");

    //Reading Local Time
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    //Sending C file
    int fd = open(argv[3], O_RDONLY);
    write(sockfd, buffer, read(fd, buffer, 1024));
    close(fd);


    //Reading Message
    char message[5] = {0};
    read(sockfd, message, 5);
    if(!strcmp(message, "CERR"))
      printf("COMPILATION_ERROR\n");
    else if(!strcmp(message, "RERR"))
      printf("RUNTIME_ERROR\n");
    else if(!strcmp(message, "OERR"))
      printf("Output Error\n");
    else
      printf("PASS\n");

    if(strstr(message, "ERR"))
    {
      memset(buffer, 0, 1024);
      read(sockfd, buffer, 1024);
      printf("%s\n", buffer);
    }
    alarm(0);
    //Reading local time
    gettimeofday(&end, NULL);

    
    response_time = ((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec);
    response_time_sum += response_time ;
    successful_response++ ;

    sleep(atoi(argv[5]));
    close(sockfd);
  }

  gettimeofday(&Throughput_end, NULL);
  total_time = ((Throughput_end.tv_sec * 1000000) + Throughput_end.tv_usec) - ((Throughput_start.tv_sec * 1000000) + Throughput_start.tv_usec);

  printf("Successful responses=%d\n", successful_response);
  printf("Total response time in microseconds=%ld\n", response_time_sum);
  response_time_avg = response_time_sum/successful_response ;
  printf("Average response time in microseconds=%ld\n", response_time_avg);
  float total_time_sec=total_time/1000000.0;
  printf("Throughput=%f\n", successful_response/total_time_sec);
  return 0;  
}

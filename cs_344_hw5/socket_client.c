/* 
 * Kamil Agrawala
 * agrawalk@onid.orst.edu
 * CS344-001
 * Assingment #5: socket_client.c
 * Citations: 
 *            Getopt: http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt              
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <limits.h>

#include "socket_hdr.h"

#define QUOTE(_NAME_) #_NAME_
#define STR(_NAME_) QUOTE(_NAME_)

# define SEND_FILE_FLAGS (O_CREAT | O_WRONLY | O_TRUNC)
# define synchMessage "START"


/* Function declarations */

void helpFunction(void);



void helpFunction(void){
  puts("  Available client commands:");
  puts("  help      : print this help text");
  puts("  exit      : exit the client, causing the client server to also exit");
  puts("  cd  <dir> : change to directory <dir> on the server side");
  puts("  lcd <dir> : change to directory dir> on the client side");
  puts("  dir       : show a `ls -lF` of the server side");
  puts("  ldir      : show a `ls -lF` on the client side");
  puts("  home      : change current directory of the client server to the user's home directory");
  puts("  lhome     : change current directory of the client to the user's home directory");
  puts("  pwd       : show the current directory from the server side");
  puts("  lpwd      : show the current directory on the client side");
  puts("  get <file>: send <file> from server to client");
  puts("  put <file>: send <file> from client to server");
  puts("  clear     : clear screen");
}



int 
main(
     int argc
     , char *argv[]
     , char *envp[])
{
  // File Descriptors
  int sockfd;
  int fd_c;
  struct sockaddr_in servaddr;
  //char sendline[MAXLINE];
  char recvline[MAXLINE];
  char writeBuff[MAXLINE];
  char putCommand[MAXLINE];
  // char readBuff[MAXLINE];
  ssize_t ln;
  FILE *file;
  char * token;
  int option = 0;
  char *ipAddr;
  char *serverPort;
  int p_flag = 0; 
  int i_flag = 0;
  
  umask(0);

  while ((option = getopt(argc, argv,"p:i:")) != -1) {
    switch (option) {
    case 'p' :
      p_flag = 1;
      serverPort = optarg;
      fputs("p flag triggered\n",stderr);
      break;
    case 'i':
      i_flag = 1;
      ipAddr = optarg;
      fputs("i flag triggered\n",stderr);
      break;
    default: printf("Usuage: ./socket_client -p <PORT_NUMBER> -i <IP_ADDR>\n");
      exit(EXIT_FAILURE);
    }
  }
  
  if ((i_flag == 1) && (p_flag == 1)){

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atol(serverPort));
    inet_pton(AF_INET, ipAddr, &servaddr.sin_addr.s_addr);
  
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
      perror("could not connect");
      exit(EXIT_FAILURE);
    }

    do {
      printf(PROMPT);
      fflush(stdout);
    
      memset(writeBuff, 0,MAXLINE);

      fgets(writeBuff,MAXLINE, stdin);
    
      ln = strlen(writeBuff) - 1;
      if (writeBuff[ln] == '\n'){
	writeBuff[ln] = 0;
      }
    
      if(strcmp(writeBuff,CMD_HELP) == 0){
	helpFunction();
      }
      else if(strcmp(writeBuff,CMD_LOCAL_DIR) == 0){
	file = popen(CMD_LS_POPEN,"r");
	while(fgets(writeBuff,sizeof(writeBuff),file) != NULL){
	  printf("%s",writeBuff);
	}
	pclose(file);
      }
      else if(strncmp(writeBuff,"clear",(ssize_t)4) == 0){
	file = popen("clear","w");
	perror("Local Popen (Extra Clear Command)");
	pclose(file);
	//perror("Local Popen (Extra Clear Command)");
      }
      else if (strcmp(writeBuff,CMD_LOCAL_PWD) == 0){
	file = popen("pwd","r");
	//perror("Local Popen for PWD");
	while(fgets(writeBuff,sizeof(writeBuff),file) != NULL){
	  printf("%s",writeBuff);
	}
	pclose(file);
      }
      else if(strcmp(writeBuff,CMD_LOCAL_HOME) == 0){
	memset(writeBuff,' ',sizeof(writeBuff));
	chdir(getenv("HOME"));
	perror("chdir: ");
	getcwd(writeBuff,sizeof(writeBuff));
	perror("getcwd: ");
	fprintf(stdout,"HOME: <%s>  CWD: <%s> \n",getenv("HOME"),writeBuff);
      }else if(strncmp(writeBuff,CMD_LOCAL_CHDIR, (ssize_t)3) == 0){
	int k;
	fprintf(stderr,"Changing Local Client Directory\n");
	printf("lcd command: <%s>\n", writeBuff);

	token = strtok(writeBuff," ");
	token = strtok(NULL, " ");
	printf("Changing into <<<%s>>>\n", token);
      
	if(token[0] != 0){
	  //Change the directory
	  k = chdir(token);
	  perror("chdir:");
	
	  // Get new CWD                                                                                                                                      
	  getcwd(token,sizeof(token));
	  if(k == 0){
	    fprintf(stdout,"Local Client PWD is now <%s>\n",token);
	  }else{
	    fprintf(stdout,"Local Client PWD was not **changed**\n");
	  }
	  memset(token, 0, sizeof(token));
	}else{
	  printf("Format is lcd  <dir>\n");
	}
      }else if (strcmp(writeBuff,CMD_REMOTE_PWD) == 0){
	// send the message
	write(sockfd, writeBuff, strlen(writeBuff));
	perror("write (pwd): ");

	// receive the message
	read(sockfd, recvline, MAXLINE);
	perror("read (pwd): ");

	// print
	fprintf(stderr,"Remote Server PWD:\n \t%s\n",recvline);

      }else if(strncmp(writeBuff,CMD_REMOTE_DIR,(ssize_t)3) == 0){
	//printf("CM_REMOTE_DIR : <%s>\n", writeBuff);
	//Send Dir request
	write(sockfd, writeBuff, strlen(writeBuff));
	while(1){
	  // receive the message
	  read(sockfd, recvline, MAXLINE);
	  //perror("read (pwd): ");

	  if(strncmp(recvline, STR(EOT_CHAR), (ssize_t) MAXLINE) == 0 ){
	    break;
	  }else{
	    fprintf(stdout,"%s",recvline);
	  }
	}
      }else if(strncmp(writeBuff, CMD_REMOTE_CHDIR,(ssize_t)2) == 0){
	fprintf(stderr, "%s\n", writeBuff);

	// Write Request to Server for CD <dir>
	write(sockfd, writeBuff, strlen(writeBuff));
	perror("write (cd dir): ");

	// Read the response
	read(sockfd, recvline, MAXLINE);
	perror("read (CD dir) ");
	fprintf(stderr,"Remote Server PWD Changed:\n \t%s\n",recvline);
      }else if(strncmp(writeBuff, CMD_REMOTE_HOME,(ssize_t)4) == 0){
	fprintf(stderr, "%s\n", writeBuff);
	// Write Request to Server for CD <dir>                                                                                                                
	write(sockfd, writeBuff, strlen(writeBuff));
	perror("write (cd dir): ");

	// Read the response                                                                                                                                   
	read(sockfd, recvline, MAXLINE);
	perror("read (CD dir) ");

	fprintf(stdout,"\nHOME: <%s>  CWD: <%s> \n",getenv("HOME"),recvline);
      }else if(strstr(writeBuff,CMD_GET_FROM_SERVER) != NULL){
	fprintf(stderr, "EOT_CHAR is \n\t%s\n",STR(EOT_CHAR));
	// Send GET Request to Server
	write(sockfd, writeBuff, MAXLINE);
	perror("Write (initial) GET");
      
	token = strtok(writeBuff," ");
	token = strtok(NULL," ");
      
	// open file to be written to
	fd_c = open(token, SEND_FILE_FLAGS, SEND_FILE_PERMISSIONS);
	perror("fd_c open");
      
	// receive the message
	while(1){
	  int numBytes;
	  // receive the message
	  numBytes = read(sockfd, recvline, MAXLINE);
	  perror("read (GET) ");
	  fprintf(stderr, "%d\n", numBytes);
	  //fprintf(stderr, "<%s>",recvline);

	  if(strcmp(STR(EOT_CHAR), recvline) == 0){
	    close(fd_c);
	    perror("close fd_c");
	    break;
	  }else{
	    write(fd_c, recvline, numBytes);
	    //memset(recvline, 0, sizeof(recvline));      
	    perror("write (GET) ");
	  }
	
	  memset(recvline, 0, sizeof(recvline));      
	}
      }else if(strncmp(CMD_PUT_TO_SERVER,writeBuff,(ssize_t)3) == 0){
	printf("PUT command: <%s>\n", writeBuff);
      
	strcpy(putCommand,writeBuff);
      
	token= strtok(writeBuff," ");
	token = strtok(NULL," ");
      
	if(access(token, F_OK ) != -1 ) {
	  int k;
	  /****  file exists  ****/

	  //Set Message command to put
	  fprintf(stderr,"Sending <<<%s>>>\n",putCommand);
	  write(sockfd, putCommand, sizeof(putCommand));
	  

	  // Send the intial synch message
	  write(sockfd, putCommand, sizeof(putCommand));
	  perror("Write 1");
	  
	  printf("PUT File: <%s>\n", token);
	  
	  //Memset message to 0
	  memset(putCommand, 0 ,sizeof(putCommand));
	  
	  //OPEN filename for reading
	  fd_c = open(token,O_RDONLY);
	  perror("Open fd_c");
	
	  while((k = read(fd_c, writeBuff,MAXLINE)) != 0){
	    fprintf(stderr,"<<<%d>>>\n",k);
	    write(sockfd, writeBuff, k);
	    if (k == -1){
	      fprintf(stderr,"*** Read Error BREAK ***\n");
	      break;
	    }
	  }
	
	  fprintf(stderr,"Done sending file <%s>\n",token);
	
	  //Send Final Message EOT MESSAGE
	  strcpy(writeBuff,STR(EOT_CHAR));
	
	  /* // Send final message */
	  write(sockfd, writeBuff, MAXLINE); 
	  perror("write (final)"); 
	}
	else {
	  char *currentDir = getcwd(currentDir,PATH_MAX);
	  fprintf(stderr,"File: <%s> does not exist in client's cwd (%s) \n",token,currentDir);
	}
    
      }else{
	// send the message
	write(sockfd, writeBuff, strlen(writeBuff));
	perror("write (other) ");
    
	// receive the message
	read(sockfd, recvline, MAXLINE);
	perror("read (other) ");
	fprintf(stderr,"<%s>\n",recvline);
      }
    }while(strcmp(writeBuff,CMD_EXIT) != 0);
 
    close(sockfd);
    return(EXIT_SUCCESS);
  }else{
    printf("Usuage2: ./socket_client -p <PORT_NUMBER> -i <IP_ADDR> \n");
    return(EXIT_FAILURE);
  }
}


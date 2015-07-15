/* Kamil Ali Agrawala
 * agrawalk@onid.orst.edu
 * CS344-001
 * Assingment 5
 * Citations:  
 *               fgets():  http://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm
 *               getopt(): http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt  
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

// This will be the backlog argument.  The maximum length
// to which the queue of pending connections for sockfd
// may grow.
//#define LISTENQ 1024      // 2nd argument to listen()

//#define MAXLINE 4096      // max text line length

#define NOT_IN_USE -1
#define QUOTE(_NAME_) #_NAME_
#define STR(_NAME_) QUOTE(_NAME_)

/*
 * Structure to hold all client information
 */
typedef struct clients{
  int  avail;
  int  changed;
  int  clientfd; // MAX_SIZE fd's for select
  char pwd[PATH_MAX];
}clients;

/*
 * Helper Functions
 */
int findFree(clients clientsArray[]);
int dirFunction(clients mArray[], int sock_fd);
int changeDirFunction(clients mArray[],int pos, int sock_fd, char* dir);
int getFunction(char* filename, int sock_fd, clients clients[], int pos);
int putFunction(char* filename, int sock_fd, clients clients[], int pos);

// Allocate Space for MAX Clients
clients mclients[MAX_CLIENTS];
char serverHome[PATH_MAX];

int
main(
     int argc
     , char *argv[]
     , char *envp[])
{
  int i;
  int maxfd;
  int listenfd;
  int connfd;
  // File Descriptors
  int sockfd;
  
  int nready;
  char *token;
  //int client[MAX_CLIENTS]; // max size of a fd set for select
  int nclients;
  fd_set rset;
  fd_set allset;
  char buf[MAXLINE];
  //char resp[PATH_MAX];
  ssize_t nbytes;
  socklen_t clilen;
  struct sockaddr_in cliaddr;
  struct sockaddr_in servaddr;
  //struct timeval timeout;

  // Before Anything get the "Virtual" Server Home Directory
  getcwd(serverHome,PATH_MAX);
  perror("Server Home getcwd: ");

  fprintf(stderr,"Virtual Server Home is set to\n\t%s\n",serverHome);

  //Set all mclients as available
  for(i=0; i<= MAX_CLIENTS; i++){
    mclients[i].avail = 0; // 0 means available -1 means not available
    getcwd(mclients[i].pwd,PATH_MAX); // Set Inital home path
  }

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  // Make sure everything is nice and clean.
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atol(argv[1]));

  if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
    perror("cannot bind address");
    exit(EXIT_FAILURE);
  }

  if (listen(listenfd, LISTENQ) != 0) {
    perror("cannot listen on port address");
    exit(EXIT_FAILURE);
  }
  printf("server listening on %s\n", argv[1]);  

  // For now, this is out highest numbered file desc that
  // we will check.
  maxfd = listenfd; // initialize
  nclients = 0;

  // Go through the entire array and initialize the entries
  // to a "not used" state.  We know that all file descriptors
  // are a non-negative value, so using -1 indicates it is 
  // unused.
  for (i = 0 ; i < MAX_CLIENTS ; i++) {
    mclients[i].clientfd = NOT_IN_USE; // A value of -1 indicates available entry
  }

  // Clean out the set of all file descriptors.
  FD_ZERO(&allset);

  // Add the listen file desc into the list of all descriptors.
  FD_SET(listenfd, &allset);
  //timeout.tv_sec = 5;  // 5 seconds
  for ( ; ; ) {
    rset = allset;

    printf("sever: calling select with %d clients\n>> ", nclients);
    fflush(stdout);

    // In this case, we are only looking at the readfds.
    // We are looking at either the writefds or the exceptfds.
    // We are not specifying a time out value.
    //
    // Select will return the total number of fds that have
    // become ready for i/o.
    //
    // A file descriptor is considered ready if it is possible
    // to perform the corresponding I/O operation without blocking.
    //
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    //timeout.tv_sec = 5;  // 5 seconds
    //nready = select(maxfd + 1, &rset, NULL, NULL, &timeout);
    printf("\n");

    //    if (nready == 0) {
    //      printf("  server: timeout occured\n");
    //      continue;
    //    }

    printf("  server: select found %d ready fds\n", nready);

    if (nready == -1) {
      perror("select returned failure");
      exit(EXIT_FAILURE);
    }

    // Check the listenfd first.  If it is ready, then it is
    // a new client connection request.
    if (FD_ISSET(listenfd, &rset)) {
      // A new client connection...  Woooo  Whoooo!!!
      nclients ++;
      printf("  server: a new client connection request\n");
      clilen = sizeof(cliaddr);

      // Get the new connection file desc.
      connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

      // Find a place to save the new file descriptor in
      // our set of file descriptors.
      for (i = 0 ; i < MAX_CLIENTS ; i++) {
	if (mclients[i].clientfd < 0) {
	  mclients[i].avail = -1; // Make not availble
	  mclients[i].clientfd = connfd; // save descriptor
	  break;              // not need to keep looking
	}
      }
      
      if (i == MAX_CLIENTS) {
	printf("!!! TOO MANY CLIENTS  !!!!\n");
	// This might not be the best way to handle this condition,
	// but it is a good demonstration.
	//close(connfd);
	exit(EXIT_FAILURE);
      }

      FD_SET(connfd, &allset); // add new descriptor to  the all set

      // Make sure that we keep track of the highest numbered
      // file descriptor.  We need it for the select() call.
      if (connfd > maxfd) {
	maxfd = connfd;
      }

      // Decrement the number of processed file desc left
      // to handle.
      if (--nready <= 0) {
	continue; // no more readable descriptors
      }
    }

    // Check file desc to see if there is ready data.
    for (i = 0 ; i <= MAX_CLIENTS ; i++) {
      sockfd = mclients[i].clientfd;
      if (sockfd < 0) {
	// not a spot in use, keep looking
	continue;
      }
      // This checks to see if this file desc is one of the
      // ready ones.
      if (FD_ISSET(sockfd, &rset)) {
	printf("  server: received communication from client\n");

	// Make sure this is nice and clean.
	memset(buf, 0, sizeof(buf));
	// Received data on this file desc, read and echo it.
	if ((nbytes = read(sockfd, buf, sizeof(buf))) == 0) {
	  // The connection was closed by the client, because
	  // we received a zero bytes on the the read.
	  // Close the socket fd, remove it from the list to check,
	  // and set its spot to available.
	  close(sockfd);
	  FD_CLR(sockfd, &allset);
	  mclients[i].clientfd = NOT_IN_USE;
	  mclients[i].avail = 0;
	  printf("    server: client closed connection\n");
	  nclients --;
	}else if(strncmp(CMD_REMOTE_PWD,buf,(ssize_t)3) == 0){
	  fprintf(stderr,"***nbytes: %ld***\n",nbytes);
	  fprintf(stderr,"PWD request by client (%d)\n",i);
	  fprintf(stderr,"Response: %s\n",mclients[i].pwd);
	  write(sockfd, mclients[i].pwd, MAXLINE);
	  perror("pwd write: ");
	  //memset(buf,0,sizeof(buf));
	}else if(strncmp(CMD_REMOTE_DIR,buf,(ssize_t)3) == 0){
	  fprintf(stderr,"DIR request by client (%d)\n",i);
	    
	  // You must cd into client  dir first
	  chdir(mclients[i].pwd);
	  perror("chdir dir command (client pwd):");
	  fprintf(stderr, "Virtual Server temporarily in:\n\t ***%s***\n", mclients[i].pwd);

	  // Run Dir Function
	  dirFunction(mclients, sockfd);

	  // cd back into Home Server Dir
	  chdir(serverHome);
	  perror("chdir dir command (serverHome): ");
	  fprintf(stderr, "Virtual Server back in:\n\t ***%s***\n", serverHome);

	  // Memset 
	  memset(buf,0,sizeof(buf));

	}else if (strncmp(CMD_REMOTE_CHDIR,buf,(ssize_t)2) == 0){
	  token = strtok(buf," ");
	  token = strtok(NULL," ");
	  fprintf(stderr,"Client(%d) request change directory to:\n\t %s\n", i, token);

	  // Change Dir Function
	  changeDirFunction(mclients, i, sockfd, token);

	  // Send Response
	  write(sockfd, mclients[i].pwd, MAXLINE);
	  perror("Write ch dir: ");
	  fprintf(stderr, "Sending new Client PWD ***%s***\n", mclients[i].pwd);

	  fprintf(stderr,"Client(%d) changed Server Directory (%s)\n",i, token);

	  memset(buf,0,sizeof(buf));
	}else if (strncmp(CMD_REMOTE_HOME,buf,(ssize_t)4) == 0){
	  char * homeBuff;
          token = strtok(buf," ");
          token = strtok(NULL," ");
          fprintf(stderr,"Client(%d) request change directory to:\n\t %s\n", i, token);
	    
	  //Get the Home Dir for User
	  homeBuff = getenv("HOME");
 
	  // Change Dir Function                                                                                                                            
          changeDirFunction(mclients, i, sockfd, homeBuff);
          // Send Response                                                                                                                                   

          write(sockfd, mclients[i].pwd, MAXLINE);
          perror("Write ch dir: ");
          fprintf(stderr, "Sending new Client PWD ***%s***\n", mclients[i].pwd);

          fprintf(stderr,"Client(%d) changed Server Directory (%s)\n",i, token);

          memset(buf,0,sizeof(buf));
        }else if(strstr(buf,CMD_GET_FROM_SERVER) != NULL){
	  int k, z;
	  printf("Received from client <%d>: \"%s\"\n", i , buf);
      
	  /*
	   * Parse for filename 
	   */
      
	  token = strtok(buf," ");
	  token = strtok(NULL," ");
      
	  printf("This is the token representing gets filename (Start): <%s>\n",token);
      
	  memset((buf),0,strlen((buf)));
	  printf("This is the token representing gets filename (after first memset): <%s>\n",token);

	  if((k = getFunction(token, sockfd, mclients, i)) == 0){
	    char tempBuff[MAXLINE];
	    memset(tempBuff, 0, MAXLINE);
	    strncpy(tempBuff, STR(EOT_CHAR), MAXLINE);
	        
	    //For Synching 
	    sleep(1);

	    // SEND EOF
	    z = write(sockfd, tempBuff, MAXLINE);
	    fprintf(stderr, "Final write size %d\n",z);
	    fprintf(stderr, "*** Final resp ***\n\t<%s>\n",tempBuff);
    

	    printf("getFunction returned (%d) :\n",k);
	  }else{
	    char tempBuff[MAXLINE];
	    memset(tempBuff, 0, MAXLINE);
	    strncpy(tempBuff, STR(EOT_CHAR), MAXLINE);

	    printf("getFunction returned (%d) :\n",k);
	        
	    //For Synching 
	    sleep(1);
	        
	    // SEND EOF
	    z = write(sockfd, tempBuff, MAXLINE);
	    fprintf(stderr, "Final write size %d\n",z);
	    fprintf(stderr, "*** Final resp ***\n\t<%s>\n",tempBuff);
	        
	        
	  }
	}else {
	  fprintf(stderr,"***nbytes: %ld***\n",nbytes);
	  fprintf(stderr,"UNKNOWN ***REQUEST*** from client (%d)\n",i);
	  printf("    server: echo data back to client <%s>\n", buf);
	  // Echo back to the client the received data.  Of course
	  // we SHOULD check the return value from the write().
	  write(sockfd, buf, MAXLINE);
	  perror("Unknown request write: ");
	  memset(buf,0,sizeof(buf));
	}

	if (--nready <= 0) {
	  // All the changed file desc have been checked.
	  // can exit the for loop.
	  break;
	}
      }
    }
  }
}


int findFree(clients clientsArray[]){
  int i;
  for( i=0; i<=MAX_CLIENTS; i++){
    if(clientsArray[i].avail == 0){
      return i;
    }
  }
  return -1;
}

int dirFunction(clients mArray[], int sock_fd){
  FILE *file;
  char resp[MAXLINE];

  if((file = popen(CMD_LS_POPEN,"r")) != NULL){
    while(fgets(resp,sizeof(resp),file) != NULL){
      write(sock_fd, resp, MAXLINE);
      memset(resp,0,sizeof(resp));
    }
    pclose(file);
  }else{
    fprintf(stderr,"SERVER popen error for dirFunction \n");
    return -1;
  }
  
  //Once dir listing is finished send EOT_CHAR
  strcpy(resp,STR(EOT_CHAR));
  write(sock_fd, resp, MAXLINE);
  return 0;
}

int changeDirFunction(clients client[], int pos, int sock_fd, char* dir){

  // Change into required dir for client
  chdir(dir);
  perror("chdir ");
  fprintf(stderr, "Virtual Server temporarily in:\n\t ***%s***\n", dir);

  // Update Client PWD Struct member
  getcwd(client[pos].pwd, PATH_MAX);
  perror("Client pwd update ");
  fprintf(stderr, "Client PWD updated to:\n\t ***%s***\n", client[pos].pwd);

  //Change back to Virtual Server Home
  chdir(serverHome);
  perror("Change back to Server Home: ");
  fprintf(stderr, "Virtual Server back in:\n\t ***%s***\n", serverHome);

  return 0;
}


int getFunction(char* filename, int sock_fd,clients client[],int pos){
  int fd_s;
  char resp[MAXLINE];
  int read_bytes; 

  //Cd into the client's home server
  printf("THIS IS THE CLIENT_HOME DIR <%s>\n",client[pos].pwd);
  chdir(mclients[pos].pwd);
  perror("chdir to client server_home");
  
  if(access( filename, F_OK ) != -1 ) {
    // file exists
 
    //Open (filename)
    fd_s = open(filename,O_RDONLY);
    perror("open fd_s");

    if(fd_s == -1){
      fprintf(stderr,"OPEN fd_s ERROR (GET)\n");
      return -1;
    }
    
    while((read_bytes = read(fd_s, resp, MAXLINE)) != 0){
      fprintf(stderr, "**read bytes** = \n\t%d\n",read_bytes);
      // Send via message to client
      write(sock_fd, resp, read_bytes);
      perror("Write (GET)");
      memset(resp, 0,sizeof(resp));
    }

    //Close the fd
    close(fd_s);
    perror("close fd_s");
    
    // Change server back to main server home directory                                                                                                   
    chdir(serverHome);
    perror("chdir of server_m_home");
    printf("SERVER MAIN HOME RETURNED TO  <%s>\n",serverHome);

  } else {
    // file doesn't exist
    
    // Change server back to main server home directory                                                                                                   
    chdir(serverHome);
    perror("chdir of server_m_home");
    printf("SERVER MAIN HOME RETURNED TO  <%s>\n",serverHome);

    fprintf(stderr,"NO FILE NAMED (((%s))) exsists\n",filename);
    return -1;
  }
  return 0;
}

int putFunction(char* filename, int sock_fd,clients client[],int pos){


  return 0;
}

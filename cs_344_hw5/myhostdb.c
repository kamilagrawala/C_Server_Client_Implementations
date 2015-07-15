/* Author Kamil Agrawala"
Class: CS344-001
Email: agrawalk@onid.orst.edu\
Homework Assingment #5\
Due: 03/15/15\      
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <semaphore.h> /* Semaphore include file*/
#include <sys/mman.h>  /* Smh_open */ 
#include <ctype.h>     /* Various functions */
#include <stdio.h>     /* For printf FUNCTIONS */
#include <stdlib.h>    /* For exit */
#include <unistd.h>    /* For getopt is in unistd.h & pipe system call */
#include <getopt.h>    /* Another library for getopt*/
#include <string.h>    /* General use for C strings  */
#include <errno.h>     /* For error functions */
#include <pthread.h>   /* For pthread functions */
#include <signal.h>    /* For Signal handling functions */
#include <dirent.h>    /* For opendir() command  */
#include <sys/socket.h>/* For getaddrinfo */
#include <netdb.h>     /* Additional include for getaddinfo */
#include <arpa/inet.h> /* For inet_ntop - convert IPv4 and IPv6 addresses from binary to text form */
#include <time.h>      /* For Time functionality */


/*
 * Include file hostdb.h
 */
#include "hostdb.h"


/*
 * Few Constant definations
 */

# define SHARE_WITHIN_PROCESS    0  // Share for threads within a single process.
# define SHARE_BETWEEN_PROCESSES 1  // Share between multiple processes.
# define NUM_RESOURCES           1  //Number of processes allowed through semaphore at a time.
/*
 * Function Definations
 */
int getInfo(char* mtoken,struct addrinfo mhints, struct addrinfo *maddr_list, host_row_t* mhost);
int findFreeRow(host_row_t* mhost);
int insert(char* rowName, char* ipv4,char* ipv6, host_row_t* mhost, sem_t* msem);
int search(char* rowName, host_row_t* mhost, int mloced);
int update(char* rowName, char ipv4[],char ipv6[], host_row_t* mhost, sem_t* msem);
int delete(char* rowName, host_row_t* mhost, sem_t* msem, int all);
int lockRow(char* rowName,host_row_t* mhost);
int ulockRow(char* rowName, host_row_t* mhost);
int lockDb(sem_t* msem);
int ulockDb(sem_t* msem);
int save(char* filename, host_row_t* mhost);
int load(char* filename, host_row_t* mhost, sem_t* msem);
void help(void);
void printAll(host_row_t* mhost);
void printRow(host_row_t* mhost, int val);

/*
 * General variables
 */
int i, j, z;
int res, lockedDb;
int lockedRow = -1; 

/*
 * Variable Definations and Struct host_row_t
 */
void *shared_void;
sem_t *globalSem;
host_row_t *host;
char input[PATH_MAX];
char sharedMemName[PATH_MAX];
char ipstr4[INET6_ADDRSTRLEN];
char ipstr6[INET6_ADDRSTRLEN];

/* 
 * Definations for getting ipv4/ipv6 addresses
 */ 
struct addrinfo *p;

int main(int argc, char* argv[]){
  /*
   * Main variables
   */
  int shmfd;
  char *token;
  //void *shared_void;
  // shared segment capable of storing MAX_ROWS elements
  ssize_t shared_seg_size = (sizeof(sem_t)) + (MAX_ROWS * sizeof(host_row_t));
  
  // Struct defination
  struct addrinfo hints;
  struct addrinfo *addr_list;

  //printf("sizeof sem_t %lu\n",sizeof(sem_t));
  //Umask
  umask(0);

  //Define shm Name
  SHARED_MEM_NAME(sharedMemName);
  //fprintf(stderr,"Shared Mem Name: <%s>\n",sharedMemName);

  // Create shared Memory object
  if((shmfd = shm_open(sharedMemName,O_CREAT | O_RDWR | O_EXCL, SHARED_MEM_PERMISSIONS)) >= 0){
    //fprintf(stderr, "sharedMemName opened with O_EXCL: %s\n", strerror(errno));

    // Ftruncate space in shmfd 
    ftruncate(shmfd,shared_seg_size);
    //fprintf(stderr, "ftruncate: %s", strerror(errno));
  
    //mmap space (create space)
    shared_void = mmap(NULL, shared_seg_size, PROT_WRITE | PROT_READ, MAP_SHARED, shmfd, 0);
    //fprintf(stderr, "mmap: %s\n", strerror(errno));
    
    // Memset entire mmap space (host) to NULL if shm object doesn't exsist
    memset(shared_void,0,sizeof(shared_void));
    //fprintf(stderr, "memset host: %s\n", strerror(errno));
   
    // sem_t for entire DataBase
    globalSem = (sem_t *) shared_void;
    sem_init(globalSem, SHARE_BETWEEN_PROCESSES,NUM_RESOURCES);
    //fprintf(stderr, "globalSem init: %s\n", strerror(errno));


    // Go to Structs area in memory
    host = (host_row_t *) (shared_void + sizeof(sem_t));

    // Initialize semaphores
    for(i=0; i<=MAX_ROWS ;i++){
      sem_init(&host[i].host_lock, SHARE_BETWEEN_PROCESSES,NUM_RESOURCES);
      //fprintf(stderr, "sem_init[%d]: %s\n",i, strerror(errno));
    }
    
  }else{
    //fprintf(stderr,"shm object exsists\n");
    
    shmfd = shm_open(sharedMemName,O_RDWR, SHARED_MEM_PERMISSIONS);
    //fprintf(stderr, "sharedMemName opened no O_EXCL: %s", strerror(errno));

    //mmap space (get space) 
    shared_void = (host_row_t*)mmap(NULL, shared_seg_size, PROT_WRITE | PROT_READ, MAP_SHARED, shmfd, 0);
    //fprintf(stderr, "mmap: %s", strerror(errno));

    // sem_t for entire DataBase
    globalSem = (sem_t *) shared_void;

    // Go to Structs area in memory
    host = (host_row_t *) (shared_void + sizeof(sem_t));

  }

  while(1){
    printf("%s",PROMPT);
    fgets(input,NAME_SIZE,stdin);
    //perror("fgets");
    
    //fprintf(stderr,"User Command <%s>\n",input);
    
    // Strip newline charector from fgets 
    for(i = 0; i<= strlen(input); i++){
      if(strcmp(&input[i], "\n") == 0){
	input[i] = '\0';
      }
    }

    if(strncmp(CMD_SELECT,input,(ssize_t)6) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      //fprintf(stderr,"In select\n");
      //fprintf(stderr,"token: %s \n",token);

      if(token == NULL){
	//fprintf(stderr,"All rows being printed\n");
	printAll(host);
      }else{
	//fprintf(stderr,"***** Finding & printing selected row(token) ******\n");
	i = search(token, host, 0);
	if(i >= 0){
	  printRow(host, i);
	}else{
	  fprintf(stdout,"There is no row named %s in database\n",token);
	}
      }
    }else if(strcmp(CMD_EXIT,input) == 0){
      printf("exit called\n");
      break;
    }else if(strncmp(CMD_INSERT,input,(ssize_t)6) == 0){
      //fprintf(stderr,"Insert Command <%s>\n",input);
      strtok(input," ");
      token = strtok(NULL, " ");
      if( token == NULL){
	printf("You must provide a hostname!\n");
      }else{
	//Memset hints
	memset(&hints, 0, sizeof(struct addrinfo));
	// memset addr_list
	memset(&addr_list, 0, sizeof(struct addrinfo));
	// Uset getInfo function to return relevant addresses
	getInfo(token,hints,addr_list,host);
	// Use insert function to insert the new row data
	if((z = insert(token, ipstr4, ipstr6, host, globalSem)) != -1){
	  //fprintf(stderr,"insert done\n");
	  memset(&ipstr6,0,sizeof(ipstr6));
	  memset(&ipstr4,0,sizeof(ipstr4));
	  printRow(host, z);
	} else{
	  //fprintf(stderr,"\ninsert function returned %d\n",z);
	}
      }
    }else if(strncmp(input,"clear",(ssize_t)5) == 0){
      printf("\e[1;1H\e[2J");
    }else if(strncmp(input,CMD_UPDATE,(ssize_t)6) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      if( token == NULL){
        printf("You must provide a hostname!\n");
      }else{
	// Uset getInfo function to return relevant addresses
	getInfo(token,hints,addr_list,host);
	// Update Command
	if((z = update(token, ipstr4, ipstr6, host, globalSem)) == 0){
	  //done
	  memset(&ipstr6,0,sizeof(ipstr6));
	  memset(&ipstr4,0,sizeof(ipstr4));
	}else{
	  //fprintf(stderr,"No row <%s> found\n",token);
	}
      }
    }else if(strncmp(input,CMD_DELETE,(ssize_t)6) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      if(token == NULL){
	printf("Deleting ***All*** Rows!\n");
	if((z = delete(token, host, globalSem, 1)) == 0){
	  printf("Delete ***All*** done\n");
	}else if (z == -1){
	  fprintf(stderr,"Delete all was NOT completed\n");
	}
      }else{
	if((z = delete(token, host, globalSem, 0)) == 0){
	  //fprintf(stderr,"Row %s was deleted\n",token);
	}else if (z == -1){
	  //fprintf(stderr,"Row %s was NOT deleted\n",token);
	}
      }
    }else if(strncmp(input,CMD_DROP_DB,(ssize_t)13) == 0){
      // Start to clean things up.
      res = munmap(shared_void, shared_seg_size);
      if (res < 0) {
	//fprintf(stderr, "Failed un-mapping shared memory object: %s\n", strerror(errno));
      }
      
      res = shm_unlink(sharedMemName);
      if(res < 0){
	//fprintf(stderr, "Failed un-linking shared memory object: %s", strerror(errno));
      }

      // Notice that the function to close a shared memory object is our
      //   old friend close(), but to close a named semaphore, we have to
      //   call a different function.
      res = close(shmfd);
      if (res < 0) {
	//fprintf(stderr,"Failed closing shared memory object: %s\n" , strerror(errno));
      }
      fprintf(stdout,"Database dropped\n");
    }else if(strncmp(input,CMD_LOCK_ROW,(ssize_t)8) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      if(token == NULL){
	fprintf(stdout,"You must enter a Row Name\n");
      }else{
	lockRow(token,host);
      }
    }else if(strncmp(input,CMD_UNLOCK_ROW,(ssize_t)10) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      if(token == NULL){
	fprintf(stdout,"You must enter a Row Name\n");
      }else{
	//fprintf(stderr,"unlock row %s\n",token);
	ulockRow(token,host);
      }
    }else if(strncmp(input,CMD_LOCK_DB,(ssize_t)7) == 0){
      lockDb(globalSem);
    }else if(strncmp(input,CMD_UNLOCK_DB,(ssize_t)7) == 0){
      ulockDb(globalSem);
    }else if(strncmp(input,CMD_SAVE,(ssize_t)4) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      if(token == NULL){
        fprintf(stdout,"You must enter a File Name\n");
      }else{
	save(token,host);
	fprintf(stdout,"Host names saved in file: %s\n", token);
      }
    }else if(strncmp(input,CMD_LOAD,(ssize_t)4) == 0){
      strtok(input," ");
      token = strtok(NULL, " ");
      if(token == NULL){
        fprintf(stdout,"You must enter a File Name\n");
      }else{
        load(token, host, globalSem);
        fprintf(stdout,"Host names from file %s were loaded into database\n", token);
      }
    }else if (strncmp(input,CMD_HELP,(ssize_t)4) == 0){
      help();
    }else{
      printf("Command not understood <%s>\n",input);
    }
  } 
  return 0;
}


int getInfo(char* mtoken,struct addrinfo mhints, struct addrinfo *maddr_list, host_row_t* mhost){
  int s;
  // Set mhints values
  mhints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  mhints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  mhints.ai_flags = 0;
  mhints.ai_protocol = 0;          /* Any protocol */
  
  s = getaddrinfo(mtoken,NULL,&mhints,&maddr_list);
  //fprintf(stderr,"getaddrinfo: %s\n" , strerror(errno));

  if (s != 0) {
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }
  
  // Loop through the list of returned addresses.                                                                                                           
  for(p = maddr_list; p != NULL; p = p->ai_next) {
    void *addr;
    char *ipver;
    
    // Get the pointer to the address itself,                                                                                                               
    //   different fields in IPv4 and IPv6:                                                                                                                 
    if (p->ai_family == AF_INET) { // IPv4                                                                                                                  
      // Notice that after we determine the address family, we have to                                                                                      
      // cast to a new type and that the types are different between                                                                                        
      // IPv4 and IPv6.                                                                                                                                     
      struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    }
    else { // IPv6                                                                                                                                          
      // We are assuming that AF_INET and AF_INET6 will be the                                                                                              
      // only return types.                                                                                                                                 
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }
    
    // convert the IP to a string and print it:                                                                                                             
    if(strcmp(ipver,"IPv4") == 0){
      inet_ntop(p->ai_family, addr, ipstr4, sizeof ipstr4);
      // Print no-matter what
      //printf("\t%s: %s\n", ipver, ipstr4);
   
    }else if(strcmp(ipver,"IPv6") == 0){
      inet_ntop(p->ai_family, addr, ipstr6, sizeof ipstr6);
      // Print no-matter what
      //printf("\t(Before insert) %s: %s\n", ipver, ipstr6);
    }
  }
  
  printf("\n");
  
  //   Call this to free up the memory allocated to                                                                                                  
  //   the list from the call to getaddrinfo().                                                                                                             
  freeaddrinfo(maddr_list);
  //fprintf(stderr, "freeaddrinfo: %s\n", strerror(errno));

  return 0;
}

int findFreeRow(host_row_t* mhost){
  int k;
  
  for(k = 0; k <= MAX_ROWS; k++){
    if(mhost[k].in_use == '\0'){
      return k;
    }
  }

  return -1;
}


int search(char* rowName, host_row_t* mhost, int mlocked){
  int k;
  char timeString[PATH_MAX];
  struct tm *ts;
  
  for(k = 0; k <= MAX_ROWS; k++){
    if(mlocked ==  0){
      // Lock while searching on that element
      res = sem_wait(&mhost[k].host_lock);
      if (res < 0) {
	//fprintf(stderr, "sem_wait mhost[%d].host_lock: %s\n", k, strerror(errno));
	exit(EXIT_FAILURE);
      }
      //fprintf(stderr, "  ++ Got the semaphore lock\n");
      
      // //fprintf(stderr, "%d \n", __LINE__);
      //fprintf(stderr, "sem_wait in search: %s\n",strerror(errno));
    }
    if(strcmp(mhost[k].host_name,rowName) == 0){
      //fprintf(stderr, "%d \n", __LINE__);
      if (res < 0) {
	//fprintf(stderr, "sem_wait mhost[%d].host_lock: %s\n", k, strerror(errno));
	exit(EXIT_FAILURE);
      }
      
      ts = localtime(&mhost[k].time_when_fetched);
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %Z", ts);

      //fprintf(stdout,"Name: %-25s\n\t IPv4: %s\n\t Ipv6: %s\n\t Time fetched: %s\n",mhost[k].host_name,mhost[k].host_address_ipv4,mhost[k].host_address_ipv6,timeString);      

      if(mlocked == 0){
	//Unlock Semaphore 
	res = sem_post(&mhost[k].host_lock);
	//fprintf(stderr, "sem_post in search: %s\n",strerror(errno));
	//fprintf(stderr, "%d \n", __LINE__);
	if (res < 0) {
	  //fprintf(stderr, "sem_post mhost[%d].host_lock: %s\n", k, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "  -- Unlocked the semaphore\n");
      }
      return k;
    }else{
      if(mlocked ==  0){
	//Unlock Semaphore 
	res = sem_post(&mhost[k].host_lock);
	//fprintf(stderr, "sem_post (2)in search: %s\n",strerror(errno));
	////fprintf(stderr, "%d \n", __LINE__);
	if (res < 0) {
	  //fprintf(stderr, "sem_post mhost[%d].host_lock: %s\n", k, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "  -- Unlocked the semaphore\n");
      }
    }
  }
  
  //fprintf(stderr,"Row with hostname <%s> not found!\n",rowName);
  
  return -1;
}

int insert(char* rowName, char ipv4[],char ipv6[], host_row_t* mhost, sem_t* msem){
  int k;
  int val;
  struct tm *ts;
  char timeString[PATH_MAX];
  
  if(lockedDb != 1){
    if((val = search(rowName,mhost,0)) == -1){
      // rowName doesn't exist
      k = findFreeRow(mhost);
    
      //fprintf(stderr, "  ++ Got the semaphore lock (ENTIRE DB)\n");

      //Lock DB
      res = sem_wait(&msem[0]);
      if (res < 0) {
	//fprintf(stderr, "sem_wait globalSem: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      //fprintf(stderr, "  ++ Got the semaphore lock DB\n");

      // Lock row
      res = sem_wait(&mhost[k].host_lock);
      if (res < 0) {
	//fprintf(stderr, "sem_wait mhost[%d].host_lock: %s\n", k ,strerror(errno));
	exit(EXIT_FAILURE);
      }
      //fprintf(stderr, "  ++ Got the semaphore lock\n");
    
      /* 
       * strcpy functions
       */
      // Hostname Copy
      strcpy(mhost[k].host_name,rowName);
      // ipv4 copy
      sprintf(mhost[k].host_address_ipv4,"%s",ipv4);
      // ipv6 copy 
      sprintf(mhost[k].host_address_ipv6,"%s",ipv6);
      // Time fetched copy
      time(&mhost[k].time_when_fetched);
      ts = localtime(&mhost[k].time_when_fetched);
      
      // Print formatted String
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %z %Z %a", ts);
      //printf("%s \n",timeString);
      // Set  in_use 
      mhost[k].in_use = 1;
      
    }else{
      //fprintf(stderr,"%s *****exsits***** @ positon %d\n",rowName,val);
      
      // Print formatted String
      ts = localtime(&mhost[val].time_when_fetched);
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %Z", ts);
      //fprintf(stdout,"Name: %-25s\n\t IPv4: %s\n\t Ipv6: %s\n\t Time fetched: %s\n",mhost[val].host_name,mhost[val].host_address_ipv4,mhost[val].host_address_ipv6,timeString);

      return -1;
    }

    //Unlock DB
    res = sem_post(&msem[0]);
    if (res < 0) {
      //fprintf(stderr, "sem_post globalSem: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    //fprintf(stderr, "  -- Got the semaphore unlock DB\n");
    
    //Unlock Semaphore
    res = sem_post(&mhost[k].host_lock);
    if (res < 0) {
      //fprintf(stderr, "sem_post mhost[%d].host_lock: %s\n",k, strerror(errno));
      exit(EXIT_FAILURE);
    }
    //fprintf(stderr, "  -- Unlocked the semaphore\n");
    
    return k;
  }else{
    fprintf(stdout, "  ***SORRY database is LOCKED***\n");
  }
  return -1;
}

int update(char* rowName, char ipv4[],char ipv6[], host_row_t* mhost, sem_t* msem){
  int val;
  struct tm *ts;
  char timeString[PATH_MAX];
  
  if(lockedDb != 1){
    if((val = search(rowName,mhost,0)) >= 0){
      // rowName exists
      
      //Lock DB Semaphore
      res = sem_wait(&msem[0]);
      if (res < 0) {
	//fprintf(stderr, "sem_wait globalSem: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      //fprintf(stderr, "  ++ Got the semaphore lock DB\n");
      

      // Lock row when updating
      res = sem_wait(&mhost[val].host_lock);
      if (res < 0) {
	//fprintf(stderr, "sem_wait[%d].host_lock: %s\n",val, strerror(errno));
	exit(EXIT_FAILURE);
      }
      
      //fprintf(stderr, "  ++ Got the semaphore lock\n");
      /* 
       * strcpy functions
       */
    
      // Hostname Copy
      strcpy(mhost[val].host_name,rowName);
      // ipv4 copy
      sprintf(mhost[val].host_address_ipv4,"%s",ipv4);
      // ipv6 copy 
      sprintf(mhost[val].host_address_ipv6,"%s",ipv6);
      // Time fetched copy
      time(&mhost[val].time_when_fetched);
      ts = localtime(&mhost[val].time_when_fetched);
      
      // Print formatted String
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %z %Z %a", ts);
      //fprintf(stderr,"%s \n",timeString);
      // Set  in_use 
      mhost[val].in_use = 1;


      //Unlock DB Semaphore
      res = sem_post(&msem[0]);
      if (res < 0) {
	//fprintf(stderr,"sem_post mhost[%d].host_lock: %s\n" ,val, strerror(errno));
	exit(EXIT_FAILURE);
      }
      //fprintf(stderr, "  -- Got the semaphore unlock DB\n");
      
      //Unlock Semaphore
      res = sem_post(&mhost[val].host_lock);
      if (res < 0) {
	//fprintf(stderr,"sem_post mhost[%d].host_lock: %s\n" ,val, strerror(errno));
	exit(EXIT_FAILURE);
      }
      
      //fprintf(stderr, "  -- Unlocked the semaphore\n");
      
      return 0;
    }
  }else{
    fprintf(stdout, "  ***SORRY database is LOCKED***\n");
  }
  return -1;
}

int delete(char* rowName,  host_row_t* mhost, sem_t* msem, int all){
  int k; 
  int f;
  if(lockedDb != 1){
    if (all == 0){
      if((k = search(rowName,mhost,0)) >= 0){
	//Lock DB Semaphore
	res = sem_wait(&msem[0]);
	if (res < 0) {
	  //fprintf(stderr,"sem_wait globalSem %s\n", strerror(errno));
	  exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "  ++ Got the semaphore lock DB\n");
	// Lock Row
	res = sem_wait(&mhost[k].host_lock);
	if (res < 0) {
	  //fprintf(stderr,"sem_wait mhost[%d].host_lock: %s\n" ,k, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "  ++ Got the semaphore lock (Delete)\n");

	//Delete selected
	memset(&mhost[k].in_use,0,sizeof(&mhost[k].in_use));
	memset(&mhost[k].host_name,0,sizeof(&mhost[i].host_name));
	memset(&mhost[k].host_address_ipv4,0,sizeof(&mhost[k].host_address_ipv4));
	memset(&mhost[k].host_address_ipv6,0,sizeof(&mhost[k].host_address_ipv6));
	memset(&mhost[k].time_when_fetched,0,sizeof(&mhost[k].time_when_fetched));

	// Initialize sem_t for mhost[k] again
	//sem_init(&mhost[k].host_lock, SHARE_BETWEEN_PROCESSES,NUM_RESOURCES);
	//fprintf(stderr, "  Delete sem_init re-initialization: %s\n",strerror(errno));
	
	//Unlock DB Semaphore
	res = sem_post(&msem[0]);
	if (res < 0) {
	  //fprintf(stderr,"sem_post mhost[%d].host_lock: %s\n" , k, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "  -- Got the semaphore unlock DB\n");
	
	// Unlock Row
	res = sem_post(&mhost[k].host_lock);
	if (res < 0) {
	  //fprintf(stderr,"sem_post mhost[%d].host_lock: %s\n",k, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	
	//fprintf(stderr, "  -- Unlocked the semaphore (Delete)\n");
	return 0;
      }
    }else if(all == 1){
      // Delete all Rows
      
      //Lock Entire Database
      res = sem_wait(&msem[0]);
      sem_getvalue(&msem[0],&f);
      //fprintf(stderr,"sem value (wait) DB: %d\n",f);

      for(i=0; i<=MAX_ROWS ;i++){
	// Lock Row
	res = sem_wait(&mhost[i].host_lock);
	sem_getvalue(&msem[0],&f);
	//fprintf(stderr,"sem value (wait): %d\n",f);

	if (res < 0) {
	  //fprintf(stderr,"sem_wait mhost[%d].host_lock: %s\n",k, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	
	memset(&mhost[i].in_use,0,sizeof(&mhost[i].in_use));
	memset(&mhost[i].host_name,0,sizeof(&mhost[i].host_name));
	memset(&mhost[i].host_address_ipv4,0,sizeof(&mhost[i].host_address_ipv4));
	memset(&mhost[i].host_address_ipv6,0,sizeof(&mhost[i].host_address_ipv6));
	memset(&mhost[i].time_when_fetched,0,sizeof(&mhost[i].time_when_fetched));

	// Unlock Row
	res = sem_post(&mhost[i].host_lock);
	sem_getvalue(&msem[0],&f);
	//fprintf(stderr,"sem value (post): %d\n",f);

	if (res < 0) {
	  fprintf(stderr,"sem_post mhost[%d].host_lock: %s\n", i, strerror(errno));
	  exit(EXIT_FAILURE);
	}
	
      }
    }

    //Unlock DB Semaphore
    res = sem_post(&msem[0]);
    sem_getvalue(&msem[0],&f);
    //fprintf(stderr,"sem value (post) DB: %d\n",f);
	
    if (res < 0) {
      //fprintf(stderr,"sem_post globalSem: %s\n" , strerror(errno));
      exit(EXIT_FAILURE);
    }
    return 0;
  }else{
    fprintf(stdout, "  ***SORRY database is LOCKED***\n");
  }
  return -1;
}

int lockRow(char* rowName, host_row_t* mhost){
  int k;
  if((k = search(rowName, mhost,0)) >= 0){
    // Lock Row                                                                                                               
    res = sem_wait(&mhost[k].host_lock);
    //fprintf(stderr,"sem_wait in lockRow function: %s\n", strerror(errno));
    if (res < 0) {
      //fprintf(stderr,"sem_wait mhost[%d].host_lock: %s\n",k,strerror(errno));
      exit(EXIT_FAILURE);
    }
    //fprintf(stderr,"Row %s has been ***LOCKED*** by the lockRow function\n",rowName);
  }else{
    fprintf(stdout,"There is no row named %s in the database\n",rowName);
  }
  return 0;
}

int ulockRow(char* rowName, host_row_t* mhost){
  int k;
  ////fprintf(stderr, "%d \n", __LINE__);
  k = search(rowName, mhost,1);
  //fprintf(stderr,"k is %d\n",k);
    if(k != -1){
    // Unlock specified row                                                                                                                                
    res = sem_post(&mhost[k].host_lock);
    //fprintf(stderr, "%d \n", __LINE__);
    if (res < 0) {
      //fprintf(stderr,"sem_post mhost[%d].host_lock: %s\n" ,k, strerror(errno));
      exit(EXIT_FAILURE);
    }
    //fprintf(stderr, "  -- Unlocked the semaphore\n");
    fprintf(stdout,"Row %s has been ***UN-LOCKED***\n",rowName);
  }else{
    fprintf(stdout,"There is no row named %s in the database\n",rowName);
    return -1;
  }
  return 0;
}

int lockDb(sem_t* msem){
  int f;

  if(lockedDb != 1){
    //fprintf(stderr, "%d \n", __LINE__);

    res = sem_wait(&msem[0]);
    //fprintf(stderr,"sem_wait msem: %s\n" , strerror(errno));
  
    lockedDb = 1;
  
    sem_getvalue(&msem[0],&f);
    //fprintf(stderr,"sem value: %d\n",f);

    if (res < 0) {
      //fprintf(stderr, "sem_wait msem: %s\n",strerror(errno));
      exit(EXIT_FAILURE);
    }
  
    //fprintf(stderr, "  ++ Got the semaphore lock (ENTIRE DB)\n");
  }else{
    fprintf(stdout, "  You have already ***LOCKED*** the Database!!!(ENTIRE DB)\n");
  }
  return 0;
}


int ulockDb(sem_t* msem){
  ////fprintf(stderr, "%d \n", __LINE__);
  if(lockedDb == 1){
    res = sem_post(&msem[0]);
    if (res < 0) {
      //fprintf(stderr, "sem_post msem: %s\n",strerror(errno));
      exit(EXIT_FAILURE);
    }else{
      //fprintf(stderr, "  -- Unlocked the semaphore (ENTIRE DB)\n");
    }
    
    lockedDb = 0;
  }else{
    //fprintf(stderr,"Database is already locally ***UN-LOCKED*** \n");
  }
  return 0;
}

int save(char* filename, host_row_t* mhost){
  int k;
  int fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0644);
  
  for(k = 0; k <= MAX_ROWS; k++){
    // Lock Row                                                                                                                                                                                               
    res = sem_wait(&mhost[k].host_lock);
    //sem_getvalue(&msem[0],&f);
    //fprintf(stderr,"sem value (wait): %d\n",f);                                                                                                                                                             

    if (res < 0) {
      //fprintf(stderr,"sem_wait mhost[%d].host_lock: %s\n",k, strerror(errno));                                                                                                                              
      exit(EXIT_FAILURE);
    }
    if(mhost[k].in_use != '\0'){
      write(fd, &mhost[k].host_name,strlen(mhost[k].host_name));
      write(fd,0, (ssize_t)1);
      write(fd,"\n", (ssize_t)1);
    }
    // Un-lock Row
    res = sem_post(&mhost[k].host_lock);
    //sem_getvalue(&msem[0],&f);
    //fprintf(stderr,"sem value (wait): %d\n",f);
    if (res < 0) {
      //fprintf(stderr,"sem_wait mhost[%d].host_lock: %s\n",k, strerror(errno));                                                                                                                                
      exit(EXIT_FAILURE);
    }
  }
  
  close(fd);

  return 0;
}

int load(char* filename, host_row_t* mhost, sem_t* msem){
  FILE *fp = fopen(filename,"r"); 
  ssize_t read_bytes;
  size_t len = 0;
  char *name = NULL;
  struct addrinfo mhints;
  struct addrinfo * maddr_list;
  
  // Intial ipstr4 and ipstr6 memset
  memset(&ipstr6,0,sizeof(ipstr6));
  memset(&ipstr4,0,sizeof(ipstr4));

  while ((read_bytes = getline(&name, &len, fp)) != -1) {
    //fprintf(stderr, "Retrieved line of length %zu :\n", read_bytes);
    //fprintf(stderr, "%s", name);
    for(i = 0; i<= strlen(name); i++){
      if(name[i] == '\n'){
	name[i] = 0;
      }
    }

    //Memset hints                                                                                                                                                                                            
    memset(&mhints, 0, sizeof(struct addrinfo));
    // memset addr_list                                                                                                                                                                                       
    memset(&maddr_list, 0, sizeof(struct addrinfo));
    // Get the info
    getInfo(name , mhints, maddr_list, mhost); 
    insert(name, ipstr4, ipstr6, mhost, msem);
    //Memset ipstr4 & ipstr6 for next round
    memset(&ipstr6,0,sizeof(ipstr6));
    memset(&ipstr4,0,sizeof(ipstr4));
  }

  fclose(fp);
  if(name)
    free(name);

  return 0;

}

void help(void){
  fprintf(stdout, "Available commands:\n");
  fprintf(stdout, " help             : print this help text\n");
  fprintf(stdout, " exit             : exit the application\n");
  fprintf(stdout, " select [<host>]  : display hostname and address(es)\n");
  fprintf(stdout, " insert <host>    : insert an address given a name\n");
  fprintf(stdout, " update <host>    : refresh <name> in database\n");
  fprintf(stdout, " delete [<host>]  : remove one or all hosts from the database\n");
  fprintf(stdout, " count            : count the number of rows in db - EXTRA CREDIT\n");
  fprintf(stdout, " drop_database    : drop the database, unlink shared memory\n");
  fprintf(stdout, " save <file>      : save all names to a file\n");
  fprintf(stdout, " load <file>      : load all names from a file and insert addresses\n");
  fprintf(stdout, " lock_db          : lock db\n");
  fprintf(stdout, " unlock_db        : unlock db\n");
  fprintf(stdout, " lock_row <host>  : lock row <name>\n");
  fprintf(stdout, " unlock_row       : unlock the locked row\n");
  fprintf(stdout, " locks            : show all locks - EXTRA CREDIT\n");
}


void printAll(host_row_t* mhost){
  int k;
  int f = 0;
  char timeString[PATH_MAX];
  struct tm *ts;

  for(k = 0; k <= MAX_ROWS; k++){
    if(mhost[k].in_use != '\0'){
      f++;
      ts = localtime(&mhost[k].time_when_fetched);
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %Z", ts);

      fprintf(stdout,"Name: %-25s\n\t IPv4: %s\n\t Ipv6: %s\n\t Time fetched: %s\n",mhost[k].host_name,mhost[k].host_address_ipv4,mhost[k].host_address_ipv6,timeString);
    }
  }
  fprintf(stdout,"Rows selected: %d\n",f);
}


void printRow(host_row_t* mhost, int val){
  char timeString[PATH_MAX];
  struct tm *ts;
  
  ts = localtime(&mhost[val].time_when_fetched);
  
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %Z", ts);
  
  fprintf(stdout,"Name: %-25s\n\t IPv4: %s\n\t Ipv6: %s\n\t Time fetched: %s\n",mhost[val].host_name,mhost[val].host_address_ipv4,mhost[val].host_address_ipv6,timeString);
  
}

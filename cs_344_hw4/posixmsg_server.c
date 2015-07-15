/* Author Kamil Agrawala
* Class: CS344-001       
* Homework Assingment #4 
* Email: agrawalk@onid.orst.edu
* Due: 03/02/15 (with extra credit)                                           
* Program: posixmsg_server.c                                                  
* Citations:                                                                  
 */


/* 
 * Header File Inclusions
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <ctype.h>     /* Various functions */
#include <mqueue.h>    /* For message queues */
#include <stdio.h>     /* For printf FUNCTIONS */
#include <stdlib.h>    /* For exit */
#include <unistd.h>    /* For getopt is in unistd.h & pipe system call */
#include <getopt.h>    /* Another library for getopt*/
#include <string.h>    /* General use for C strings  */
#include <errno.h>     /* For error functions */
#include <pthread.h>   /* For pthread functions */
#include <signal.h>    /* For Signal handling functions */
#include <dirent.h>    /* For opendir() command  */

#include "posixmsg.h"

/*
 * Variable definations
 */
#define MAX_DIR_SIZE PATH_MAX

int j, k, z, rc;
long t;
char messageQueueName[100];
char mclientReaderName[100];
char mclientWriterName[100];
mqd_t mq;
char pidBuff[100];
char dirName[PATH_MAX];
FILE* file;

pthread_mutex_t mutex;

/*
 *  Messages Queue Variables;
 */
struct mq_attr attr;
message_t msg;
message_t msgPairs[MAX_CLIENTS];

typedef struct myThread {
  //Thread Home DIR
  char server_thread_home[MAX_DIR_SIZE];
  // List of associated mq_w's
  mqd_t mq_w[MAX_CLIENTS];
  // List of associated mq_r's
  mqd_t mq_r[MAX_CLIENTS];
  // Threads lists
  pthread_t mthread[MAX_CLIENTS];
  // Thread DataStructure
  char* threadMap[MAX_CLIENTS];
  // Available ?
  int avail;
} myThread;


myThread mytArray[MAX_CLIENTS + 1];

/*
 * Thread Variables
 */
pthread_attr_t pattr;

/*
 * Function Definations
 */
intptr_t findNextFree(myThread mytArray[]);
void *threadHandler(void* threadNumber);
int changeDir(char* tempBufff);
int getFunction(char* filename,myThread mytArray[], void* threadNum);
int putFunction(char* filename,myThread mytArray[], void* threadNum);
int dirFunction(myThread mArray[],void* threadNum);

/*
 * Signal Handler
 */
void sigIntHandler(int);

int main(int argc, char* argv[]){
  intptr_t i = 0;
  
  //unmask permissions
  umask(0);

  /*
   * Signal handler
   */
  signal(SIGINT, sigIntHandler);
  
  /*
   * Set Attributes
   */
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(message_t); // Specify the maximum size of a message.                                                                        
  attr.mq_curmsgs = 0;
  
  CREATE_SERVER_QUEUE_NAME(messageQueueName);
  
  fprintf(stderr,"Name of message queue:\n %s\n",messageQueueName);
  
  fprintf(stderr,"\nCreating message queue:\n");  
  
  mq = mq_open(messageQueueName, O_CREAT | O_RDONLY,QUEUE_PERMISSIONS,&attr);
  
  perror("mq Open");

  // Set All threads indexes as available
  for(k=0; k < MAX_CLIENTS; k++){
    mytArray[k].avail = 0;
    strcpy(mytArray[k].server_thread_home,"null");
  }
  
  // While loop to read PIDs form client

  while(1){
    ssize_t bytes_read;
    char strNumber[25];
    // receive the message
    bytes_read =  mq_receive(mq, (char *) &msg, sizeof(message_t), 0);    
    
    i = findNextFree(mytArray);
    fprintf(stderr, "Free mytArray index found @ <%ld>\n",i);
    if( i >= 0){
      // Make mytArray[i] not availale
      mytArray[i].avail = -1;
      // Get the pid for the reader/writer
      sprintf(strNumber,"%s",msg.command/*Should be the getpid*/);
      CREATE_CLIENT_WRITER_NAME(mclientWriterName,atoi(strNumber));
      CREATE_CLIENT_READER_NAME(mclientReaderName,atoi(strNumber));

      pthread_create(&mytArray[i].mthread[i],NULL,threadHandler,(void *)i);
      perror("pthread_create");

      mytArray[i].threadMap[i] = mclientWriterName;
      printf("The message queue for thread (%ld) is <%s> \n", i,mytArray[i].threadMap[(int)i]);
    }else{
      printf("MAX Threads Created!(BREAKING)\n");
      break;
    }
  }

  sigIntHandler(1);
  return (EXIT_SUCCESS);
}


void *threadHandler(void* threadNumber){

  ssize_t bytes_read_thread;
  char server_main_home[MAX_DIR_SIZE];
  //ssize_t bytes_sent_thread;
  char* tempBuff;
 
  //Detach the thread for clean exit.
  pthread_detach(pthread_self());
  perror("pthread_detach");

  mytArray[(intptr_t) threadNumber].mq_w[(intptr_t) threadNumber] = mq_open(mclientWriterName, O_RDONLY,QUEUE_PERMISSIONS,&attr);
  perror("open mq_w");

  mytArray[(intptr_t) threadNumber].mq_r[(intptr_t) threadNumber] = mq_open(mclientReaderName, O_WRONLY,QUEUE_PERMISSIONS,&attr);
  perror("open mq_r");


  do{
    // Thread specific Read message
    bytes_read_thread = mq_receive(mytArray[(intptr_t)threadNumber].mq_w[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
    perror("mq_receive mq_w");

    // Change dir listing of thread
    if(strstr(msgPairs[(intptr_t) threadNumber].command,"cd ") != NULL){
      printf("Received from thread <%ld>: \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);
      fprintf(stderr,"Request to change thread dir to: <%s>\n",msgPairs[(intptr_t) threadNumber].command);
      // Change "working" directory of client to think it is in <dir> provided 
      //Update the struct myThreadDir parameter
      strtok(msgPairs[(intptr_t) threadNumber].command," ");
      tempBuff = strtok(NULL," ");
      printf("This is the tempBuff representing dir: <%s>\n",tempBuff);
    
      // Get CWD of Server main home to return to later
      getcwd(server_main_home,MAX_DIR_SIZE);
      perror("getcwd of server_main_home");
      printf("THIS IS THE SERVER_HOME DIR <%s>\n",server_main_home);
      
      // CH dir thread server_thread_home
      if(strcmp(mytArray[(intptr_t) threadNumber].server_thread_home,"null") != 0){
	chdir(mytArray[(intptr_t) threadNumber].server_thread_home);
	perror("chdir to server_thread_home (local thread home)");
	printf("THIS IS the THREAD (%ld) HOME: <%s>\n",(intptr_t)threadNumber,mytArray[(intptr_t) threadNumber].server_thread_home);
      }
      
      // CH dir into th cd <dir> directory
      chdir(tempBuff);
      perror("chdir to tempBuf (which is dir in ch <dir>");
      printf("THIS IS DIR <%s>\n",tempBuff);
      
      // Get the CWD of <dir> directory and put it into the sever_thread_home
      getcwd(mytArray[(intptr_t) threadNumber].server_thread_home,MAX_DIR_SIZE);
      perror("getcwd of server_home");
      printf("THIS IS PATH for DIR for thread (%ld): <%s>\n",(intptr_t)threadNumber,mytArray[(intptr_t) threadNumber].server_thread_home);
      
      // Change server back to main server home directory
      chdir(server_main_home);
      perror("chdir of server_main_home");
      printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_main_home);
      
      //strcpy(mytArray[(intptr_t) threadNumber].myThreadDir,mytArray[(intptr_t) threadNumber].server_thread_home);
      strcpy(msgPairs[(intptr_t) threadNumber].payload, mytArray[(intptr_t) threadNumber].server_thread_home);
      
      fprintf(stderr,"Sending Response <<<<%s>>>>\n",msgPairs[(intptr_t) threadNumber].payload);
      mq_send(mytArray[(intptr_t)threadNumber].mq_r[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
      perror("mq_send (cd dir)");
      
    }else if(strcmp(msgPairs[(intptr_t) threadNumber].command,CMD_REMOTE_PWD) == 0){
	printf("Received from thread <%ld>: \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);

	fprintf(stderr,"THREAD HOME VALUE *****<%s>*****: \n",mytArray[(intptr_t) threadNumber].server_thread_home);

	if(strcmp(mytArray[(intptr_t) threadNumber].server_thread_home,"null") != 0){
	  // Thread has a different home then default server_main_home 
	  fprintf(stderr,"**DIFFERENT** THREAD HOME <%s>: \n",mytArray[(intptr_t) threadNumber].server_thread_home);
	  strcpy(msgPairs[(intptr_t) threadNumber].payload, mytArray[(intptr_t) threadNumber].server_thread_home);

	  fprintf(stderr,"Sending Response <<<<%s>>>>\n",msgPairs[(intptr_t) threadNumber].payload);
	  mq_send(mytArray[(intptr_t)threadNumber].mq_r[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
	  perror("mq_send (pwd)");
		
	}else{
	  // Get CWD of Server main home as this is the home for threaded client
	  getcwd(server_main_home,MAX_DIR_SIZE);
	  perror("getcwd (for pwd) of server_main_home");
	  fprintf(stderr,"DEFAULT THREAD HOME <%s>: \n",server_main_home);
	  strcpy(msgPairs[(intptr_t) threadNumber].payload, server_main_home);
	  
	  fprintf(stderr,"Sending Response <<<<%s>>>>\n",msgPairs[(intptr_t) threadNumber].payload);
	  mq_send(mytArray[(intptr_t)threadNumber].mq_r[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
	  perror("mq_send (pwd)");

	}
    }else if(strstr(msgPairs[(intptr_t) threadNumber].command,CMD_GET) != NULL){
      printf("Received from thread <%ld>: \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);
      
      /*
       * Parse for filename 
       */
      
      tempBuff = strtok(msgPairs[(intptr_t) threadNumber].command," ");
      tempBuff = strtok(NULL," ");
      
      printf("This is the tempBuff representing gets filename (Start): <%s>\n",tempBuff);
      
      memset((msgPairs[(intptr_t) threadNumber].command),0,strlen((msgPairs[(intptr_t) threadNumber].command)));
      printf("This is the tempBuff representing gets filename (after first memset): <%s>\n",tempBuff);

      if((k = getFunction(tempBuff, mytArray,threadNumber)) == 0){
	//done
	printf("getFunction returned (%d) :\n",k);
      }else{
	printf("getFunction returned (%d) :\n",k);
	msgPairs[(intptr_t) threadNumber].message_type = MESSAGE_TYPE_ERROR;
	fprintf(stderr,"<<<<%d>>>>\n",msgPairs[(intptr_t) threadNumber].message_type);
	mq_send(mytArray[(intptr_t)threadNumber].mq_r[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
      }
    }else if(strcmp(msgPairs[(intptr_t) threadNumber].command,CMD_REMOTE_HOME) == 0){
      printf("Received from thread <%ld>: \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);
      tempBuff = getenv("HOME");
      perror("getenv HOME");
      fprintf(stderr,"Request to change thread dir to home dir <%s>\n",tempBuff);
      printf("This is the tempBuff representing HOME DIR: <%s>\n",tempBuff);
      
      if((k = changeDir(tempBuff)) == 0){
	
	// Get CWD of Server main home to return to later
	getcwd(server_main_home,MAX_DIR_SIZE);
	perror("getcwd of server_main_home");
	printf("THIS IS THE SERVER_HOME DIR <%s>\n",server_main_home);
	
	// CH dir thread server_thread_home
	if(strcmp(mytArray[(intptr_t) threadNumber].server_thread_home,"null") != 0){
	  chdir(mytArray[(intptr_t) threadNumber].server_thread_home);
	  perror("chdir to server_thread_home (local thread home)");
	  printf("THIS IS the THREAD (%ld) HOME: <%s>\n",(intptr_t)threadNumber,mytArray[(intptr_t) threadNumber].server_thread_home);
	}

	// CH dir into the HOME directory
	chdir(tempBuff);
	perror("chdir to tempBuf (which is HOME Dir");
	printf("THIS IS DIR <%s>\n",tempBuff);

	// Get the CWD of HOME directory and put it into the sever_thread_home
	getcwd(mytArray[(intptr_t) threadNumber].server_thread_home,MAX_DIR_SIZE);
	perror("getcwd of server_home");
	printf("THIS IS PATH for DIR for thread (%ld): <%s>\n",(intptr_t)threadNumber,mytArray[(intptr_t) threadNumber].server_thread_home);

	// Change server back to main server home directory
	chdir(server_main_home);
	perror("chdir of server_main_home");
	printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_main_home);

	strcpy(msgPairs[(intptr_t) threadNumber].payload, mytArray[(intptr_t) threadNumber].server_thread_home);

 	fprintf(stderr,"Sending Response <<<<%s>>>>\n",msgPairs[(intptr_t) threadNumber].payload);
	mq_send(mytArray[(intptr_t)threadNumber].mq_r[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
	perror("mq_send (CMD_REMOTE_HOME)");
      }
    }else if(strstr(msgPairs[(intptr_t) threadNumber].command,CMD_PUT) != NULL){
      /*                                                                                                                                               
       * Parse for filename
       */
      printf("Received from thread <%ld> (PUT): \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);
      fprintf(stderr,"THIS IS ***** THE MESSAGE COMMAND: <%s> *****\n",msgPairs[(intptr_t) threadNumber].command);
      tempBuff = strtok(msgPairs[(intptr_t) threadNumber].command," ");
      tempBuff = strtok(NULL," ");
      
      fprintf(stderr,"THIS IS ***** TEMPBUFF aka filname: <%s> *****\n",tempBuff);

      putFunction(tempBuff,mytArray, threadNumber);
      
    }else if(strncmp(CMD_REMOTE_DIR,msgPairs[(intptr_t) threadNumber].command,(ssize_t) 3) == 0){
      // Mutex Lock 
      pthread_mutex_lock(&mutex);
      
      printf("Received from thread <%ld> (DIR): \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);

      // Get CWD of Server main home to return to later                                                                                                     
      getcwd(server_main_home,MAX_DIR_SIZE);
      perror("getcwd of server_main_home");
      printf("THIS IS THE SERVER_HOME DIR <%s>\n",server_main_home);

      // CH dir thread server_thread_home                                                                                                                   
      if(strcmp(mytArray[(intptr_t) threadNumber].server_thread_home,"null") != 0){
	chdir(mytArray[(intptr_t) threadNumber].server_thread_home);
	perror("chdir to server_thread_home (local thread home)");
	printf("THIS IS the THREAD (%ld) HOME: <%s>\n",(intptr_t)threadNumber,mytArray[(intptr_t) threadNumber].server_thread_home);
      }
            
      dirFunction(mytArray, threadNumber);
    
      // Change server back to main server home directory                                                                                                   
      chdir(server_main_home);
      perror("chdir of server_main_home");
      printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_main_home);
      
      // unlock mutex 
      pthread_mutex_unlock(&mutex);
    }
    else{
      printf("Received from thread <%ld>: \"%s\"\n",(intptr_t)threadNumber ,msgPairs[(intptr_t) threadNumber].command);
      // Command not understood
      strcpy(msgPairs[(intptr_t) threadNumber].payload, "What Now?");
      fprintf(stderr,"<<<<%s>>>>\n",msgPairs[(intptr_t) threadNumber].payload);
      mq_send(mytArray[(intptr_t)threadNumber].mq_r[(intptr_t)threadNumber], (char *) &msgPairs[(intptr_t) threadNumber], sizeof(message_t), 0);
      perror("mq_send (default)");
    }
  }while(strcmp(msgPairs[(intptr_t) threadNumber].command,CMD_EXIT) != 0);
  
  fprintf(stderr,"Exit command from thread: <%ld> received\n",(intptr_t)threadNumber);
  // mytArray[(intptr_t) threadNumber].threadMap[(intptr_t)threadNumber]);
  // Make mytArray[(intptr_t threadNumber)] NULL
  
  memset(&mytArray[(intptr_t) threadNumber], 0, sizeof(mytArray[(intptr_t) threadNumber]));
  
  // Set Default Values
  mytArray[(intptr_t) threadNumber].avail = 0;
  strcpy(mytArray[(intptr_t) threadNumber].server_thread_home,"null");
  perror("mytArray[(intptr_t) threadNumber] is now NULL");
  
  pthread_exit(NULL);
  
}

void sigIntHandler(int number){
  int status;
  fprintf(stderr,"Signal Int Handler Called!\n");
  mq_close(mq);
  perror("mq Close");
  mq_unlink(messageQueueName);
  perror("mq_unlink(messageQueueName)");
  fprintf(stderr,"EXITING...\n");
  wait(&status);
  exit(EXIT_SUCCESS);
}

int changeDir(char* tempBufff){

  DIR* dir = opendir(tempBufff);      

  if (dir) {
      /* Directory exists. */
       closedir(dir);
      return 0;
    }
  else if (ENOENT == errno) {
      /* Directory does not exist. */
      fprintf(stderr,"There is no  directory called : \"%s\"\n",tempBufff);
      return -1;
    }else{
      /* opendir() failed for some other reason. */
       return -2;
    }
}


intptr_t findNextFree(myThread mtArray[]){
  for(z = 0; z<= MAX_CLIENTS; z++){
    if(mtArray[z].avail == 0){
      return z;
    }
  }
  return -1;
}

int getFunction(char* filename, myThread mArray[],void* threadNum){
  int fd_s;
  char server_m_home[MAX_DIR_SIZE];

  // Get CWD of Server main home to return to later                                                                                                     
  getcwd(server_m_home,MAX_DIR_SIZE);
  perror("getcwd of server_main_home");
  printf("THIS IS THE SERVER_HOME DIR <%s>\n",server_m_home);
  
  //Cd into the client's home server
  if(strcmp(mArray[(intptr_t) threadNum].server_thread_home,"null") != 0){
    printf("THIS IS THE THREAD_HOME DIR <%s>\n",mArray[(intptr_t) threadNum].server_thread_home);
    chdir(mArray[(intptr_t) threadNum].server_thread_home);
    perror("chdir to client server_home");
  }

  if(access( filename, F_OK ) != -1 ) {
    // file exists
    msgPairs[(intptr_t) threadNum].message_type = MESSAGE_TYPE_NORMAL;
    fprintf(stderr,"<<<<%d>>>>\n",msgPairs[(intptr_t) threadNum].message_type);
    mq_send(mytArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);

    //Open (filename)
    fd_s = open(filename,O_RDONLY);
    perror("open fd_s");
    
    if(fd_s == -1){
      fprintf(stderr,"OPEN fd_s ERROR (GET)\n");
      return -1;
    }
    
    while((msgPairs[(intptr_t) threadNum].num_bytes = (int) read(fd_s,msgPairs[(intptr_t) threadNum].payload,PATH_MAX)) != 0){
      
      // Send via message payload to client
      mq_send(mArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);    
    }
    memset(&msgPairs[(intptr_t) threadNum],0,sizeof(msgPairs[(intptr_t) threadNum]));
    msgPairs[(intptr_t) threadNum].message_type = MESSAGE_TYPE_SEND_END;
    mq_send(mArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);
    
    //Close the fd
    close(fd_s);
    perror("close fd_s");
    
    // Change server back to main server home directory                                                                                                   
    chdir(server_m_home);
    perror("chdir of server_m_home");
    printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_m_home);

  } else {
   // file doesn't exist
    
    // Change server back to main server home directory                                                                                                   
    chdir(server_m_home);
    perror("chdir of server_m_home");
    printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_m_home);

    fprintf(stderr,"NO FILE NAMED (((%s))) exsists\n",filename);
    return -1;
  }
  return 0;
}


int putFunction(char* filename, myThread mArray[],void* threadNum){
  int fd_s;
  char server_m_home[MAX_DIR_SIZE];

  // Get CWD of Server main home to return to later                                                                                                     
  getcwd(server_m_home,MAX_DIR_SIZE);
  perror("getcwd of server_main_home");
  printf("THIS IS THE SERVER_HOME DIR <%s>\n",server_m_home);
  
  //Cd into the client's home server
  if(strcmp(mArray[(intptr_t) threadNum].server_thread_home,"null") != 0){
    printf("THIS IS THE THREAD_HOME DIR <%s>\n",mArray[(intptr_t) threadNum].server_thread_home);
    chdir(mArray[(intptr_t) threadNum].server_thread_home);
    perror("chdir to client server_home");
  }
  
  if(msgPairs[(intptr_t) threadNum].message_type == MESSAGE_TYPE_SEND){
    //strcpy(msgPairs[(intptr_t) threadNum].payload,"MESSAGE_SEND_PUT");
    fprintf(stderr,"message type <<<%d>>>\n",msgPairs[(intptr_t) threadNum].message_type);
  }
  
  fd_s = open(filename,SEND_FILE_FLAGS,SEND_FILE_PERMISSIONS);
  perror("Open File for PUT");
  
  if(fd_s == -1){
    fprintf(stderr,"OPEN fd_s ERROR (PUT) \n");
    // Change server back to main server home directory                                                                                                   
    chdir(server_m_home);
    perror("chdir of server_m_home");
    printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_m_home);
    
    strcpy(msgPairs[(intptr_t) threadNum].payload,"SERVER ERROR!");
    mq_send(mArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);
    
    return -1;
 
  }else{
    
    //Write the payload to fd_s 
    while(1){
      mq_receive(mArray[(intptr_t)threadNum].mq_w[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);
      perror("mq_receive(mq_w)");
      
      if(msgPairs[(intptr_t) threadNum].message_type == MESSAGE_TYPE_SEND_END){
	fprintf(stderr,"********* MESSAGE_TYPE_SEND_END Received *********\n");
	// Close the fd_s
	close(fd_s);
	perror("Close fd_s (PUT)");
	break;
      }else{
	fprintf(stderr,"num bytes (%d) written to file \n",msgPairs[(intptr_t) threadNum].num_bytes);
	write(fd_s,msgPairs[(intptr_t) threadNum].payload,msgPairs[(intptr_t) threadNum].num_bytes);
	perror("Write PUT");
      }
    }
    // Change server back to main server home directory                                                                                                   
    chdir(server_m_home);
    perror("chdir of server_m_home");
    printf("SERVER MAIN HOME RETURNED TO  <%s>\n",server_m_home);
    
    strcpy(msgPairs[(intptr_t) threadNum].payload,"PUT COMMAND DONE!"); 
    mq_send(mArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);
  }
  return 0;
}


int dirFunction(myThread mArray[],void* threadNum){
  
  // Initial send for client to know dir data is coming
  msgPairs[(intptr_t) threadNum].message_type = MESSAGE_TYPE_SEND;

  if((file = popen(CMD_LS_POPEN,"r")) != NULL){
    while(fgets(msgPairs[(intptr_t) threadNum].payload,PATH_MAX,file) != NULL){
      mq_send(mArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);
    }
    pclose(file);
  }else{
    fprintf(stderr,"SERVER popen error for dirFunction \n");
    return -1;
  }

  //Once dir listing is finished send MESSAGE_DIR_END
  msgPairs[(intptr_t) threadNum].message_type = MESSAGE_TYPE_DIR_END;
  memset(msgPairs[(intptr_t) threadNum].payload,0,PATH_MAX);
  strcpy(msgPairs[(intptr_t) threadNum].payload,"DIR COMMAND DONE!\n"); 
  mq_send(mArray[(intptr_t)threadNum].mq_r[(intptr_t)threadNum], (char *) &msgPairs[(intptr_t) threadNum], sizeof(message_t), 0);
  return 0;
}



/* Foot Notes:
 *  
 * get() function
 *    Open(filename) both C & S
 *    Read(filename,payload) on S
 *    Write(filename,payload) on C
 *    Repeat Until Message end Received
 *    Close file descriptors on both C and S
 */

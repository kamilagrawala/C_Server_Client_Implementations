/* Author Kamil Agrawala
* Class: CS344-001       
* Homework Assingment #4 
* Email: agrawalk@onid.orst.edu
* Due: 03/02/15 (with extra credit)                                           
* Program: posixmsg_client.c                                                  
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
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getopt is in unistd.h & pipe system call */
#include <getopt.h>    /* another library for getopt*/
#include <string.h>    /* general use for C strings  */
#include <errno.h>
#include <mqueue.h>

#include "posixmsg.h"

char mclientReaderName[100];
char mclientWriterName[100];
char messageQueueName[100];
mqd_t mq_r, mq_w, mq;
char writeBuff[sizeof(message_t)];
char readBuff[sizeof(message_t)];
char *tempBuff;
ssize_t ln;
int fd_c;
FILE *file;
int k;
char currentDir[PATH_MAX];
/*
 * Test Vars
 */ 
#define test 1

/* 
 * Message Queue Variables
 */
message_t msg;
struct mq_attr attr;

/*                                                                                                                                                            * Function Definations                                                                                                                                       */
void sigIntHandler(int);
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


int main(int argc, char* argv[]){
  /*
   * Main Variables
   */
  char putCommand[2 * PATH_MAX];

  // umask permissions
  umask(0);

  /*
   * Signal handlers
   */
  signal(SIGINT, sigIntHandler);

  // initialize the queue attributes   
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(message_t); // Specify the maximum size of a message.                                                                        
  attr.mq_curmsgs = 0;
  
  //CREATE_CLIENT_READER_NAME
  CREATE_CLIENT_READER_NAME(mclientReaderName,(int)getpid());
  fprintf(stderr,"Client Reader Name:\n %s\n",mclientReaderName);  

  // Open the message queue

  CREATE_SERVER_QUEUE_NAME(messageQueueName);

  fprintf(stderr,"Name of message queue:\n %s\n",messageQueueName);

  mq = mq_open(messageQueueName, O_CREAT | O_WRONLY,QUEUE_PERMISSIONS,&attr);

  perror("mq_open(mq)");

  mq_r = mq_open(mclientReaderName, O_CREAT | O_RDONLY,QUEUE_PERMISSIONS,&attr);
  
  perror("mq_open (mq_r) (reader)");
  
  CREATE_CLIENT_READER_NAME(mclientReaderName,(int)getpid());

  fprintf(stderr,"Client Writer Name:\n %s\n",mclientWriterName);

  CREATE_CLIENT_WRITER_NAME(mclientWriterName,(int)getpid());  
  
  mq_w = mq_open(mclientWriterName,O_CREAT | O_WRONLY,QUEUE_PERMISSIONS,&attr);
  
  perror("mq_open (mq_w) (writer)");
  
  fprintf(stderr,"Client Writer Name:\n %s\n",mclientWriterName);  

  /*
   * Send intial PID to mssage queue
   */
  memset(&msg, 0, sizeof(message_t));
  sprintf(writeBuff,"%d",getpid());
  strcpy(msg.command, writeBuff);
  // send the message                                                                                                                                       
  mq_send(mq, (char *) &msg, sizeof(message_t), 0);
  perror("mq_send(mq)");
  fprintf(stderr,"<%s>\n",writeBuff);
  // Now Close the Server Connection
  mq_close(mq);
  perror("Close mq");

  puts("Ready for input!");
  do {
    printf(PROMPT);
    fflush(stdout);
    
    memset(&msg, 0, sizeof(message_t));
    
    fgets(writeBuff,sizeof(message_t), stdin);
    
    ln = strlen(writeBuff) - 1;
    if (writeBuff[ln] == '\n'){
      writeBuff[ln] = 0;
    }
    
    if(strcmp(writeBuff,CMD_HELP) == 0){
       helpFunction();
     }
    else if(strcmp(writeBuff,CMD_LOCAL_DIR) == 0){
      file = popen(CMD_LS_POPEN,"r");
      perror("Local Popen");
      while(fgets(writeBuff,sizeof(writeBuff),file) != NULL){
        printf("%s",writeBuff);
      }
      pclose(file);
    }
    else if(strncmp(writeBuff,"clear",(ssize_t)4) == 0){
      file = popen("clear","w");
      perror("Local Popen (Extra Clear Command)");
      pclose(file);
      perror("Local Popen (Extra Clear Command)");
    }
    else if (strcmp(writeBuff,CMD_LOCAL_PWD) == 0){
      file = popen("pwd","r");
      perror("Local Popen for PWD");
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
      fprintf(stderr,"HOME: <%s>  CWD: <%s> \n",getenv("HOME"),writeBuff);
    }
    else if(strncmp(writeBuff,CMD_LOCAL_CHDIR, (ssize_t)3) == 0){
      fprintf(stderr,"Changing Local Client Directory\n");
      printf("lcd command: <%s>\n", writeBuff);

      //dirString[PATH_MAX] = 0;
      tempBuff = strtok(writeBuff," ");
      tempBuff = strtok(NULL, " ");
      printf("Changing into <<<%s>>>\n", tempBuff);
      
      if(tempBuff[0] != 0){
	//Change the directory
	k = chdir(tempBuff);
	perror("chdir:");
	
	// Get new CWD                                                                                                                                      
	getcwd(tempBuff,sizeof(tempBuff));
	if(k == 0){
	  fprintf(stdout,"Local Client PWD is now <%s>\n",tempBuff);
	}else{
	  fprintf(stdout,"Local Client PWD was not **changed**\n");
	}
	memset(tempBuff, ' ', sizeof(tempBuff));
      }else{
	printf("Format is lcd  <dir>\n");
      }
    }
    else if(strstr(writeBuff,CMD_GET) != NULL){
      strcpy(msg.command, writeBuff);
      // send the message
      mq_send(mq_w, (char *) &msg, sizeof(message_t), 0);
      perror("mq_send(mq_w)");
      
      tempBuff = strtok(writeBuff," ");
      tempBuff = strtok(NULL," ");
      
      mq_receive(mq_r, (char *) &msg, sizeof(message_t), 0);
      fprintf(stderr,"<%d>\n",msg.message_type);
      
      if(msg.message_type == MESSAGE_TYPE_NORMAL){
    	// open file to be written to
	fd_c = open(tempBuff,SEND_FILE_FLAGS,SEND_FILE_PERMISSIONS);
	perror("fd_c open");

	// receive the message
	while(1){
	  mq_receive(mq_r, (char *) &msg, sizeof(message_t), 0);
	  perror("mq_receive(mq_r)");
	
	  if(msg.message_type == MESSAGE_TYPE_SEND_END){
	    close(fd_c);
	    perror("close fd_c");
	    break;
	  }

	  write(fd_c,msg.payload,(msg.num_bytes));
	  
	  memset(msg.payload,0,sizeof(msg.payload));      
	}
      }else if(msg.message_type == MESSAGE_TYPE_ERROR){
	
	printf("No file name <%s> was found\n", tempBuff);
	
      }
    }else if(strncmp(CMD_PUT,writeBuff,(ssize_t)3) == 0){
      printf("PUT command: <%s>\n", writeBuff);

      strcpy(putCommand,writeBuff);
      
      tempBuff = strtok(writeBuff," ");
      tempBuff = strtok(NULL," ");
      
      if(access(tempBuff, F_OK ) != -1 ) {
	/****  file exists  ****/
	
	//Set Message command to put
	fprintf(stderr,"Sending <<<%s>>>\n",putCommand);
	//Set message type as MESSAGE_SEND
	msg.message_type = MESSAGE_TYPE_SEND;

	//Set intial PUT Command 
	strcpy(msg.command, putCommand);
	
	// Send the intial message
	mq_send(mq_w, (char *) &msg, sizeof(message_t), 0);
	perror("mq_send(mq_w)");
		
	printf("PUT File: <%s>\n", tempBuff);
	
	//Memset message to 0
	memset(&msg,0,sizeof(msg));
        
	//OPEN filename for reading
	fd_c = open(tempBuff,O_RDONLY);
	perror("Open fd_c");
	
	while((k = read(fd_c,msg.payload,PATH_MAX)) != 0){
	  perror("read");
	  msg.num_bytes = k;
	  fprintf(stderr,"<<<%d>>>\n",msg.num_bytes);
	  mq_send(mq_w, (char *) &msg, sizeof(message_t), 0);
	  if (k == -1){
	    fprintf(stderr,"*** Read Error BREAK ***\n");
	    break;
	  }
	}
	
	fprintf(stderr,"Done sending file <%s>\n",tempBuff);
      
	//Send Final Message MESSAGE_SEND_END
	msg.message_type = MESSAGE_TYPE_SEND_END;
      
	/* // Send final message */
	mq_send(mq_w, (char *) &msg, sizeof(message_t), 0); 
	perror("mq_send (mq_r)"); 

	mq_receive(mq_r, (char *) &msg, sizeof(message_t), 0);
	fprintf(stderr,"<%s>\n",msg.payload);
	
      }
      else {
	getcwd(currentDir,PATH_MAX);
	fprintf(stderr,"File: <%s> does not exist in client's cwd (%s) \n",tempBuff,currentDir);
      }
    }
    else if(strncmp(writeBuff,CMD_REMOTE_DIR,(ssize_t)3) == 0){
      printf("CM_REMOTE_DIR : <%s>\n", writeBuff);
      
      msg.message_type = MESSAGE_TYPE_DIR;
      strcpy(msg.command, writeBuff);

      // send the intial dir message
      mq_send(mq_w, (char *) &msg, sizeof(message_t), 0);
      perror("mq_send (mq_w)");
      
      // receive the intial dir message response
      mq_receive(mq_r, (char *) &msg, sizeof(message_t), 0);
      perror("mq_receive (mq_r)");

      if(msg.message_type == MESSAGE_TYPE_SEND){
	printf("\n");
	while(1){
	  mq_receive(mq_r, (char *) &msg, sizeof(message_t), 0);
	  printf("%s",msg.payload);
	  if(msg.message_type == MESSAGE_TYPE_DIR_END){
	    fprintf(stdout,"\n");
	    break;
	  }
	}
      }
    }
     else{
       strcpy(msg.command, writeBuff);
       // send the message
       mq_send(mq_w, (char *) &msg, sizeof(message_t), 0);
       perror("mq_send (mq_w)");
       
       // receive the message
       mq_receive(mq_r, (char *) &msg, sizeof(message_t), 0);
       perror("mq_receive (mq_r)");
       fprintf(stderr,"<%s>\n",msg.payload);
     }
    
  } while(strcmp(writeBuff,CMD_EXIT) != 0);
    
  /*
   * Close the mq_r and mq_w
   */
  sigIntHandler(1);
  return (EXIT_SUCCESS);
}


void sigIntHandler(int number){
  int status;
  fprintf(stderr,"SigIntHandler Called\n");
  /*
   * Send Exit command to the Server
   */
  strcpy(msg.command, CMD_EXIT);
  mq_send(mq_w, (char *) &msg, sizeof(message_t), 0);
  
  /*
   * Close the mq_r and mq_w
   */
  mq_close(mq_r);
  perror("close: mq_r");
  mq_unlink(mclientReaderName);
  perror("unlink: mq_r");
  mq_close(mq_w);
  perror("close: mq_w");
  mq_unlink(mclientWriterName);
  perror("unlink: mq_w");
  fprintf(stderr,"EXITING...\n");
  wait(&status);
  exit(EXIT_SUCCESS);
}

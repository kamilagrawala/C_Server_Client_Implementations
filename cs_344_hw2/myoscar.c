/* Author: Kamil Agrawala
 * Email: agrawalk@onid.orst.edu
 * Class: Cs344-001
 * Assingment #2
 * Programming Projects myoscar.c file
 * USING ONE LATE GRACE DAY ! (Emailed Professor Prior to deadline as per requirements)
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getopt is in unistd.h*/
#include <getopt.h>    /* another library for getopt*/
#include <string.h>    /* general use for C strings  */
#include <ctype.h>     /* for addtional libary functions*/
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <math.h>
/* 
 * Oscar File include
 */
#include "oscar.h"

/*
 * Various Definations
 */
#define BUF_SIZE 16384
#define BUF_SIZE_SMALL 1
#define TEMP_STRING_SIZE 100
/*
 * General variable definations
 */
int i; 
int j;
int k;
int z;
int start, end;
int lengthOfInputArguments;
char readBuf[OSCAR_MAX_MEMBER_FILE_SIZE];
char readBufHeader[OSCAR_ID_LEN];
struct stat statBuf;
struct passwd* pwd;
struct group* grp;
char tempString[TEMP_STRING_SIZE];
char popenCmd[100];
FILE *file;
/*
 * SubFlags
 */
int E_flag = 0;
int o_Flag = 0;
int do_Flag = 0;
/*
 * File Descriptors
 */
int fd_ArchiveFile;
int fd_DataFile;
int fd_tempArchiveFile;
int fd_junkFile;
/*
 * Function Definations
 */
int handleArgv(int countArgc, char *inputArguments[]);
void formatArgv(int argc, char* argv[]);
void helpFunction(void);
int aFunction(int argc, char* argv[]);
int A_Function(int argc,char* argv[]);
int tFunction(int argc, char* argv[]);
int T_Function(int argc, char* argv[]);
int eFunction(int argc, char* argv[]);
int dFunction(int argc, char* argv[]);
void V_Function(void);
/*
 * Helper Functions
 */
char *permOfFile(mode_t mode);
struct stat statFunction(int argc, char* argv[]);
struct passwd* userDetails(uid_t userId);
struct group* groupDetails(gid_t groupId);
int dSearch(char* search[],char* value);
void strip_char(char *str, char strip);
void getReg(void);
int getLine(char* filename);

/*
 * Struct Definations
 */
struct oscar_hdr_s *ino;

int handleArgv(int countArgc, char* inputArguments[]){

  char argvBuf[10000];

  for ( i = 1; i <= countArgc - 1; i++){
    strcpy(argvBuf, inputArguments[i]);
    if((strstr(argvBuf,"-h") != NULL)){
      printf("-h caught at %d position\n",i);
      helpFunction();
    }
  
    if((strstr(argvBuf,"-o") != NULL)){
      puts("do_Flag is set");
      do_Flag = 1;
    }
  }

  if(countArgc >= 2){
    if(strstr(inputArguments[1],"h") != NULL){
      helpFunction();
    }
    if(strstr(inputArguments[1], "a") != NULL) {
      aFunction(countArgc, inputArguments);
    }
    if(strstr(inputArguments[1], "A") != NULL) {
      // A_Function(countArgc, inputArguments);
      getLine("junkFile");
    }
    if(strstr(inputArguments[1], "v") != NULL) {
      
    }
    if(strstr(inputArguments[1],"t") != NULL){
      tFunction(countArgc, inputArguments);
    }
    if(strstr(inputArguments[1],"V") != NULL){
      V_Function();
    }
    if(strstr(inputArguments[1],"T") != NULL){
      T_Function(countArgc, inputArguments);
    }
    if(strstr(inputArguments[1] , "o") != NULL){
      o_Flag = 1;
      puts("o Flag set!\n");
    }
    if(strstr(inputArguments[1],"E") != NULL){
      E_flag = 1;
      printf("E_flag is: <%d>",E_flag);
      eFunction(countArgc, inputArguments);
    }
    if(strstr(inputArguments[1],"e") != NULL){
      eFunction(countArgc, inputArguments);
    }
    if(strstr(inputArguments[1],"d") != NULL){
      dFunction(countArgc, inputArguments);
    }
  }else{
    puts("User must provide input");
  }
  
  return(EXIT_SUCCESS);
}

struct stat statFunction(int argc, char* argv[]){
  if (argv[2] != NULL &&  argv[3] != NULL){
    i = 3;
    j = 0;

    if((ino = malloc(sizeof(struct oscar_hdr_s) * argc)) != NULL){
       if(access(argv[2], F_OK ) != -1 ) {
	// file exists
	fd_ArchiveFile = open(argv[2],O_RDWR | O_CREAT | O_APPEND, 0644);
	if (fd_ArchiveFile == -1) {
	  perror(argv[1]);
	  exit(EXIT_FAILURE);
	}	
       }else {
	// file doesn't exist
	fd_ArchiveFile = open(argv[2],O_RDWR | O_CREAT | O_APPEND, 0644);
	if (fd_ArchiveFile == -1) {
	  perror(argv[1]);
	  exit(EXIT_FAILURE);
	}	
	write(fd_ArchiveFile,"!<oscar>\n",9);
       } 
      
      while(argv[i] != NULL){
	if(lstat(argv[i],&statBuf) == 0){
	  /*
	   * Name of File
	   */
	  if(strlen(argv[i]) > OSCAR_MAX_FILE_NAME_LEN){
	    printf("Member file <%s> name length too long!\nexiting...\n",argv[i]);
	    exit(EXIT_FAILURE);
	  }

	  memset(&ino[j], ' ', sizeof(oscar_hdr_t));
	  sprintf(tempString,"%s",argv[i]);
	  memcpy(ino[j].oscar_fname,tempString,strlen(tempString));
	  puts("#### : 1");
	  puts(ino[j].oscar_fname);
	  ino[j].oscar_fname[30] = 0;
	  
	  /*
	   * Length of File
	   */
	  sprintf(tempString,"%2d",(int) strlen(tempString));
	  memcpy(ino[j].oscar_fname_len,tempString,strlen(tempString));
	  ino[j].oscar_fname[32] = 0;
	  puts("#### : 2");
	  puts(ino[j].oscar_fname);
	  
	  /*
	   * Time a_time
	   */
	  sprintf(tempString,"%lld",(long long)statBuf.st_atime);
	  memcpy(ino[j].oscar_adate,tempString,strlen(tempString));
	  printf("Last file Access: Unix %lld  %s",(long long) statBuf.st_atime, ctime(&statBuf.st_atime));
	  ino[j].oscar_fname[42] = 0;
	  puts("#### : 3");
	  puts(ino[j].oscar_fname);
	  
	  /*
	   * Time m_time
	   */
	  sprintf(tempString,"%lld",(long long)statBuf.st_mtime);
	  memcpy(ino[j].oscar_mdate,tempString,strlen(tempString));
	  printf("Last file Modification: Unix %lld  %s",(long long) statBuf.st_mtime, ctime(&statBuf.st_mtime));
	  ino[j].oscar_fname[52] = 0;
	  puts("#### : 4");
	  puts(ino[j].oscar_fname);
	  
	  /*
	   * Time c_time
	   */
	  sprintf(tempString,"%lld",(long long)statBuf.st_ctime);
	  memcpy(ino[j].oscar_cdate,tempString,strlen(tempString));
	  printf("Last status change: Unix %lld  %s",(long long) statBuf.st_ctime, ctime(&statBuf.st_ctime));
	  ino[j].oscar_fname[62] = 0;
	  puts("### : 5");
	  puts(ino[j].oscar_fname);

	  /*
	   * User ID of File
	   */
	  sprintf(tempString,"%lu",(long int)statBuf.st_uid);
	  memcpy(ino[j].oscar_uid,tempString,strlen(tempString));
	  printf("User ID: %s\n",ino[j].oscar_uid);
	  printf("User Name (converted from uid): %s\n",(userDetails(statBuf.st_uid))->pw_name);
	  ino[j].oscar_fname[67] = 0;
	  puts("### : 6");
	  puts(ino[j].oscar_fname);

	  /*
	   * Group ID of File
	   */
	  sprintf(tempString,"%lu",(long int)statBuf.st_gid);
	  memcpy(ino[j].oscar_gid,tempString,strlen(tempString));
	  printf("Group ID: %s\n",ino[j].oscar_uid);
	  printf("Group Name (converted from gid): %s\n",(groupDetails(statBuf.st_gid))->gr_name);
	  ino[j].oscar_fname[73] = 0;
	  puts("### : 7");
	  puts(ino[j].oscar_fname);
	  
	  /*
	   * File Mode
	   */
	  sprintf(tempString,"%o",statBuf.st_mode);
	  memcpy(ino[j].oscar_mode,tempString,strlen(tempString));
	  printf("Mode: %s\n",ino[j].oscar_mode);
	  ino[j].oscar_fname[79] = 0;
	  puts("### : 8");
	  puts(ino[j].oscar_fname);
	  
	  /*
	   * Size of File
	   */
	  if( (long long)statBuf.st_size > OSCAR_MAX_MEMBER_FILE_SIZE){
	    printf("Member file <%s> size (%lld)too big!\nexiting...\n",argv[i],(long long)statBuf.st_size);
	    exit(EXIT_FAILURE);
	  }
	  //memset(tempString, ' ', sizeof(tempString));
	  sprintf(tempString,"%-16lu",(unsigned long)statBuf.st_size);
	  memcpy(ino[j].oscar_size,tempString,strlen(tempString));
	  puts("### : 9");
	  //ino[j].oscar_fname[82] = 0;
	  //puts(ino[j].oscar_fname);

	  /*
	   * Header End 
	   */
	  sprintf(tempString,"%s",OSCAR_HDR_END);
	  memcpy(ino[j].oscar_hdr_end,tempString,strlen(tempString));
	  //puts("### : 10");
	  //ino[j].oscar_fname[84] = 0;
	  //puts(ino[j].oscar_fname);
	  
	  fd_DataFile = open(argv[i],O_RDWR | O_CREAT | O_APPEND, 0644);
	  if (fd_DataFile == -1) {
	    perror(argv[i]);
	    exit(EXIT_FAILURE);
	  
	  }
	  read(fd_DataFile,readBuf,statBuf.st_size);
	  write(fd_ArchiveFile, ino[j].oscar_fname,sizeof(oscar_hdr_t)); 
	  write(fd_ArchiveFile, readBuf, statBuf.st_size); 
	  puts("\n");
	  i++;
	  j++;
	}else{
	  perror("lstat");
	  exit(EXIT_FAILURE);
	}
      }
    }else{
      perror("malloc");
      exit(EXIT_FAILURE);
    }
  }else{
    puts("Incorrect Format");
    helpFunction();
  }
  close(fd_ArchiveFile);
  perror("Close 1");
  return statBuf;
}


struct passwd* userDetails(uid_t userId){
  if ((pwd = getpwuid(userId))!= NULL){
    return pwd;
  }else{
    perror("getpwuid");
    exit(EXIT_FAILURE);
  }
}

struct group* groupDetails(gid_t groupId){
  if ((grp = getgrgid(groupId))!= NULL){
    return grp;
  }else{
    perror("getgrgid");
    exit(EXIT_FAILURE);
  }
}

void helpFunction(void){
  puts("Program: myoscar");
  printf("\n");
  puts("options: aACdeEhmotTuvVS");
  printf("\n");
  puts("\t-a add member(s) from command line to archive");
  puts("\t-d delete member(s) from command line");
  puts("\t-e extract member(s) from archive to file, from command line");
  puts("\t-E extract member(s) from archive to file, from command line, keep current time");
  puts("\t-h show the help text and exit");
  puts("\t-o overwrite existing files on extract");
  puts("\t-t short TOC");
  puts("\t-T long TOC");
  puts("\t-v verbose processing");
  puts("\t-V show version and exit");
  puts("\t-A add all regular files to archive (Extra Credit)");
  puts("\t-m mark member(s) as deleted from command line (Extra Credit)");
  puts("\t-u unmark member(s) as deleted from command line (Extra Credit)");
  puts("\t-C cleanse (remove) the archive of all marked members (Extra Credit)");
  puts("\t-S create/check SHA256 digest on member(s) added/extracted (Extra Credit)");
  printf("\n");
  exit(EXIT_SUCCESS);
}


/*
 * A option Helper Extra Credit
 */
void getReg(void){
  file = popen("ls -p | grep -v / > junkFile","r");
  pclose(file);
}

int getLine(char *filename){
  char lines[1000];
  int nL;
  
  // Get junkFile
  getReg();
  
  file = popen("wc -l junkFile","r");
  fgets(lines,sizeof(lines),file);
  perror("fgets: ");

  nL = strtoll(lines,NULL,10);
  
  printf("No of lines in <%s> are (%d)\n",filename,nL);

  pclose(file);

  return nL;
}


/*
 * The a Function
 */
int aFunction(int argc,char* argv[]){
  if (argv[2] == NULL){
    puts("*** Archive file not specified");
    puts("*** Exiting...");
    exit(EXIT_FAILURE);
  }
  else if( argv[3] == NULL){
    puts("*** Must specify at least 1 member to add ***");
    exit(EXIT_FAILURE);
  }
  else if( argv[2] != NULL && argv[3] != NULL){
    statFunction(argc,argv);
  }
  return(EXIT_SUCCESS);
}

/*
 * All regular files add to archive 
 */ 

/*int A_Function(int argc,char* argv[]){
  char* tempBuf[getLine("junkFile")];
  char delim[] = "\n";
  char* token;
 
  fd_junkFile = open("./junkFile",O_RDWR);
  read(fd_junkFile,tempBuf,sizeof(tempBuf));
  perror("Read junkFile: ");

  // Get each entry in file
  for (token = strtok(tempBuf, delim); token; token = strtok(NULL, delim))
    {
      printf("token=%s\n", token);
    }
  
  close(fd_junkFile);
  }*/

/*
 * The t option
 */
int tFunction(int argc, char* argv[]){
  
  /*
   * Function Variables
   */
  char tempSizeBuf[OSCAR_FILE_SIZE];
  char *endptr = malloc(sizeof(char*)*10);
  memset(tempSizeBuf,' ',OSCAR_FILE_SIZE);
  //memset(&endptr,0,10);
  //tempSizeBuf[16] = 0;
  if (argv[2] == NULL){
    puts("*** Archive file not specified");
    puts("*** Exiting...");
    exit(EXIT_FAILURE);
  }
  else if(argv[2] != NULL){
    if(access(argv[2], F_OK ) != -1 ) {
      // file exists
      fd_ArchiveFile = open(argv[2],O_RDWR | O_CREAT | O_APPEND, 0644);
      if (fd_ArchiveFile == -1) {
	perror(argv[2]);
	printf("Archive file %s does not exsist in current directory\n",argv[2]);
	exit(EXIT_FAILURE);
      }
      k = read(fd_ArchiveFile,readBufHeader,OSCAR_ID_LEN);
      if( k < 0){
	perror("Read error");
	exit(EXIT_SUCCESS);
      }else{
	if(strcmp(readBufHeader,OSCAR_ID) != 0){
	  printf("Bad Header in %s\n",argv[2]);
	  puts("Exiting...");
	  exit(EXIT_SUCCESS);
	}else{
	  printf("Short table of contents for oscar archive file: %s\n",argv[2]);
	  lseek(fd_ArchiveFile,OSCAR_ID_LEN,SEEK_SET);
	  memset(readBuf,' ',sizeof(readBuf));
	  readBuf[OSCAR_MAX_FILE_NAME_LEN + 1] = 0;
	  while(read(fd_ArchiveFile,readBuf,OSCAR_MAX_FILE_NAME_LEN) != 0){
	    //puts("File Name");
	    printf("\t");
	    for(i=0; i<=OSCAR_MAX_FILE_NAME_LEN;i++){
	      if(readBuf[i] != ' '){
		printf("%c",readBuf[i]);
	      }else{
		break;
	      }
	    }
	    printf("\n");
	    // OFFSET by File Length
	    lseek(fd_ArchiveFile,2,SEEK_CUR);
	    // OFFSET by 3 * DATE
	    lseek(fd_ArchiveFile,(3 * OSCAR_DATE_SIZE),SEEK_CUR);
	    // OFFSET by uid Size
	    lseek(fd_ArchiveFile,OSCAR_GUID_SIZE,SEEK_CUR);
	    // OFFSET by gid size
	    lseek(fd_ArchiveFile,OSCAR_GUID_SIZE,SEEK_CUR);
	    // OFFSET BY MODE
	    lseek(fd_ArchiveFile,OSCAR_MODE_SIZE,SEEK_CUR);
	    // READ THE FILE SIZE
	    read(fd_ArchiveFile,tempSizeBuf,OSCAR_FILE_SIZE);
	    lseek(fd_ArchiveFile,1,SEEK_CUR);
	    //puts("Size:");
	    //printf("%s\n",tempSizeBuf);
	    // OFFSET by SHA_DIGEST_LEN
	    lseek(fd_ArchiveFile,OSCAR_SHA_DIGEST_LEN,SEEK_CUR);
	    // OFFSET by HDR_END
	    lseek(fd_ArchiveFile,OSCAR_HDR_END_LEN,SEEK_CUR);
	    // OFFSET BY perviously recorded file size
	    z = strtoll(tempSizeBuf,&endptr,10);
	    //printf("Recovered size is: %lld\n",(long long)z);
	    lseek(fd_ArchiveFile,(int)z,SEEK_CUR);
	    //sleep(2);
	  }
	  close(fd_ArchiveFile);
	  //puts("Done with while loop");
	}
	//puts("Done with stncmp else statement");
      }
      //puts("Done with access else condition");
    }else {
      // file doesn't exist
      printf("*** Archive file %s does not exist\n",argv[2]);
      puts("*** Exiting...");
      exit(EXIT_SUCCESS);
    }
  }
  return (EXIT_SUCCESS);
}

/*
 * Helper function to get octal mode into human readable permissions
 */
char *permOfFile(mode_t mode)
{
  char *p;
  static char perms[10];
  
  p = perms;
  strncpy(perms, "---------",sizeof(perms));
  
  /*
   * The permission bits are three sets of three
   * bits: user read/write/exec, group read/write/exec,
   * other read/write/exec. Deal with each set
   * of three bits in one pass through the loop.
   */
  for (i=0; i < 3; i++) {
    if (mode & (S_IREAD >> i*3))
      *p = 'r';
    p++;
    if (mode & (S_IWRITE >> i*3))
      *p = 'w';
    p++;
    if (mode & (S_IEXEC >> i*3))
      *p = 'x';
    p++;
  }
  
  if ((mode & S_ISUID) != 0)
    perms[2] = 's';
  if ((mode & S_ISGID) != 0)
    perms[5] = 's';
  if ((mode & S_ISVTX) != 0)
    perms[8] = 't';
  return(perms);
}


int T_Function(int argc,char* argv[]){
  /*
   * Function Variables
   */
  int tempSize;
  int modeOctal;
  int tempUid, tempGuid;
  time_t tempTime;
  char tempSizeBuf[OSCAR_MAX_MEMBER_FILE_SIZE];
  char *endptr2 = malloc(sizeof(char*)*10);
  char timeString[80];  
  struct tm *ts;
  
  memset(tempSizeBuf,' ',OSCAR_MAX_MEMBER_FILE_SIZE);
  
  if(argv[2] == NULL){
    puts("*** Archive file not specified");
    puts("*** Exiting...");
    exit(EXIT_FAILURE);
  }else if (argv[2] != NULL){
    if(access(argv[2], F_OK ) != -1 ) {
      // file exists
      fd_ArchiveFile = open(argv[2],O_RDWR | O_CREAT | O_APPEND, 0644);
      // Check for Oscar Header
      read(fd_ArchiveFile,readBufHeader,OSCAR_ID_LEN);
      if(strcmp(readBufHeader,OSCAR_ID) != 0){
	printf("Bad Header in %s\n",argv[2]);
	puts("Exiting...");
	exit(EXIT_SUCCESS);
	
      }else{
	printf("Long table of contents for oscar archive file: %s\n",argv[2]);
	//OFFSET by OSCAR_ID_LEN
	lseek(fd_ArchiveFile,OSCAR_ID_LEN,SEEK_SET);
	// Read file name into tempSizeBuf
	while(read(fd_ArchiveFile,tempSizeBuf,OSCAR_MAX_FILE_NAME_LEN)){
	  printf("    File name: ");
	  for(i = 0; i< OSCAR_MAX_FILE_NAME_LEN; i++){
	    if(tempSizeBuf[i] != ' '){
	      printf("%c",tempSizeBuf[i]);
	      if(i == OSCAR_MAX_FILE_NAME_LEN - 1){
		tempSizeBuf[i+1] = 0;
	      }
	    }else{
	      break;
	    }
	  }
	  printf("\n");
	  //Read for Size
	  memset(tempSizeBuf,' ',OSCAR_MAX_MEMBER_FILE_SIZE);
	  lseek(fd_ArchiveFile,48,SEEK_CUR);
	  read(fd_ArchiveFile,tempSizeBuf,OSCAR_FILE_SIZE);
	  printf("\tFile size:   ");
	  for(i = 0; i< OSCAR_FILE_SIZE; i++){
	    if(tempSizeBuf[i] != ' ' ){
	      printf("%c",tempSizeBuf[i]);
	    }else{
	      if(i == OSCAR_FILE_SIZE - 1){
		tempSizeBuf[OSCAR_FILE_SIZE+1] = 0;
	      }
	      tempSize = strtoll(tempSizeBuf,NULL,10);
	      printf(" bytes\n");
	      break;
	    }
	  }
	  // Read For Permissions
	  lseek(fd_ArchiveFile,-22,SEEK_CUR);
	  memset(tempSizeBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	  read(fd_ArchiveFile,tempSizeBuf,6);
	  tempSizeBuf[7] = 0;
	  printf("\tPermissions: ");
	  /*
	   * Convert Octal string to int to be used in the permOfFile function
	   */
	  modeOctal = strtoll(tempSizeBuf,&endptr2,8);
	  printf("%s       ",permOfFile(modeOctal));

	  printf("(");
	  for (j = 2; j <= strlen(tempSizeBuf); j++){
	    if(tempSizeBuf[j] != ' '){
	      printf("%c",tempSizeBuf[j]);
	    }else{
	      break;
	    }
	  }
	  printf(")\n");
	  // Read the uid
	  memset(tempSizeBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	  lseek(fd_ArchiveFile,-16,SEEK_CUR);
	  read(fd_ArchiveFile,tempSizeBuf,5);
	  tempSizeBuf[5] = 0;
	  tempUid = strtoll(tempSizeBuf,&endptr2,10);
	  printf("\tFile owner:  %-16s",userDetails(tempUid)->pw_name);
	  printf("(uid: %s)\n",tempSizeBuf);
	  //Read the guid
	  memset(tempSizeBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	  read(fd_ArchiveFile,tempSizeBuf,5);
	  tempGuid = strtoll(tempSizeBuf,&endptr2,10);
	  tempSizeBuf[5] = 0;
	  printf("\tFile group:  %s",groupDetails(tempGuid)->gr_name);
	  printf("        (gid: %s)\n",tempSizeBuf);
	  //Read Access Date
	  memset(tempSizeBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	  lseek(fd_ArchiveFile,-40,SEEK_CUR);
	  read(fd_ArchiveFile,tempSizeBuf,OSCAR_DATE_SIZE);
	  tempSizeBuf[10] = 0;
	  printf("\tAccess date: ");
	  tempTime = (time_t) strtoll(tempSizeBuf, NULL, 10);
	  ts = localtime(&tempTime);
	  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %z (%Z) %a", ts);
	  printf("%s ",timeString);
	  printf(" %s\n",tempSizeBuf);
	  //Read Modify Date
	  memset(tempSizeBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	  lseek(fd_ArchiveFile,0,SEEK_CUR);
	  read(fd_ArchiveFile,tempSizeBuf,OSCAR_DATE_SIZE);
	  tempSizeBuf[10] = 0;
	  printf("\tModify date: ");
	  tempTime = (time_t) strtoll(tempSizeBuf, NULL, 10);
	  ts = localtime(&tempTime);
	  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %z (%Z) %a", ts);
	  printf("%s ",timeString);
	  printf(" %s\n",tempSizeBuf);
	  //Read Status Date
	  memset(tempSizeBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	  lseek(fd_ArchiveFile,0,SEEK_CUR);
	  read(fd_ArchiveFile,tempSizeBuf,OSCAR_DATE_SIZE);
	  tempSizeBuf[10] = 0;
	  printf("\tStatus date: ");
	  tempTime = (time_t) strtoll(tempSizeBuf, NULL, 10);
	  ts = localtime(&tempTime);
	  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %z (%Z) %a", ts);
	  printf("%s ",timeString);
	  printf(" %s\n",tempSizeBuf);
	  printf("\tMarked deleted: no\n");
	  // Lseek to new achive member entry
	  lseek(fd_ArchiveFile, 96 + OSCAR_HDR_END_LEN,SEEK_CUR);
	  // Lseek past the Data of the File
	  lseek(fd_ArchiveFile,tempSize+1,SEEK_CUR);
	}
      }
      // Close the file descriptor for Archive File
      close(fd_ArchiveFile);
    }else{
      perror(argv[2]);
      printf("Exiting...\n");
      exit(EXIT_FAILURE);
    }
  }
  return (EXIT_SUCCESS);
}

int eFunction(int argc,char *argv[]){
  int fd_ExtractFile;
  int tempSizeConverted;
  char tempExtBuf[OSCAR_MAX_MEMBER_FILE_SIZE];
  char tempDataBuf[OSCAR_MAX_MEMBER_FILE_SIZE];
  char tempSize[OSCAR_FILE_SIZE];
  char temp_aDate[10];
  char temp_mDate[10];
  char tempMode[6];
  struct timespec times[2];
  time_t atime;
  time_t mtime;
  mode_t mode;

  if(argv[2] == NULL){
    puts("*** Archive file not specified");
    puts("*** Exiting...");
    exit(EXIT_FAILURE);
  }
  // If file names are provided
  if (argv[3] != NULL){
    if(access(argv[2], F_OK ) != -1 ) {
      //File exists
      fd_ArchiveFile = open(argv[2], O_RDONLY, 0644);
      // memset tempExtBuf to ' '
      memset(tempExtBuf,' ',sizeof(tempExtBuf));
      //Read the Oscar file header to see if file is good
      read(fd_ArchiveFile,readBufHeader,OSCAR_ID_LEN);
      if(strcmp(readBufHeader,OSCAR_ID) != 0){
	printf("Bad Header in %s\n",argv[2]);
	puts("Exiting...");
	exit(EXIT_SUCCESS);
      }else{
	memset(tempExtBuf,' ',sizeof(tempExtBuf));
	j = 3;
	z = 0;
	while(argv[j] != NULL){
	  printf("Trying to find: %s \n",argv[j]);
	  lseek(fd_ArchiveFile,OSCAR_ID_LEN,SEEK_SET);
	  while(read(fd_ArchiveFile,tempExtBuf,OSCAR_MAX_FILE_NAME_LEN) != 0){
	    tempExtBuf[OSCAR_MAX_FILE_NAME_LEN] = 0;
	    for(i = 0; i < OSCAR_MAX_FILE_NAME_LEN; i++){
	      if(tempExtBuf[i] != ' '){
		//printf("%c\n",tempExtBuf[i]);
	      }else{
		tempExtBuf[i] = 0;
	      }
	    }
	    //printf("%s\n",tempExtBuf);
	    if(strcmp(argv[j],tempExtBuf) == 0){
	      printf("Found %s in oscar archive %s in Round %d\n\n",argv[j],argv[2],z);
	      /* EXTRACT CODE HERE!*/
	      if((access(argv[j], F_OK) != -1) && ((do_Flag == 0) && (o_Flag == 0))){
		//File exsists
		printf("Member file <%s> exsists & no '-o' option selected\n",argv[j]);
		printf("Skiping...\n");
	      }else{
		printf("o flag choosen extracting file %s...\n",argv[j]);
		printf("Member file <%s> created\n",argv[j]);
		fd_ExtractFile = open(argv[j],O_WRONLY | O_CREAT | O_APPEND | O_TRUNC,0644);
		//Set Buffer to ' '
		memset(tempDataBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
		// Get the a_Data and m_Data
		lseek(fd_ArchiveFile,2,SEEK_CUR);
		read(fd_ArchiveFile,temp_aDate,OSCAR_DATE_SIZE);
		atime = (time_t)strtoll(temp_aDate,NULL,10);
		printf("atime is: %d\n",(int)atime);
		read(fd_ArchiveFile,temp_mDate,OSCAR_DATE_SIZE);
		mtime = (time_t)strtoll(temp_mDate,NULL,10);
		printf("mtime is: %d\n",(int)mtime);
		// Lseek to mode
		lseek(fd_ArchiveFile,20,SEEK_CUR);
		read(fd_ArchiveFile,tempMode,OSCAR_MODE_SIZE);
		printf("Mode is %s\n",tempMode);
		mode = (mode_t)strtoll(tempMode,NULL,8);
		// Get the file size
		//lseek(fd_ArchiveFile,26,SEEK_CUR);
		memset(tempSize,' ',OSCAR_FILE_SIZE);
		read(fd_ArchiveFile,tempSize,OSCAR_FILE_SIZE);
		tempSize[OSCAR_FILE_SIZE] = 0;
		tempSizeConverted = strtoll(tempSize,NULL,10);
		printf("Size retrieved is: %d\n",tempSizeConverted);
		// Lseek to Data
		lseek(fd_ArchiveFile,68,SEEK_CUR);
		//Read Data into BUf
		read(fd_ArchiveFile,tempDataBuf,tempSizeConverted);
		// write Date to oped ExtractFile
		write(fd_ExtractFile,tempDataBuf, tempSizeConverted);
		// Modifiy the aData and the mDate 
		times[1].tv_sec = atime;
		times[2].tv_sec = mtime;
		printf("E_Flag is (%d)\n",E_flag);
		if (E_flag == 0){
		  k = futimens(fd_ExtractFile,times);
		  if(k == 0){
		    printf("Futimens updated time stamp of %s from oscar archive\n",argv[j]);
		  }else if( k == -1){
		    perror("Futimens");
		  }
		}
		// Set Permissions
		k = fchmod(fd_ExtractFile,mode);
		if(k == 0){
		  printf("fchmod updated mode of %s from oscar archive\n",argv[j]);
		}else if (k == -1){
		  perror("fchmod");
		}
		
		memset(tempDataBuf,' ',OSCAR_MAX_MEMBER_FILE_SIZE);
	      }
	      lseek(fd_ArchiveFile,0,SEEK_END);
	    }else{
	      printf("Did not find %s in oscar archive %s in Round %d\n",argv[j],argv[2],z);
	      z++;
	      printf("Lseeking past %s file size\n",tempExtBuf);
	      lseek(fd_ArchiveFile,48,SEEK_CUR);
	      memset(tempSize,' ',OSCAR_FILE_SIZE);
	      read(fd_ArchiveFile,tempSize,OSCAR_FILE_SIZE);
	      tempSize[OSCAR_FILE_SIZE] = 0;
	      tempSizeConverted = strtoll(tempSize,NULL,10);
	      printf("Size of %s is %d\n",tempExtBuf,tempSizeConverted);
	      lseek(fd_ArchiveFile,OSCAR_SHA_DIGEST_LEN + OSCAR_HDR_END_LEN,SEEK_CUR);
	      lseek(fd_ArchiveFile,tempSizeConverted + 1,SEEK_CUR);  
	      printf("Checking next Archive entry...\n\n");
	    }
	  }
	  memset(tempExtBuf,' ',sizeof(tempExtBuf));
	  j++;
	  z = 0;
	  printf("\n");
	}
      }
    }else{
      puts("myoscar looking in current directory"); 
      perror(argv[2]);
      printf("Exiting...\n");
    }
  }else{
    printf("No member file arguments provided extracting all files from oscar archive %s\n",argv[2]);
    if((access(argv[2], F_OK ) != -1)){
      //File exists
      fd_ArchiveFile = open(argv[2], O_RDONLY, 0644);
      // memset tempExtBuf to ' '
      memset(tempExtBuf,' ',sizeof(tempExtBuf));
      //Read the Oscar file header to see if file is good
      read(fd_ArchiveFile,readBufHeader,OSCAR_ID_LEN);
      if(strcmp(readBufHeader,OSCAR_ID) != 0){
	printf("Bad Header in %s\n",argv[2]);
	puts("Exiting...");
	exit(EXIT_SUCCESS);
      }else{
	/* Extract all member files */
	// Set tempExtBuf to spaces
	memset(tempExtBuf, ' ', sizeof(tempExtBuf));
	while(read(fd_ArchiveFile,tempExtBuf,OSCAR_MAX_FILE_NAME_LEN) != 0){
	  tempExtBuf[OSCAR_MAX_FILE_NAME_LEN] = 0;
	  for(i = 0; i < OSCAR_MAX_FILE_NAME_LEN; i++){
	    if(tempExtBuf[i] != ' '){
	      //printf("%c\n",tempExtBuf[i]);
	    }else{
	      tempExtBuf[i] = 0;
	      break;
	    }
	  }
	  if((access(tempExtBuf, F_OK ) != -1) && ((o_Flag == 0) && (do_Flag == 0))) {
	    printf("Member file <%s> exsists & no '-o' option selected\n",tempExtBuf);
	    printf("Skiping...\n");
	    //Lseek Past the file
	    // Get the file size
	    memset(tempSize,' ',OSCAR_FILE_SIZE);
	    lseek(fd_ArchiveFile,48,SEEK_CUR);
	    read(fd_ArchiveFile,tempSize,OSCAR_FILE_SIZE);
	    tempSize[OSCAR_FILE_SIZE] = 0;
	    tempSizeConverted = strtoll(tempSize,NULL,10);
	    printf("Size retrieved is: %d\n\n",tempSizeConverted);
	    lseek(fd_ArchiveFile,OSCAR_SHA_DIGEST_LEN + OSCAR_HDR_END_LEN, SEEK_CUR);
	    lseek(fd_ArchiveFile,tempSizeConverted + 1,SEEK_CUR);
	  }else{
	    if( o_Flag == 1  || (do_Flag == 1)){
	      printf("'-o' option selected!\n");
	    }
	    //File exists
	    printf("Extacting info for member file <%s>\n",tempExtBuf);
	    // Get the required details
	    fd_ExtractFile = open(tempExtBuf,O_WRONLY | O_CREAT | O_APPEND | O_TRUNC,0644);
	    //Set Buffer to ' '
	    memset(tempDataBuf,' ', OSCAR_MAX_MEMBER_FILE_SIZE);
	    // Get the a_Data and m_Data
	    lseek(fd_ArchiveFile,2,SEEK_CUR);
	    read(fd_ArchiveFile,temp_aDate,OSCAR_DATE_SIZE);
	    atime = (time_t)strtoll(temp_aDate,NULL,10);
	    printf("atime is: %d\n",(int)atime);
	    read(fd_ArchiveFile,temp_mDate,OSCAR_DATE_SIZE);
	    mtime = (time_t)strtoll(temp_mDate,NULL,10);
	    printf("mtime is: %d\n",(int)mtime);
	    // Lseek to mode
	    lseek(fd_ArchiveFile,20,SEEK_CUR);
	    read(fd_ArchiveFile,tempMode,OSCAR_MODE_SIZE);
	    printf("Mode is %s\n",tempMode);
	    mode = (mode_t)strtoll(tempMode,NULL,8);
	    // Get the file size
	    memset(tempSize,' ',OSCAR_FILE_SIZE);
	    read(fd_ArchiveFile,tempSize,OSCAR_FILE_SIZE);
	    tempSize[OSCAR_FILE_SIZE] = 0;
	    tempSizeConverted = strtoll(tempSize,NULL,10);
	    printf("Size retrieved is: %d\n",tempSizeConverted);
	    // Lseek to Data
	    lseek(fd_ArchiveFile,68,SEEK_CUR);
	    //Read Data into BUf
	    read(fd_ArchiveFile,tempDataBuf,tempSizeConverted);
	    // write Date to oped ExtractFile
	    write(fd_ExtractFile,tempDataBuf, tempSizeConverted);
	    // Modifiy the aData and the mDate 
	    times[1].tv_sec = atime;
	    times[2].tv_sec = mtime;
	    // Default timestamps if E_flag is 0
	    printf("E_flag: (%d) \n",E_flag); 
	    if (E_flag == 0){
	      k = futimens(fd_ExtractFile,times);
	      if(k == 0){
		printf("Futimens updated time stamp of %s from oscar archive\n",argv[j]);
	      }else if( k == -1){
		perror("Futimens");
	      }
	    }
	    // Set Permissions
	    k = fchmod(fd_ExtractFile,mode);
	    if(k == 0){
	      printf("fchmod updated mode of %s from oscar archive\n",argv[j]);
	    }else if (k == -1){
	      perror("fchmod");
	    }
	    memset(tempDataBuf,' ',OSCAR_MAX_MEMBER_FILE_SIZE);
	    printf("\n");
	  }
	}
      }
    }else{
      puts("myoscar looking in current directory"); 
	perror(argv[2]);
	printf("Exiting...\n");
    }
  } 
  return(EXIT_SUCCESS);
}

/*
 * Helper functions for d option
 */
void strip_char(char *str, char strip)
{
  char *p, *q;
  for (q = p = str; *p; p++)
    if (*p != strip)
      *q++ = *p;
  *q = '\0';
}


int dSearch(char* search[],char* value){
  i = 3;
  j = 0;
  strip_char(value,' ');
  while(search[i] != NULL){
    printf("Attempt (%d) to find <%s> in delete members argv[...]\n",j,value);
    j++;
    if(strcmp(search[i], value) == 0){
      printf("Exiting dSearch(...) function <%s> found!\n", value);
      return 0;
    }else{
      i++;
    }
  }
  printf("Exiting dSearch(...) function nothing found!\n");
  return -1;
}


/*
 * Delete Member function
 */

int dFunction(int argc, char *argv[]){
  char tempDataBuf[OSCAR_MAX_MEMBER_FILE_SIZE + 200];
  char tempSize[OSCAR_FILE_SIZE];
  size_t tempSizeConverted;
  // open a tempArchive 
  fd_tempArchiveFile = open("tempArchiveFile.oscar",O_RDWR |O_CREAT | O_APPEND,0644);
  write(fd_tempArchiveFile,OSCAR_ID,OSCAR_ID_LEN);
  if (argv[2] != NULL){
    // Check Archive File exsists
    if((access(argv[2], F_OK ) != -1)){
      // Check for delete member specification
      if(argv[3] != NULL){
	//Open Archive File for Reading and Writing
	fd_ArchiveFile = open(argv[2], O_RDWR, 0644);
	memset(tempDataBuf,' ',sizeof(tempDataBuf));
	memset(readBufHeader,' ',sizeof(readBufHeader));
	read(fd_ArchiveFile,readBufHeader,OSCAR_ID_LEN);
	
	if(strcmp(readBufHeader,OSCAR_ID) != 0){
	  printf("Bad Header in %s\n",argv[2]);
	  puts("Exiting...");
	  exit(EXIT_SUCCESS);
	}else{
	  puts("assinging ' ' for tempDataBuf");
	  memset(tempDataBuf,' ',sizeof(tempDataBuf));
	  //Start to find the stated files 
	  i = 3;
	  while(read(fd_ArchiveFile,tempDataBuf,OSCAR_MAX_FILE_NAME_LEN) != 0){
	    tempDataBuf[OSCAR_MAX_FILE_NAME_LEN] = 0;
	    //printf("Entering while loop for %s\n", argv[i]);
	    if(dSearch(argv,tempDataBuf) == 0){
	      // This file needs to be deleted therefore Skip
	      printf("%s file choosen to be deleted therefore skipping...\n",argv[i]);
	      memset(tempSize,' ',OSCAR_FILE_SIZE);
	      // Find file size and skip over the file.
	      lseek(fd_ArchiveFile,48,SEEK_CUR);
	      read(fd_ArchiveFile,tempSize,OSCAR_FILE_SIZE);
	      tempSize[OSCAR_FILE_SIZE] = 0;
	      tempSizeConverted = (size_t) strtoll(tempSize,NULL,10);
	      printf("Size retrieved is: %d\n",(int)tempSizeConverted);
	      lseek(fd_ArchiveFile,OSCAR_SHA_DIGEST_LEN + OSCAR_HDR_END_LEN + 1, SEEK_CUR);
	      lseek(fd_ArchiveFile,tempSizeConverted,SEEK_CUR);
	    }else{
	      printf("Writing %s file to tempArchiveFile.oscar\n",tempDataBuf);
	      lseek(fd_ArchiveFile,48,SEEK_CUR);
	      read(fd_ArchiveFile,tempSize,OSCAR_FILE_SIZE);
	      tempSize[OSCAR_FILE_SIZE] = 0;
	      tempSizeConverted = (size_t) strtoll(tempSize,NULL,10);
	      printf("Size retrieved is: %d\n",(int)tempSizeConverted);
	      lseek(fd_ArchiveFile,-48,SEEK_CUR);
	      lseek(fd_ArchiveFile,-46,SEEK_CUR);
	      read(fd_ArchiveFile,tempDataBuf,162 + tempSizeConverted);
	      write(fd_tempArchiveFile, tempDataBuf,162 + tempSizeConverted);
	    }
	    i++;
	  }
	}
      }else{
	printf("You must specify a file to delete from %s\n",argv[2]);
	puts("*** Exiting...");
      }
    }else{
      printf("No file name %s found!\n",argv[2]);
    }
  }else{
    puts("*** Archive file not specified");
    puts("*** Exiting...");
    exit(EXIT_FAILURE);
  }
  
  strcat(popenCmd,"mv tempArchiveFile.oscar ");
  strcat(popenCmd,argv[2]);
  printf("Proccessing mv command for popen: <%s>\n",popenCmd);
  file = popen(popenCmd,"w");
  perror("popen mv");
  pclose(file);
  return (EXIT_SUCCESS);
}


void V_Function(void){
  struct stat buf;
  if(lstat("./myoscar.c",&buf) == 0){
    puts("Version: Lost count!");
    printf("File myoscar.c details: \n");
    printf("Last Accessed: %s",ctime(&buf.st_atime));
    printf("Last Modified: %s",ctime(&buf.st_mtime));
  }
}

int main(int argc, char* argv[]){
  /* 
   * Main Function call
   */
  umask(0);
  handleArgv(argc,argv);
  exit(EXIT_SUCCESS);
}

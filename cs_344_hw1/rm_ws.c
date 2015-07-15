/* Author: Kamil Agrawala
 * Email: agrawalk@onid.orst.edu
 * Class: Cs344-001
 * Assingment #1
 * Programming Projects rm_ws.c file
 */

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getopt is in unistd.h*/
#include <getopt.h>    /* another library for getopt*/
#include <string.h>    /* general use for C strings  */
#include <ctype.h>     /* for addtional libary functions*/


#define BUF_SIZE 8096
#define FINAL_SIZE 8096

int option = 0;
int b_flag = 0; 
int e_flag = 0;
int i_flag = 0;
int white_space_flag = 0;
char str[BUF_SIZE];  


void flagFunction(void);

void flagFunction(void){
  /*
    For holding spaces in front on string
  */
  char output[BUF_SIZE]="";	
  /*
    For holding the actual String
  */
  char output2[BUF_SIZE]="";
  /*
    For Holding trailing spaces. 
  */
  char output3[BUF_SIZE]="";
  /*
   * For holding alpha numeric content of string. 
   */
  char output4[BUF_SIZE]="";
  
  int i, j, k, start, spaces, end;
    
  
  if ((b_flag != 0) || (i_flag != 0) || (e_flag != 0 )){
    
    while(fgets(str,BUF_SIZE,stdin)){
      if(str[strlen(str)-1] == '\n'){
	str[strlen(str)-1] = 0;
      }
      
      /*
       * Find the index of first alpha numeric charector
       */
      for(i = 0, start = 0; i < strlen(str); i++){
	if(isalpha(str[i]) || isdigit(str[i]) || ispunct(str[i])){
	  start = i;	
	  //printf("start = %d\n",start);
	  break;
	}
      }
      
      /*
       * Find the index of the last alpha numeric charector
       */
      for(i = strlen(str), end = 0; i >=  0; i--){
	if(isalpha(str[i]) || isdigit(str[i]) || ispunct(str[i])){
	  end = i;	
	  //printf("end = %d\n",end);
	  break;
	}
      }
      
      /*
       * Get the whitespace array at start of string.
       */
      if (b_flag == 0){
	int z = 0;
	if(str[0] != 0){
	  for(i = 0; i < start; i++){
	    output[z] = str[i];
	    z++;
	  }
	}
	output[z] = 0;
      }
      
      /*
       * Get tab and spaces at the end of string.
       */
      
      if(e_flag == 0){
	k = 0;
	for(i = end+1; i <= strlen(str); i++){
	  if(str[i] != 0){
	    output3[k] = str[i];
	    spaces = k;
	    k++;
	  }
	}
	output3[k] = 0;
      }
      
      if(i_flag != 0){  
	/*
	 * Remove spaces in between the String.
	 */ 
	for(i = 0, j = 0;i <= strlen(str); i++, start++){ 
	  if((!isspace(str[start]) && str[start] != '\t')){	      
	    output2[j] = str[start];
	      j++;
	  }
	}
      }

      if(e_flag != 0){
	for(i = start,j = 0; i <= end; i++){
	  if(str[i] != 0){
	    output4[j] = str[i];
	    j++;
	  }
	  output4[j] = 0;
	}
      }
	
      /*
	Organize the output
      */
      if(b_flag == 1 && i_flag == 0 && e_flag==0){
	while(( str[0] == ' ') || (str[0] == '\t')){
	  // Restructure String so that leading tabs and spaces are removed.
	  memmove(&str,&str[0+1],strlen(str) - 0);
	  strcpy(output,str);
	}
	puts(str);
      }
	
      if(b_flag == 0 && i_flag == 1 && e_flag == 0){
	strcat(output,output2);
	strcat(output,output3);
	puts(output);
      } 

      if(b_flag == 0 && i_flag == 0 && e_flag == 1){
	strcat(output,output4);
	puts(output);
      }

      if(b_flag == 1 && i_flag == 1 && e_flag == 0){
	strcat(output2,output3);
	puts(output2);
      }
      
      if(b_flag == 1 && i_flag == 0 && e_flag == 1){
	puts(output4);
      } 
	
      if(b_flag == 0 && i_flag == 1 && e_flag == 1 ){
	strcat(output,output2);
	puts(output);
      }
	
      if(b_flag == 1 && i_flag == 1 && e_flag == 1){
	puts(output2);
      }
    }  
  }else{
    puts("You must give parameters [-b, -i -e ] (or any combination)");
    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char* argv[]){  
  while ((option = getopt(argc, argv,"bei")) != -1) {
    switch (option) {
    case 'b' : 
      b_flag = 1;
      fputs("b flag triggered\n",stderr);
      break;
    case 'i' : 
      i_flag = 1;
      fputs("i flag triggered\n",stderr);
      break;
    case 'e' : 
      e_flag = 1;
      fputs("e flag triggered\n",stderr);
      break;
    default: printf("Usuage: option [-b] or [-i] or [-e] and provide stdin ");
      exit(EXIT_FAILURE);
    }
  }
  
  flagFunction();
  
  exit(EXIT_SUCCESS);  
}

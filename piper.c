
/***********************************************************************************************

 CSci 4061 Fall 2017
 Assignment# 3: Piper program for executing pipe commands

 Student name: <Hanei Moubarak>
 Student ID:   <5215277>

 Student name: <Abdelrahman Elfaramawy>
 Student ID:   <5171605>

 X500 id: <mouba005>, <elfar009>

 Operating system on which you tested your code: Linux, Unix
 CSELABS machine: <csel-kh1250-11.cselabs.umn.edu>

 GROUP INSTRUCTION:  Please make only ONLY one  submission when working in a group.
***********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define DEBUG
#define MAX_INPUT_LINE_LENGTH 2048 // Maximum length of the input pipeline command
                                   // such as "ls -l | sort -d +4 | cat "
#define MAX_CMDS_NUM   8           // maximum number of commands in a pipe list
                                   // In the above pipeline we have 3 commands
#define MAX_CMD_LENGTH 2048        // A command has no more than 4098  characters



FILE *logfp;

int num_cmds = 0;
int fildes[2],pipe2[2];
char *cmds[MAX_CMDS_NUM];
int cmd_pids[MAX_CMDS_NUM];
int cmd_status[MAX_CMDS_NUM];
char *cmdsMemo[MAX_CMDS_NUM];


/*******************************************************************************/
/*   The function parse_command_line will take a string such as
     ls -l | sort -d +4 | cat | wc
     given in the char array commandLine, and it will separate out each pipe
     command and store pointer to the command strings in array "cmds"
     For example:
     cmds[0]  will pooint to string "ls -l"
     cmds[1] will point to string "sort -d +4"
     cmds[2] will point to string "cat"
     cmds[3] will point to string "wc"

     This function will write to the LOGFILE above information.
*/
/*******************************************************************************/

int parse_command_line (char commandLine[MAX_INPUT_LINE_LENGTH], char* cmds[MAX_CMDS_NUM]){

  int  num = 0;
      char delims[] = "|";
      char *result = NULL;
      result = strtok( commandLine, delims );

      fprintf(logfp,"Extracting commands from the pipeline...\n");
      while( result != NULL ) {
          cmds[num] = result;
          fprintf(logfp,"Extracted : \"%s\"\n", result );
          result = strtok( NULL, delims );
          num++;
      }

     return num;

}

/*******************************************************************************/
/*  parse_command takes command such as
    sort -d +4
    It parses a string such as above and puts command program name "sort" in
    argument array "cmd" and puts pointers to ll argument string to argvector
    It will return  argvector as follows
    command will be "sort"
    argvector[0] will be "sort"
    argvector[1] will be "-d"
    argvector[2] will be "+4"
/
/*******************************************************************************/

void parse_command(char input[MAX_CMD_LENGTH],
                   char command[MAX_CMD_LENGTH],
                   char *argvector[MAX_CMD_LENGTH]){
  int i = 0;
  char* delim = " \t\n\0";
  argvector[i] = strtok(input, delim);
  strcpy(command, argvector[i]);
  i++;
  while ((argvector[i] = strtok(NULL, delim)) != NULL){
    i++;
  }
}


/*******************************************************************************/
/*  The function print_info will print to the LOGFILE information about all    */
/*  processes  currently executing in the pipeline                             */
/*  This printing should be enabled/disabled with a DEBUG flag                 */
/*******************************************************************************/
//    printf("%-30s %08x %8d\n", names[i], addresses[i], sizes[i]);

void print_info(char* cmdMemo[MAX_CMDS_NUM],
        int cmd_pids[MAX_CMDS_NUM],
        int cmd_stat[MAX_CMDS_NUM],
        int num_cmds) {


        #ifdef DEBUG

        fprintf(logfp,"\n%-30s %-30s\n", "PID", "COMMAND");

        for(int i=0; i<num_cmds; i++)
        {
          
            fprintf(logfp,"%-30d  %-30s\n", cmd_pids[i] , cmdMemo[i]);
        }

        #endif
}

/*******************************************************************************/
/*  The function print_info will print to the LOGFILE information about all    */
/*  processes  currently executing in the pipeline                             */
/*  This printing should be enabled/disabled with a DEBUG flag                 */
/*******************************************************************************/

void print_info_status(char* cmdMemo[MAX_CMDS_NUM],
        int cmd_pids[MAX_CMDS_NUM],
        int cmd_stat[MAX_CMDS_NUM],
        int num_cmds) {


        #ifdef DEBUG

        fprintf(logfp,"\n%-30s %-30s %-30s\n", "PID", "COMMAND", "STATUS");

        for(int i=0; i<num_cmds; i++)
        {
          
            fprintf(logfp,"%-30d  %-30s %-30d\n", cmd_pids[i] , cmdMemo[i], cmd_stat[i]);
        }

        #endif

}

/*******************************************************************************/
/*     The create_command_process  function will create a child process        */
/*     for the i'th command                                                    */
/*     The list of all pipe commands in the array "cmds"                       */
/*     the argument cmd_pids contains PID of all preceding command             */
/*     processes in the pipleine.  This function will add at the               */
/*     i'th index the PID of the new child process.                            */
/*     Following ADDED on  10/27/2017                                          */
/*     This function  will  craete pipe object, execute fork call, and give   */
/*     appropriate directives to child process for pipe set up and             */
/*     command execution using exec call                                       */
/*******************************************************************************/


void create_command_process (char cmds[MAX_CMD_LENGTH],   // Command line to be processed
                     int cmd_pids[MAX_CMDS_NUM],          // PIDs of preceding pipeline processes
                                                          // Insert PID of new command processs
                     int i)                               // commmand line number being processed
{

    int statid,  j=0;
    int valexit,  valsignal, valstop, valcont;

    char just_cmd[MAX_CMD_LENGTH];
    char *argvec[MAX_CMD_LENGTH];

    parse_command(cmds, just_cmd, argvec);  // put command in just_cmd  and all arguments in argvec

    if(cmd_pids[i]=fork()){ /*  Parent process */

    /* ..Close the unwanted pipe descriptors.. */
    close(fildes[0]);
    close(fildes[1]);
    }
    else { /* Newly created child process */

    if(fildes[0] != -1){
        /* .. indicates that standard input comes from a file .. */
        dup2(fildes[0],0);
        }
    if(pipe2[1] != -1){
        /* ..indicates that standard output redirect to a pipe.. */
        dup2(pipe2[1],STDOUT_FILENO);
    }

    close(fildes[0]);
    close(fildes[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    if(execvp(just_cmd,argvec)){
            exit(1);
        }
    }

    fildes[0] = pipe2[0];
    fildes[1] = pipe2[1];

    pipe2[0]=-1;
    pipe2[1]=-1;

}


/********************************************************************************/
/*   The function waitPipelineTermination waits for all of the pipeline         */
/*   processes to terminate.                                                    */
/********************************************************************************/

void waitPipelineTermination () {
  int finishCount = 0;
  int  signalInterrupt = 0;  // keep track of any iinterrupt due to signal
  int fatalError = 0;
  int statid = 0 ;
  int status = 0 ;
  int i;

  ////printf("\nwaiting for pipeline termination...");
  while (finishCount < num_cmds ) {
   fprintf(logfp,"\nwaiting...");

   if((statid=wait(&status))==-1){
      perror("Wait terminated:");
      signalInterrupt = 1;
      break;
      }

  if (signalInterrupt) {
   continue;
      }
  for (i=0; i<num_cmds; i++) {
   if (cmd_pids[i]==statid)  {
       cmd_status[i] = status;
       finishCount++;   // Only count those processes that belong to the pipeline
       fprintf(logfp,"Process id %d finished", cmd_pids[i]);
       fprintf(logfp,"\nProcess id %d finished with exit status %d", cmd_pids[i], cmd_status[i]);
    }
    }
  if ( WEXITSTATUS(status) != 0 ) {
     fatalError = 1;
     fprintf(logfp,"Terminating pipeline becuase process %d failedto execute\n", statid);
     break;
      }
  }

  if ( fatalError ) {
  for (i=0; i<num_cmds; i++) {
   fprintf(logfp,"Terminating process %d \n", cmd_pids[i] );
   kill(cmd_pids[i], 9);
  }
  }

}

/********************************************************************************/
/*  This is the signal handler function. It should be called on CNTRL-C signal  */
/*  if any pipeline of processes currently exists.  It will kill all processes  */
/*  in the pipeline, and the piper program will go back to the beginning of the */
/*  control loop, asking for the next pipe command input.                       */
/********************************************************************************/

 void killPipeline( int signum) {

   for(int i=0; i<num_cmds; i++)
   {
       kill(cmd_pids[i],SIGQUIT);
   }
 }

/********************************************************************************/

int main(int ac, char *av[]){

  int i,  pipcount;
  //check usage
  if (ac > 1){
    printf("\nIncorrect use of parameters\n");
    printf("USAGE: %s \n", av[0]);
    exit(1);
  }

  /* Set up signal handler for CNTRL-C to kill only the pipeline processes  */

  logfp =  fopen("LOGFILE", "a");


  while (1) {
     signal(SIGINT, SIG_DFL );
     pipcount = 0;

     //initializations
     fildes[0]=-1;
     fildes[1]=-1;
     pipe2[0] = -1;
     pipe2[1] = -1;

     /*  Get input command file anme form the user */
     char pipeCommand[MAX_INPUT_LINE_LENGTH];

     fflush(stdout);
     printf("Give a list of pipe commands: ");

     gets(pipeCommand);
     char* terminator = "quit";
     fprintf(logfp,"\nYou entered : list of pipe commands  %s\n", pipeCommand);
     if ( strcmp(pipeCommand, terminator) == 0  ) {
        fflush(logfp);
        fclose(logfp);
        printf("\nGoodbye!\n");
        exit(0);
     }

    num_cmds = parse_command_line( pipeCommand, cmds);

    /*  SET UP SIGNAL HANDLER  TO HANDLE CNTRL-C                         */
    signal(SIGINT, killPipeline);

    /*  num_cmds indicates the number of commands in the pipeline        */

    /* The following code will create a pipeline of processes, one for   */
    /* each command in the given pipe                                    */
    /* For example: for command "ls -l | grep ^d | wc -l "  it will      */
    /* create 3 processes; one to execute "ls -l", second for "grep ^d"  */
    /* and the third for executing "wc -l"                               */

    for(i=0;i<num_cmds;i++){
        //cmdsMemo[i] = cmds[i];
        cmdsMemo[i] = (char *) malloc(2048 * sizeof(char));
        //for (int j=0; j< sizeof(cmds[i])/sizeof(char); j++){
            strncpy(cmdsMemo[i], cmds[i], 2047);
        //}
         /*  CREATE A NEW PROCCES EXECUTTE THE i'TH COMMAND    */
         /*  YOU WILL NEED TO CREATE A PIPE, AND CONNECT THIS NEW  */
         /*  PROCESS'S stdin AND stdout  TO APPROPRIATE PIPES    */
         fprintf(logfp,"\nCommand %d info: %s", i,  cmds[i]);
            if(pipcount < num_cmds-1) { //we do not need pipe for the last command. So, num_cmds-1
                 pipe(pipe2);
               pipcount++;
            }
         create_command_process (cmds[i], cmd_pids, i);
    }

    fprintf(logfp,"\nNumber of commands from the input: %d", num_cmds);

    print_info(cmdsMemo, cmd_pids, cmd_status, num_cmds);

    waitPipelineTermination();

    print_info_status(cmdsMemo, cmd_pids, cmd_status, num_cmds);

  }
  fclose(logfp);
} //end main

/*************************************************/


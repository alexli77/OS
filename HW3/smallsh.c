#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <assert.h>

/******************************************************************************************
outline for the script:
1. need to declear the varibles
2. numcot funtion/buildin fution
3.in main()
 a.declear varibles
 b.funtion to stop with ctrl+C
 c.set end
 d.loop for typein
    :> =< something like this, make funtion
    :pid funtion
    :buffer
    :clean
**********************************************************************************************/

// Global Variables


    struct sigaction sigINT_action = {0};
    struct sigaction sigTSTP_action = {0};
    int permitBackground = 1;
/*********************************************************************************************/
//herlper funtion

    void showStatus(int status){
    if(WIFEXITED(status)){          //Taken from lecture
        printf("\nexit value %i\n", WEXITSTATUS(status));
    } 
    else{
        printf("terminated by signal %i\n", status);
    }
}


//crtl+c include the sleep funtion.
void sigINT(int signal) {
  printf("\nCaught SIGINT.\n");
  sigINT_action.sa_handler = SIG_IGN; // Set to ignore
  sigaction(SIGINT, &sigINT_action, NULL);
}
//source from http://www.csl.mtu.edu/cs4411.choi/www/Resource/signal.pdf
 
// This function checks for Ctrl-Z
void sigTSTP(int signo) {
  char *message = "\nCaught SigStop.\n";
  write(STDOUT_FILENO, message, 25);
} //from lecture

void catchsig(){  // To catch signals,taking from the lecture. 
  sigINT_action.sa_handler = SIG_IGN; // Set to ignore
  sigaction(sigINT, &sigINT_action, NULL);
    
  sigTSTP_action.sa_handler = SIG_IGN; 
  sigTSTP_action.sa_handler = sigTSTP;
  sigaction(sigTSTP, &sigTSTP_action, NULL);}



/***************************************************************************************
                    end of code about the signals 
****************************************************************************************/




int main(){             
      //list the varibles are in main funtion 

    char getInput[512];
    char* argument[512];
    char* sl;
    char* inputFile = 0;  
    char* outputFile = 0;    
    int wFile = -5;          
    int rFile = -5;          
    int frontP;     
    int status = 0;
    int cpid;
    
    
catchsig();
    
    
/***************************************************************************************
                    user input
 *****************************************************************************************/
  //set a while loop to run, make sure I can type many times until hits exit.
    while(1){
        frontP = 1;      
          
        //if only next prompt is displayed next(5pts)

        printf(": ");       
        fflush(stdout);     

            // Reached end, exit funtion,taking from the lecture 

        if(fgets(getInput, 400, stdin) == NULL){   //Reached the end
            return 0;
        }

      /*****************************************************************************************
                       special input
******************************************************************************************/
      
   //on the grade script, it shows like wc < junk > junk2. So we can defince the funtion with those non chartere.  
        
        
        
        int numcot = 0;                     
        sl = strtok(getInput, " \n");           //Parse the user input for instructions
        while(sl != NULL){
            if(strcmp(sl, "<") == 0){           //This means input file     
                sl = strtok(NULL, " \n");        //Parse and store
                inputFile = strdup(sl);
                sl = strtok(NULL, " \n");
            } 
            else if(strcmp(sl, ">") == 0){       //This is an output file so perform same action and save to outputFile
                sl = strtok(NULL, " \n");
                outputFile = strdup(sl);
                sl = strtok(NULL, " \n");
            } 
            else if(strcmp(sl, "&") == 0){       //Background process
                frontP = 0;
                break;
            } 
            else{
                argument[numcot] = strdup(sl);   //Save pointer to instruction in argument array at the position of numcot
                sl = strtok(NULL, " \n");
                ++numcot;                           //Increment our numcot to get next place
            }
        }
        argument[numcot] = NULL;   
        
        /*****************************************************************************************
                     end of special input
******************************************************************************************/
        
        
        
        /****************************************************************************************
                      getpid
*****************************************************************************************/
//from the grade script, we know we need to getpid, and also from $$.
char *pch;
  int i = 0;
  int j = 0;
  int argumentLength = 0;
  while(argument[i] != NULL) {
    pch = strstr(argument[i], "$$");
    if(pch != NULL) {
      argumentLength = strlen(argument[i]);
      char *buffer = malloc(sizeof(char) + (argumentLength + 5));
      char *tmp1 = malloc(sizeof(char) + (argumentLength + 5));

      while(argument[i] + sizeof(char) * j != pch) {
        tmp1[j] = argument[i][j];
        j++;
      }

      sprintf(buffer, "%s%ld", tmp1, (int) getpid);
      argument[i] = buffer;
    }
    i++;
  }

//from piazza and chegg.com. change some parts.
        
/*************************************************************************************
                        end of pid
*****************************************************************************************/
        
        
        
/*****************************************************************************************
                       buffer input
******************************************************************************************/        
        
        

        if(argument[0] == NULL || *(argument[0]) == '#'){            //if input does not make any sencse do nothing
            ;
        }
        else if(strcmp(argument[0], "cd") == 0){            
            if(argument[1] == NULL){                        //Else, change to the specifisd dir
                chdir(getenv("HOME"));
            } 
              else {
                       char cwd[256];                  //return to home, reqire in the granding script
                        char newDirectory[256];            
                        getcwd(cwd, sizeof(cwd));

                    if(strcmp(argument[1], "..") != 0) {
                    sprintf(newDirectory, "%s/%s", cwd, argument[1]);
    }
                    else {
                                strcpy(newDirectory, "..");  //to new dir
    }
                        if(chdir(newDirectory) != 0) {           //find children file
                    printf("cd: No such file or directory. \n");
    }
  }
        } 
        else if(strcmp(argument[0], "status") == 0){        //Print the status
            showStatus(status);
        }
        
        else if (strcmp(argument[0], "exit") == 0){     //User wants to quit
            exit(0);
        } 
        
        /*****************************************************************************************************
         below are the buildin funtions, some futnions are from the lecture, like the outline of switch, funtion
         like perror("dup2");open("/dev/null", O_RDONLY); etc
        ******************************************************************************************************/
        
        
        else{
            cpid = fork();                      //for cip may want to use pid_t cpid. 
            switch(cpid){
            case 0:                     //before I used if, else if and if again, but something funtion does not work
                    //this is from the lecture. I took from it. using case is beffer than if else if and if.
                    
                    
                    
                    
                if(frontP){          //still can use !background,same futnion, front test
                    
                    sigINT_action.sa_handler = SIG_DFL;
                    sigINT_action.sa_flags = 0;
                    sigfillset(&sigINT_action.sa_mask);
                    sigaction(SIGINT, &sigINT_action, NULL);
                }                
                if(inputFile != NULL){    //funtion for open the file, tell user cannot be opened if is invlad
                    //this is reqired in grading script
                    wFile = open(inputFile, O_RDONLY);
                    if(wFile == -1)            // Could not open file

                    {
                        printf("smallsh: cannot open %s for input\n", inputFile);
                        fflush(stdout);
                        exit(1);
                    }
                    if(dup2(wFile, 0) == -1){
                        fprintf(stderr, "DUP2 error\n");
                        exit(1);
                    }
                    close(wFile);      //Need to close since it was opened
                } 
                    
                    //start read the background mode
                    
                else if(!frontP){
                    wFile = open("/dev/null", O_RDONLY);       //Redirect to null device to ignore information
                    if(wFile == -1)            // Could not open file

                    {
                            // Could not open file
                     fprintf(stderr, "Could not open.\n");
                     exit(1);
                    }
                    else if(dup2(wFile, 0) == -1){
                             fprintf(stderr, "DUP2 error\n");
                             exit(1);
                    }
                }                                    //may use when using ec<junk>Junk2

                    // this part is similar to the fornt mode, just copy the code above.
                    
                    /********************************************************************/
                    
                    // If output file exists

                if(outputFile != NULL){                         
                    rFile = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);          // Open the file

                    if(rFile == -1) {
                        printf(" cannot open %s\n", outputFile);
                        fflush(stdout);
                        exit(1);
                    }
                    if(dup2(rFile, 1) == -1){
                        perror("dup2");
                        exit(1);
                    }
                    close(rFile);
                }
                

                if(execvp(argument[0], argument)){         //Command is not recognized so inform the user and change exit status
                    printf("smallsh: Sorry, command \"%s\" is not a valid command\n", argument[0]);
                    fflush(stdout);
                    _Exit(1);
                }
                break;
              
            //if there was a fork() error
        
            case -1:    
                perror("fork");
                status = 1;
                break;
              
              // The parent

            default:    
                if (frontP){                      
                              // If process isn't in the background, wait for foreground process to finish

                    waitpid(cpid, &status, 0);
                } 
                else{                    
                    printf("Background PID: %d\n", cpid);
                    break;
                }
            }

        }
        
        /*****************************************************************************************************
         end of the buildin funtions
        ******************************************************************************************************/
        
        for(int i = 0; argument[i] != NULL; i++){
            free(argument[i]);
        }
           
        free(inputFile); 
        free(outputFile);
        inputFile = NULL;
        outputFile = NULL;
        // clean and free the memory
        
        
              // Check to see if background processes have completed

        cpid = waitpid(-1, &status, WNOHANG);           // taking this futnion from lecture. 
        while(cpid > 0){
                    // Show that process is completed

            printf("Background PID %d is completed. \n", cpid);
            showStatus(status);
            cpid = waitpid(-1, &status, WNOHANG);
        }
    }

    return 0;
}
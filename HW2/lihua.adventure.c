#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// we need to set up the room first, just copy the things from buildrooms.c
char* roomList[10] = { "blue",
        "red",
        "purple",
        "pink",
        "green",
        "black",
        "orange",
        "silver",
        "yellow",
        "white"};   //Same room names and room types
char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};

struct Room{            
    char* name;
    char* type;
    int connections[6];
    int numConnections;    
};

//end of room info, we use int instead of struct here because we want the numbers.


// call the funtion first, the time funtion should always wrok.
//I do not know why there is no time showing without this calls.
void* getTime();            
void showtime();
void threading();



/************************************************************************

  get the time funtion works here

*************************************************************************/

pthread_mutex_t mutex;

void * getTime()
{
	FILE *fileptr;
	fileptr=fopen("currentTime.txt","w+"); //to write time into floder 
	struct tm *ptr;
	time_t t;
	char str[100];
	memset(str,'\0',sizeof(str));    //allcoate space from memory
	t=time(NULL);                     //set time to 0
	ptr=localtime(&t);                 //get the local tine
	strftime(str,100,"%I:%M%p, %A, %B %d, %G\n",ptr); //print the time
	fputs(str,fileptr);
	fclose(fileptr); //save and close the folder
	return NULL;
}

void  showtime()
{
	char theTime[100];
	FILE *myFile;
	myFile=fopen("currentTime.txt","r");  
	if(myFile==NULL)          
	{
		printf("ERROR\n");
	}
	else
	{
		fgets(theTime,100, myFile);     //write the time into folder
		printf("\n%s\n",theTime);
	}	
	fclose(myFile);
}
 void threading() {                               
     pthread_t threaded;                            
     pthread_mutex_init(&mutex, NULL);
     pthread_mutex_lock(&mutex);
     
     int tid = pthread_create(&threaded, NULL, getTime, NULL);
     pthread_mutex_unlock(&mutex);
     pthread_mutex_destroy(&mutex);
     
     usleep(50);
}
//the source of threading funtion are from https://computing.llnl.gov/tutorials/pthreads


/***************************************************************************************

finished the gettime funtion, next part is to read from the files, and get it into the game

***************************************************************************************/

char* getFolder(){
  time_t t;
	int i=0;
	
	int newestDirTime=-1;                           
	char tar[32] = "lihua.rooms.";
	DIR * dirToCheck;
    dirToCheck = opendir(".");
	char *folderName= malloc(sizeof(char) * 64);
	memset(folderName, '\0', sizeof(folderName));
	struct dirent *dir;
	struct stat ptr;
	if(dirToCheck >0)
	{
		while((dir= readdir(dirToCheck))!=NULL)
		{
			if(strstr(dir->d_name,tar)!=NULL)
			{
        			stat(dir->d_name, &ptr);
				if ((int)ptr.st_mtime > newestDirTime)
				{
					newestDirTime = (int)ptr.st_mtime;
          				memset(folderName, '\0', sizeof(folderName));
          				strcpy(folderName, dir->d_name);
        			}
      			}
    		}
  	}
    // the outline of the getFolder is from the pizza and copy from  Lecture minus


  closedir(dirToCheck);
return folderName;
}

//now we set the place where start to read, next we will read the room files and conver to the array that we can use.
// It is the step to set up the room files, and we can read every lines from it. 

int setRooms(struct Room* room, char roomName[128]){        
  char folder[128];            
  sprintf(folder, getFolder());     //copy from the folder using the getfolder funtion                        
  char fileName[128];
  sprintf(fileName, "./%s/%s", folder, roomName);
  FILE* myFile;                         
  myFile = fopen(fileName, "r");        // fopen to read the file

  //we need to inti the first room and it connections.
  room -> name = roomList[0];           
  room -> numConnections = 0;           
    
    
  //read from the line, the name is in the first line, so do not worry ohter lines here
  char myLine[100];                   
  fgets(myLine, 100, myFile);         
  
  char* name = strtok(myLine, " ");   //strtok funtion to break the line, this is copied from lecture.
  name = strtok(NULL, " ");          //strtok funtion to break the line, this is copied from lecture.
  name = strtok(NULL, "\n");            //strtok funtion to break the line, this is copied from lecture.
  
    
  int i;
  for(i = 0; i < 10; i++){              
    if(strcmp(name, roomList[i]) == 0){
      room -> name = roomList[i];
      break;
    }
  }  
  // This part is to avoid get the same room. if it is not same,then we break it. 
   
    
 // we already read the first line and setup the room now. Next, we need to read the other lines which are conncetions.
// we just basicly copy from above and make it into a loop.
  while(fgets(myLine, 100, myFile) != NULL){  
    char* search = strtok(myLine, " ");
    if(strcmp(search, "CONNECTION") == 0){    
      search = strtok(NULL, " ");
      search = strtok(NULL, "\n");
      for(i = 0; i < 10; i++){                  
        if(strcmp(search, roomList[i]) == 0){
          room -> connections[room -> numConnections] = i;
          break;
        }
      }
      room -> numConnections++;                 
    }
    else{
      search = strtok(NULL, " ");             //If not found search by the room type and set that
      search = strtok(NULL, "\n");
      for(i = 0; i < 10; i++){
        if(strcmp(search, roomType[i]) == 0){
          room -> type = roomType[i];
          break;
        }
      }
    }
  }
  return 0;                                    
}
/********************************************************************************************************************

Play funtion

********************************************************************************************************************/

//need to set a funtion that check the correct name or not.

int checkname(struct Room* room, char roomName[50]){
  int i;
  for(i = 0; i < room -> numConnections; i++) //setup a loop
  {     
    if(strcmp(roomName, roomList[room -> connections[i]]) == 0) //to check if the name is correct or not.
    {
      return 1;
    }
  }
  return 0;
}
//set a funtion to run the game 

//Function to get the starting room, to tell where the game start, the important is to search th start room from files.
const char* firstRoom() //use a constant number here because there is only one start room per game.
{                        
    
    
  char* directory = getFolder();
  char* start = malloc(sizeof(char) * 16); //allocate from the memory
  
  DIR* folder;                                  
  struct dirent* search;          //call the search funtion
  
  if((folder = opendir(directory)) != NULL){
    while((search = readdir(folder)) != NULL){
      if(!strcmp(search -> d_name, "..") || !strcmp(search -> d_name, ".")){        
        continue;
      }
      strcpy(start, search -> d_name);            //search to start 
      return start;                               //return to start
      printf("%s", search -> d_name);
      break;
    }
    closedir(folder);                           //Close and free variable
    free(folder);
  }
}

void gamePlay(char start[100]){          //Function to start the game
  int steps = 0;                        //we need to set steps to 0 first.
  int ganeover=1;    
  int ending = 0; 
  int iterator;
  char roomName[100];                   
  char* roomPath[256];
  struct Room* room = malloc(sizeof(struct Room));
  iterator = setRooms(room, start);

  do{
    printf("\nCURRENT LOCATION: %s\n", room -> name);       
    printf("POSSIBLE CONNECTIONS:");
    printf(" %s,", roomList[room -> connections[0]]);
    for(iterator = 1; iterator < room -> numConnections; iterator++){   
      printf(" %s,", roomList[room -> connections[iterator]]);
    }
    printf(". \nWHERE TO? >");          
    scanf("%s", roomName);              //Get the user input 
      
    // determine where to go with the input.  
    
      //if the user type time, go to show the time using the funtions
    if(strcmp(roomName, "time") == 0){  
      threading();
      showtime();
    }
      
      //check the typing is valid or not
    else if(checkname(room, roomName) == 0){                 
      printf("\nHUH? I DON'T UNDERSTAND THE INPUT. TRY AGAIN.\n");    
    }
      
    //if enter the same room as the locations 
    else{                                   
      roomPath[steps] = room -> name;
      steps++;                              
      iterator = setRooms(room, roomName);      
    }
    // if find the end of the room
    if(room -> type == "END_ROOM"){         
      ending = 1;
    } 
  }while(ending != 1);                      
    //exist the room if find the end room
    
    
    // show the winning and count the steps.
  int i;
  printf(" CONGRATULATIONS! YOU HAVE FOUND THE END ROOM.\n");            
  printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
  for(i = 0; i < steps; i++){          
    printf("%s\n", roomPath[i]);
  }
  free(room);          // free the memory. 
}

/********************************************************************************************************************

main funtion

********************************************************************************************************************/

int main(){
  char start[24];              
  strcpy(start, firstRoom());
  gamePlay(start);    
  return 0;
}
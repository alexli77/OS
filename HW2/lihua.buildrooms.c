#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>



//assign 10 difrerent room names based on color

char *roomList[10] = {
        "blue",
        "red",
        "purple",
        "pink",
        "green",
        "black",
        "orange",
        "silver",
        "yellow",
        "white"
    };
//assign 3 different type of rooms    
    enum room_type
{
	START_ROOM,
	MID_ROOM,
	END_ROOM
};

// set up the room 
struct Room {
  char roomName [9];
  int numConnections;
  struct Room * outboundConnections[6];
  enum room_type rt;

};


//Gobal numbers
struct Room rooms[7];
char dirName[256];

char directoryName[50];

int wholeroom[10];

// the codes outline are provided in 2.2, and I just follow the outline.
// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull() {
  int i;
  for (i = 0; i < 7; i++) {
    if (rooms[i].numConnections < 3 || rooms[i].numConnections > 6) {
      return false;
    }
  }
  return true;
}

// Returns a random Room, does NOT validate if connection can be added
struct Room* GetRandomRoom()
{
  return &rooms[rand() % 7];
}
// first time i try the %4+3 which are selteting from 4 to 7, but here we do not really need it.




// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(struct Room * x)
{
  if(x->numConnections < 6)
    return true;
  return false;
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(struct Room * x, struct Room * y)
{
  int i;
  for(i = 0; i < x->numConnections; i++){
    if(x->outboundConnections[i] == y){
      return true;
    }
  }
  return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room * x, struct Room * y)
{
  x->outboundConnections[x->numConnections] = y;

  x->numConnections += 1;
   
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct Room * x, struct Room * y)
{
  if(x == y) {
    return true;
  }
  return false;
}


// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection()
{
  struct Room* A;  // Maybe a struct, maybe global arrays of ints
  struct Room* B;

  while(true)
  {
    A = GetRandomRoom();

    if (CanAddConnectionFrom(A) == true)
      break;
  }

  do
  {
    B = GetRandomRoom();
  }
  while(!CanAddConnectionFrom(B) || IsSameRoom(A, B) || ConnectionAlreadyExists(A, B));

  ConnectRoom(A, B);  // TODO: Add this connection to the real variables,
  ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
}
//end of outline, and I followed it extalty, I may change some orders to avoid some conflictions

//Now, I finish the codes of add the connections, then I will setup the room and make it connects correctly.


//we need to initciate the connect funtion. 

void initConnect(int index)
{
	int i;
	for(i=0; i<6;i++)      //First we meed to set up 7 rooms with random connetions by the loop
	{	
		rooms[index].outboundConnections[i]=NULL; // make it eaqal to 0. I always set to null instead of 0.
	}
}

// To aviod getting the same room, we can bool the room to see if it already exists.
bool sameroom(int x)
{
	int i;
	for(i=0;i<10;i++)
	{
		if(wholeroom[i]==x) 
		{
			return false; //room is already in game
		}
	}
	return true; //if room is not in the game
}

//next, we can build the rooms.
void setroom()
{

	int i=0;
	while(i<7)  // while loop to build 7 rooms
	{
		int j=rand()%10; // randomly selteting from the 10 rooms.
		if(sameroom(j)==true)//room is not same; to aviod the same room
		{
			memset(rooms[i].roomName,'\0',sizeof(rooms[i].roomName)); // this lines is from pizzza
			strcpy(rooms[i].roomName,roomList[j]); // this lines is from pizzza 
			initConnect(i); 
			rooms[i].rt=MID_ROOM; // setting middle room
			wholeroom[i]=j;
			i++;
		}
	}
	rooms[0].rt = START_ROOM;//Setting start room
	rooms[6].rt = END_ROOM;// setting end room	

	   while (IsGraphFull()==false) // this is from outline, excute to add connections. 
		{
			AddRandomConnection();
		}
		
}

//we finish building the rooms and connections now. Now I need to make it into files for next part.
// If we skip this part, we can also copy all the followwing codes into the next part to get the room.
//However, it is required to write these rooms into files. 

void buildroomfolder()
{

	int result;	
	char  directoryName[256];
	char * nameof="lihua.rooms.";
	int pid= getpid();
	//lines above are the sources from lecture videos
	
    int permissionSetting = 0770; // to setup the permission

        memset(directoryName, '\0', sizeof(directoryName));//copy into the dirName from directoryName.
        sprintf(directoryName, "%s%d", "lihua.rooms.", getpid());    

	result = mkdir(directoryName, permissionSetting);




	chdir(directoryName); //setup the child directory
	FILE * fileptr; //pointer
	
	//strat to write the file
        int i;
	    int j;
	    for(i = 0; i < 7; i++){
        fileptr = fopen(rooms[i].roomName,"w");

        fprintf(fileptr,"ROOM NAME: %s\n", rooms[i].roomName);
        for(j = 0; j < rooms[i].numConnections; j++){
            fprintf(fileptr,"CONNECTION %d: %s\n", j+1, rooms[i].outboundConnections[j]->roomName);
        }
        
        if(rooms[i].rt == START_ROOM){
            fprintf(fileptr,"ROOM TYPE: %s\n", "START_ROOM");
        }
        else if(rooms[i].rt == MID_ROOM){
            fprintf(fileptr,"ROOM TYPE: %s\n", "MID_ROOM");
        }
        else if(rooms[i].rt == END_ROOM){
            fprintf(fileptr,"ROOM TYPE: %s\n", "END_ROOM");
        }
        else{
            fprintf(fileptr,"ROOM TYPE: %s\n", "NULL");
        }
        fclose(fileptr);
    }
}
int main()
{

	srand(time(NULL));

	setroom();
	buildroomfolder();
}
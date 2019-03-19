#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(int argc, char *argv[]){
              
    
    //declear some varibles here.
    int keylength;
    int i=0; 
    srand(time(NULL));  
    
    // Make sure the arguments number has at least 2.
    
    if (argc < 2){              //Need two arguments
        fprintf(stderr, "Error, key is too short.\n");
        exit(1);
    }
    
    keylength = atoi(argv[1]);  //Set the length of the key. Rquired +1 because # of key20 should be 21.
    
  srand(time(0));
	int key;
  //ASCII if there is letter, conver it, if it is not,print space
	for(i; i < keylength; i++)
	{
		key = rand() % 27;
		if(key != 26)
		{
			printf("%c", 'A'+(char)key);
		}
		else
		{
			printf(" ");
		}
	}
	//Print Newline
	printf("\n");
	return 0;                  
}
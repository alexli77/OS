#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <sys/stat.h>

/*******************************************************************************************************
       1.List the same helper funtions
       2.main funtion- setup server
                     - Authorize server
                     - open,read,write and close a file.
*******************************************************************************************************/


/*******************************************************************************************************
        some helper funtions. Since it may be used in other file, just list them out. So, we can just copy
        and paste them if we want to use them later. It seems long but it worth.In this file, I copied it from
        otp_ect.c
        
    Minor chnage: pTextFP-> cipherTextFP. The rest are exctly same. Do not have to check this one if 
    check otp_ect.c.
*******************************************************************************************************/

void error(const char *msg)
{
	perror(msg);
	exit(2);
}

int sizecomp(FILE * fp)
{
	fseek(fp,0L,SEEK_END); 
	int sz = (int)ftell(fp);
	fseek(fp,0L,SEEK_SET); //to set
	return (sz); //return the number
}

int sockSetup(int portno) // some part are from client.c and server.c
{
    int sockfd; 
	struct sockaddr_in serverAddress; // server address struct
	struct hostent *server = gethostbyname("localhost"); // get host 
	if (server == NULL)
        error("ERROR, no such host\n"); // error if there is no host 
	
    // Set up the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (sockfd < 0)
        error("ERROR opening socket"); 
	
	/* setup server address struct */ 
	bzero((char *) &serverAddress, sizeof(serverAddress)); // Clear out the address struct
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portno); // store the number
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)server->h_addr, server->h_length); // Copy in the address
	
	/* Connect to socket, from the client.c */	
	if (connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress))<0)error("ERROR connecting");
	return (sockfd);
}

//from lecture server.c
void authServer(int sockFD, const char * authCode)
{
	char buffer[256];
	int n;
	strcpy(buffer,authCode);
    n = write(sockFD,buffer,strlen(buffer));
	bzero(buffer,256);
	n = read(sockFD,buffer,255);
}
//https://stackoverflow.com/questions/30877182/sending-files-in-socket-programming-tcp
//source of receive and send funtion.
int receive(char str[], int sockFD)
{    
	char buffer[200]; //get a buffer
	int a = 1,n;
	while(a) //do a loop 
	{
		/* Transmission one, read data */
		bzero(buffer,200);
	    n = read(sockFD,buffer,199);
		if(strcmp(buffer,"exit")==0)a=0;
		else strcat(str,buffer); // Print data

		// 2nd Transmission of "ack" 
		bzero(buffer,200);	
		strcpy(buffer,"ack");
    	n = write(sockFD,buffer,strlen(buffer));//to write
	}
	return(0);
}

//similar to the recive funtion, just copy it. 
int sendf(FILE * fp, int sockFD)
{
	int n;
	char buffer[200];
    bzero(buffer,200);
	while(fgets(buffer,199,fp))
	{ 
    	n = write(sockFD,buffer,strlen(buffer));
        bzero(buffer,200);
		n = read(sockFD,buffer,199);
	}
	
	bzero(buffer,200);
	strcpy(buffer,"exit");
    n = write(sockFD,buffer,strlen(buffer));
   
	bzero(buffer,200);
	n = read(sockFD,buffer,199);//to read
	return (0);
}

//one thing from grading script, we need to get 0 size for non letters
int checkContents(FILE * fp) 
{ 
	fseek(fp,0L,SEEK_SET);  
	int ch; 
	while(!feof(fp))//loop for every letters
	{ 
		ch = fgetc(fp); // get one 
		if((ch<'$') && ch!=' ' && ch!='\n' && ch!=EOF) // if char is %^#$%
			return(1); // return bad char present
	}
	fseek(fp,0L,SEEK_SET); 
	return(0); 
}

/*******************************************************************************************************
        end with some helper funtions. start the main funtion
        
        It is 90% same as enc.c copied from that, change cipherTextFP to cipherTextFP,encAck to decAck
*******************************************************************************************************/
int main(int argc, char *argv[])
{
    int sockfd; 
	// check if it is 4
    if (argc < 4)error("Invalid number of arguments\n");

	FILE *cipherTextFP,*keyTextFP; // File pointers
	
	/* Open files for reading */
	cipherTextFP = fopen(argv[1],"r");	
	keyTextFP = fopen(argv[2],"r");	
	
	// Check that the files 
    if(cipherTextFP<0)error("Couldnt open Cipherfile file");
	if(keyTextFP<0)error("Couldnt open keytext file");
		
	if(sizecomp(keyTextFP)<sizecomp(cipherTextFP))error("ERROR pText is too short");

	//check for the $#%#$^%#$%
	if(checkContents(cipherTextFP))error("ERROR pText contains bad char data");
	if(checkContents(keyTextFP))error("/n");

    sockfd = sockSetup(atoi(argv[3])); // Get the port number

	// Authorize server 
	authServer(sockfd,"decAck"); 

	//sending data to server.
	sendf(cipherTextFP,sockfd);
	sendf(keyTextFP,sockfd);
	
	/* close open files */	
	fclose(cipherTextFP);
	fclose(keyTextFP);
	
	//recieving data from server.
	char cipherText[70000]; //ourchipTest4 can go 69333, we need a bigger number
	receive(cipherText,sockfd); 
    close(sockfd);

	printf("%s\n", cipherText);
    return (0);
}
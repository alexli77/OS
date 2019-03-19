#include <stdio.h>  
#include <stdlib.h> 
#include <string.h>
#include <dirent.h> 
#include <unistd.h> 
#include <sys/stat.h> 
#include <assert.h> 
#include <sys/wait.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/socket.h> 
#include <netdb.h>

/*******************************************************************************************************
       1.List the same helper funtions
       2.set up the encrypted funtions
       3.main funtion- conect to the otp_enc.c
                     - read and input the letters
                     - run encrypted
                     - create the chipper
                     - write the results to a file
                     
*******************************************************************************************************/


/*******************************************************************************************************
        some helper funtions. Since it may be used in other file, just list them out. So, we can just copy
        and paste them if we want to use them later. It seems long but it worth.In this file, I copied it from
        otp_ect.c
*******************************************************************************************************/
void error(const char *msg)
{
	perror(msg);
	exit(2);
}

int sockSetup(int portno) // some part are from client.c and server.c
{
	int sockfd;
    struct sockaddr_in serv_addr; //get address
    sockfd = socket(AF_INET, SOCK_STREAM, 0);//AF_INET refers to addresses from the internet, SOCK_STREAM Provides sequenced
    if (sockfd < 0)error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));//could use memset, but it may bring the crash
    serv_addr.sin_family = AF_INET; //set the adress
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);//set theport
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))<0)error("ERROR on binding");
	listen(sockfd,5); // Create a client connection queue
	return (sockfd);
}

void authServer(int sockFD, const char * authCode)//from the Server.c of the lecture
{ 
	char buffer[256],badACK[256];
	int n;
	bzero(buffer,256);
	n = read(sockFD,buffer,255);
 	strcpy(badACK,buffer);

	bzero(buffer,256);	
	strcpy(buffer,authCode);
   	n = write(sockFD,buffer,strlen(buffer));
}

//https://stackoverflow.com/questions/30877182/sending-files-in-socket-programming-tcp
//source of receive and send funtion.
int receive(char str[], int sockFD)//from the Cleint.c of the lecture
{    
	char buffer[200]; //get a buffer
	int a = 1,n;
	while(a) //do a loop 
	{
		//Transmission one, read data 
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


int sendString(char str[], int sockFD)
{
	FILE * fp;
	fp = fmemopen(str,strlen(str)+1,"r");
	int n;
	char buffer[256];
	// Transmission of files
    bzero(buffer,256);
	while(fgets(buffer,255,fp))
	{ 
    	n = write(sockFD,buffer,strlen(buffer));
    	if(n < 0)error("ERROR writing to socket");
		bzero(buffer,256);
		n = read(sockFD,buffer,255);
		if (n < 0)error("ERROR reading from socket");
		bzero(buffer,256);
	}
	
	/* Transmit exit */
	strcpy(buffer,"exit");
    n = write(sockFD,buffer,strlen(buffer));
    if(n < 0)error("ERROR writing to socket");
   
	/* Receive ack */
	bzero(buffer,256);
	n = read(sockFD,buffer,255);
	if (n < 0)error("ERROR reading from socket");
	return (0);
	fclose(fp);
}
//have problem with sending string, this one is from piazza and chegg.com. I create the top part, did not 
//realized have to do that one outside the loop.

/*******************************************************************************************************
        encrypted the code funtion. Will change to decrypted in dec_d file.
        We need to chnage the letter to number and do some funtion and then chnage back to letters.
        The idea of this is from the piazza.
*******************************************************************************************************/

//chnage the number back to the letter
int numAscii(int num)
{
	if(num == 26)
		return (32);
	return (num + 65);
}

//change the letter to number
int asciiNum(char aChar)
{
	if(aChar == 32)
		return (26);
	return (aChar - 65);
}


//the funtion chnage the numbers. 
char codeChar(char letter,char key)
{
	char cipher;
    
	cipher = numAscii((asciiNum(letter)+asciiNum(key))%27);
    // it will be using modulo 27 operations: your 27 characters are the 26 capital letters
    
	return (cipher);
}
//we need to keep this funtion as a comment in  dec_d file, so we can know how to chnage it back. 




/*******************************************************************************************************
        end with helper funtions. start the main funtion
        Just write the main as the outline. 
        
*******************************************************************************************************/

int main(int argc, char *argv[])
{
    char pText[300000];
	char keyText[300000];
	char cipherText[300000];
    char cText[0];
	/* Check port was provided */
    if(argc < 2)error("ERROR, no port provided\n");

    int clientSockFD; // client socket number
	int sockfd = sockSetup(atoi(argv[1])); // setup socket

    socklen_t clilen; // Client address length 
	struct sockaddr_in cli_addr; // set address
	signal(SIGCHLD, SIG_IGN);
    // above are from Client.c from the lecture.
    
    
	while(1)
	{
   		clilen = sizeof(cli_addr);
		clientSockFD = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
    	if (clientSockFD<0)error("ERROR on accept");
        //set the child to the parent 
		if(fork()==0)
		{ 		
			authServer(clientSockFD,"encAck"); // send one time auth code
            receive(pText,clientSockFD);
	        receive(keyText,clientSockFD);
            //use a do while to get the change.
            int i = -1;
	     do{
	         i+=1;
	         if(pText[i]!='\0' && pText[i]!='\n')
                cText[i]=codeChar(pText[i],keyText[i]);
	          else cText[i] = '\0';
        	}while(pText[i]!='\0');
            //finished with do while
            sendString(cipherText,clientSockFD); 
            exit(0);
			close(clientSockFD);
		}
		else
		{ 
			close(clientSockFD);
		}
	}
    
    close(sockfd);
    return (0); 
}


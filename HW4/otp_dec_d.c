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
	struct sockaddr_in serverAddress; // server address struct
	struct hostent *server = gethostbyname("localhost"); // get host 
	if (server == NULL)
		error("ERROR, no such host\n"); // error if there is no host 

										// Set up the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (sockfd < 0)
		error("ERROR opening socket");

	/* setup server address struct */
	bzero((char *)&serverAddress, sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portno); // store the number
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)server->h_addr, server->h_length); // Copy in the address

																							/* Connect to socket, from the client.c */
	if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress))<0)error("ERROR connecting");
	return (sockfd);
}

void authServer(int sockFD, const char * authCode)
{
	char buffer[256], badACK[256];
	int n;
	bzero(buffer, 256);
	n = read(sockFD, buffer, 255);
	strcpy(badACK, buffer);

	bzero(buffer, 256);
	strcpy(buffer, authCode);
	n = write(sockFD, buffer, strlen(buffer));
}


int receive(char str[], int sockFD)
{
	char buffer[200]; //get a buffer
	int a = 1, n;
	while (a) //do a loop 
	{
		/* Transmission one, read data */
		bzero(buffer, 200);
		n = read(sockFD, buffer, 199);
		if (strcmp(buffer, "exit") == 0)a = 0;
		else strcat(str, buffer); // Print data

								  // 2nd Transmission of "ack" 
		bzero(buffer, 200);
		strcpy(buffer, "ack");
		n = write(sockFD, buffer, strlen(buffer));//to write
	}
	return(0);
}
/*******************************************************************************************************
More helper funtion, will use for otp_dec_d.c
*******************************************************************************************************/



int sendString(char str[], int sockFD)
{
	FILE * fp;
	fp = fmemopen(str, strlen(str) + 1, "r");
	int n;
	char buffer[256];
	/* Transmission of files */
	bzero(buffer, 256);
	while (fgets(buffer, 255, fp))
	{ // send pTextFile
	  /* Transmit data */
		n = write(sockFD, buffer, strlen(buffer));
		if (n < 0)error("ERROR writing to socket");

		/* Receive acks */
		bzero(buffer, 256);
		n = read(sockFD, buffer, 255);
		if (n < 0)error("ERROR reading from socket");

		bzero(buffer, 256);
	}

	/* Transmit exit */
	strcpy(buffer, "exit");
	n = write(sockFD, buffer, strlen(buffer));
	if (n < 0)error("ERROR writing to socket");

	/* Receive ack */
	bzero(buffer, 256);
	n = read(sockFD, buffer, 255);
	if (n < 0)error("ERROR reading from socket");
	return (0);
	fclose(fp);
}

//conver this into 
int numAlph(int num)
{
	if (num == 26)
		return (32);
	return (num + 65);
}

int alphNum(char aChar)
{
	if (aChar == 32)
		return (26);
	return (aChar - 65);
}



char codeChar(char letter, char key)
{
	int num;
	char aChar;
	if (num == 26)
		return (32);
	else
		return (num + 65);
	if (aChar == 32)

		return (26);
	return (aChar - 65);
	char cipher;
	cipher = numAlph((alphNum(letter) + alphNum(key)) % 27);
	return (cipher);
}

char decodeChar(char cipher, char key)
{
	char decodedLetter;
	if ((alphNum(cipher) - alphNum(key)) < 0)
		decodedLetter = numAlph(27 + (alphNum(cipher) - alphNum(key)));
	else
		decodedLetter = numAlph((alphNum(cipher) - alphNum(key)) % 27);
	return (decodedLetter);
}

void codeString(char pText[], char keyText[], char cText[])
{
	int i = -1;
	do {
		i += 1;
		if (pText[i] != '\0' && pText[i] != '\n')cText[i] = codeChar(pText[i], keyText[i]);
		else cText[i] = '\0';
	} while (pText[i] != '\0');
}

void decodeString(char pText[], char keyText[], char cText[])
{
	int i = -1;
	do {
		i += 1;
		if (cText[i] != '\0' && cText[i] != '\n')pText[i] = decodeChar(cText[i], keyText[i]);
		else pText[i] = '\0';
	} while (cText[i] != '\0');
}

int servSockSetup(portno)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)error("ERROR opening socket");
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)error("ERROR on binding");
	listen(sockfd, 5); // Create a client connection queue
	return (sockfd);
}
int serviceClient(int sockFD)
{
	/* Buffers */
	char pText[290000];
	char keyText[290000];
	char cipherText[290000];
	/* These are rediculously large,
	* but dynamic allocation
	* isn't a requirement of
	* this assignment */

	/* Recieve cipherText and keyText files */
	receive(cipherText, sockFD);
	receive(keyText, sockFD);

	/* Decode cipher */
	decodeString(pText, keyText, cipherText);

	/* Transmit the decoded text */
	sendString(pText, sockFD);

	return (0);
}/*******************************************************************************************************
        end with some helper funtions. start the main funtion
        
*******************************************************************************************************/

int main(int argc, char *argv[])
{
    int sockfd, clientSockFD, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) 
             error("ERROR on binding");
    listen(sockfd,5);
	
	signal(SIGCHLD, SIG_IGN);
	while(1)
	{
		clilen=sizeof(cli_addr);
		clientSockFD = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
		if(clientSockFD<0)error("ERROR on accpet");
		if(fork()==0)
		{ // I am the child
			authServer(clientSockFD,"decAck"); // send one time auth code
			serviceClient(clientSockFD);
			exit(0);
			close(clientSockFD);
		}
		else
		{ // I am the parent, and my work is done
			close(clientSockFD);
		}
	}
	
    close(sockfd);
    return (0); 
}





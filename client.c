/*
* Simple client to work with server.c program.
* Host name and port used by server are to be
* passed as arguments.
*
* To test: Open a terminal window.
* At prompt ($ is my prompt symbol) you may
* type the following as a test:
*
* $./client 127.0.0.1 54554
* Please enter the message: Programming with sockets is fun!
* I got your message
* $
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_LEN 256
#define SLEEP_TIME 1 //1s
#define END_WORD "!END!"

sem_t mutex;
void encode(char*){}
void decode(char*){}
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int sockfd;

FILE *inputfile  = 0; // read only 
FILE *resultfile  = 0; 

void * clientDecoder(void * arg)
{
	printf("Begin Sending Words to Server Now... \n");
	char word_buf[MAX_LEN];

	if (inputfile == NULL) 
    	error("ERROR Reading Input File");

	while(!feof(inputfile)){
	bzero(word_buf,MAX_LEN);
	fscanf(inputfile,"%s",word_buf);
		//write to socket!
		sem_wait(&mutex);
	write(sockfd,word_buf,sizeof(word_buf));
		sem_post(&mutex);
		sleep(SLEEP_TIME);
		if(strcmp(word_buf,END_WORD) == 0) {
			fclose(resultfile);
			exit;
		}
		printf("Sending : %s\n",word_buf);
	}
	fclose(resultfile);
	printf("End Sending Words to Server Now... \n");
}


void *clientEncoder(void *arg)
{
	if (resultfile == NULL) 
    		error("ERROR Opening Result File");
	char word_buf[MAX_LEN];

	while(1){
		sleep(SLEEP_TIME);
		bzero(word_buf,MAX_LEN);
			//write to socket!
			sem_wait(&mutex);
		int readlen = read(sockfd,word_buf, MAX_LEN);
			sem_post(&mutex);
			if(readlen > 0) {
				fprintf(resultfile,"%s, " ,word_buf);
				//printf("From server : %s\n", word_buf);
		}
    }
}

void connectToServer(char* host,int portno)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	server = gethostbyname(host);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");
}

int main(int argc, char *argv[])
{

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	inputfile  = fopen("input.txt", "r"); // read only 
	resultfile  = fopen("result.txt", "a"); // read only
	sem_init(&mutex, 0, 1);

	connectToServer(argv[1],atoi(argv[2]));

	pthread_t threadEncoderId, threadDecoderId;
	int err = pthread_create(&threadDecoderId, NULL, &clientDecoder, NULL);
	err = pthread_create(&threadEncoderId, NULL, &clientEncoder, NULL);

	err = pthread_join(threadDecoderId, NULL);
	err = pthread_join(threadEncoderId, NULL);
	
	fclose(inputfile);
	fclose(resultfile);
	sem_destroy(&mutex);
	close(sockfd);
	
	//close(sockfd);
	return 0;
}



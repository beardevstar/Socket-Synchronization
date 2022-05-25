#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <semaphore.h>
#include<pthread.h> //for threading , link with lpthread

#define MAX_LEN 256
#define PORT_NO 8888
#define QUE_LEN 10
#define END_WORD "!END!"
#define SLEEP_TIME 0.4

typedef struct _StrQueue{
	char list[QUE_LEN][MAX_LEN];
	int header, tail, cnt;
} StrQueue;

void *ServerEncoder(void *);

void encode(char*){}
void decode(char*){}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void initQueue(StrQueue* queue){
	bzero((char*)queue,sizeof(queue));
}

void pushStr(char* str, StrQueue* queue){
	if(queue->cnt == QUE_LEN || str == NULL) return;

	bzero(queue->list[queue->tail],MAX_LEN);
	strncpy(queue->list[queue->tail],str,strlen(str));
	if(queue->cnt == 0) queue->header = queue->tail;

	queue->tail ++;
	queue->tail %= QUE_LEN;
	queue->cnt++;
}

char* popStr(StrQueue* queue){
	if(queue->cnt <= 0) return NULL;

    	char * ret = queue->list[queue->header];
    	
	queue->header ++;
	queue->header %= QUE_LEN;
	queue->cnt--;

	return ret;
}

StrQueue st_origin;sem_t mutex_origin;
StrQueue st_ae;sem_t mutex_ae;
StrQueue st_ei;sem_t mutex_ei;
StrQueue st_io;sem_t mutex_io;
StrQueue st_ou;sem_t mutex_ou;
StrQueue st_ud;sem_t mutex_ud;
StrQueue st_dw;sem_t mutex_dw;
StrQueue st_result;sem_t mutex_result;

void change(char _from, char _to, char* src){
	int len = strlen(src);
	for(int i=0;i<len;i++)
		src[i] = src[i] == _from ? _to : src[i];
}

int calcDigitSum(char* src){
	int len = strlen(src);
	int sum = 0;
	for(int i=0;i<len;i++){
		char cc = src[i] - 30;
		if(cc > 0 && cc < 10) sum += cc;
	}
	return sum;
}

void* Thread_A(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_origin);
		char* str = popStr(&st_origin);
		sem_post(&mutex_origin);

		if(str == NULL) continue;
		change('a','A',str);
		//locking...
		sem_wait(&mutex_ae);
		pushStr(str, &st_ae);
		sem_post(&mutex_ae);
		if(strcmp(str,END_WORD) == 0 ) break;
	}
	return NULL;
}

void* Thread_E(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_ae);
		char* str = popStr(&st_ae);
		sem_post(&mutex_ae);

		if(str == NULL) continue;
		change('b','E',str);
		//locking...
		sem_wait(&mutex_ei);
		pushStr(str, &st_ei);
		sem_post(&mutex_ei);

		if(strcmp(str,END_WORD) == 0 ) break;
	}
	return NULL;
}
void* Thread_I(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_ei);
		char* str = popStr(&st_ei);
		sem_post(&mutex_ei);

		if(str == NULL) continue;
		change('i','I',str);
		//locking...
		sem_wait(&mutex_io);
		pushStr(str, &st_io);
		sem_post(&mutex_io);

		if(strcmp(str, END_WORD) == 0 ) break;
	}
	return NULL;
}
void* Thread_O(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_io);
		char* str = popStr(&st_io);
		sem_post(&mutex_io);

		if(str == NULL) continue;
		change('o','O',str);
		//locking...
		sem_wait(&mutex_ou);
		pushStr(str, &st_ou);
		sem_post(&mutex_ou);

		if(strcmp(str, END_WORD) == 0 ) break;
	}
	return NULL;
}
void* Thread_U(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_ou);
		char* str = popStr(&st_ou);
		sem_post(&mutex_ou);

		if(str == NULL) continue;
		change('u','U',str);
		//locking...
		sem_wait(&mutex_ud);
		pushStr(str, &st_ud);
		sem_post(&mutex_ud);

		if(strcmp(str, END_WORD) == 0 ) break;
	}
	return NULL;
}

void* Thread_D(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_ud);
		char* str = popStr(&st_ud);
		sem_post(&mutex_ud);
		
		if(str == NULL) continue;

		//calculating sum
		int sum = calcDigitSum(str);
		if(sum > 0) printf("Sum : %d", sum);

		//locking...
		sem_wait(&mutex_dw);
		pushStr(str, &st_dw);
		sem_post(&mutex_dw);

		if(strcmp(str, END_WORD) == 0 ) break;
	}
	return NULL;
}

void* Thread_W(void* arg){
	while(1){
		sleep(SLEEP_TIME);
		//locking...
		sem_wait(&mutex_dw);
		char* str = popStr(&st_dw);
		sem_post(&mutex_dw);
		
		if(str == NULL) continue;

		//locking...
		sem_wait(&mutex_result);
		pushStr(str, &st_result);
		sem_post(&mutex_result);

		if(strcmp(str, END_WORD) == 0 ) break;
	}
	return NULL;
}

void* ServerDecoder(void*);
void* ServerEncoder(void*);

int main(int argc , char *argv[])
{
	int socket_desc , new_socket , c , *new_sock;
	struct sockaddr_in server , client;
	char *message;
	sem_init(&mutex_origin, 0, 1);
	sem_init(&mutex_ae, 0, 1);
	sem_init(&mutex_ei, 0, 1);
	sem_init(&mutex_io, 0, 1);
	sem_init(&mutex_ou, 0, 1);
	sem_init(&mutex_ud, 0, 1);
	sem_init(&mutex_dw, 0, 1);
	sem_init(&mutex_result, 0, 1);

	initQueue(&st_origin);
	initQueue(&st_ae);
	initQueue(&st_ei);
	initQueue(&st_io);
	initQueue(&st_ou);
	initQueue(&st_ud);
	initQueue(&st_dw);
	initQueue(&st_result);

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT_NO );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("bind failed");
		return 1;
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
				
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;
		
		pthread_create( &sniffer_thread , NULL ,  ServerDecoder , (void*) new_sock);
		pthread_create( &sniffer_thread , NULL ,  ServerEncoder , (void*) new_sock);
		
		pthread_create( &sniffer_thread , NULL ,  Thread_A , NULL) ;
		pthread_create( &sniffer_thread , NULL ,  Thread_E , NULL) ;
		pthread_create( &sniffer_thread , NULL ,  Thread_I , NULL) ;
		pthread_create( &sniffer_thread , NULL ,  Thread_O , NULL) ;
		pthread_create( &sniffer_thread , NULL ,  Thread_U , NULL) ;
		pthread_create( &sniffer_thread , NULL ,  Thread_D , NULL) ;
		pthread_create( &sniffer_thread , NULL ,  Thread_W , NULL) ;
		//Now join the thread , so that we dont terminate before the thread
		//puts("Handler assigned\n");
//		pthread_join( sniffer_thread , NULL);

	}
	
	if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}
	
	sem_destroy(&mutex_origin);
	sem_destroy(&mutex_ae);
	sem_destroy(&mutex_ei);
	sem_destroy(&mutex_io);
	sem_destroy(&mutex_ou);
	sem_destroy(&mutex_ud);
	sem_destroy(&mutex_dw);
	sem_destroy(&mutex_result);

	return 0;
}
/*
 * This will handle connection for each client
 * */
void *ServerDecoder(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[MAX_LEN];	

	//Receive a message from client	
	while( (read_size = read(sock , client_message , MAX_LEN)) > 0 )
	{
		//Send the message back to client
		sem_wait(&mutex_origin);
		if(strcmp(client_message, END_WORD) != 0 ) 
			decode(client_message);
		pushStr(client_message, &st_origin);
		sem_post(&mutex_origin);

		if(strcmp(client_message, END_WORD) == 0) error("Terminating By Client...");
	}
	
	if(read_size == 0)
	{
		puts("Client disconnected\n");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
		
	//Free the socket pointer
	free(socket_desc);
	
	return 0;
}

void* ServerEncoder(void *socket_desc){
	int sock = *(int*)socket_desc;

	while(1){
		//locking...
		sem_wait(&mutex_dw);
		char* str = popStr(&st_result);
		sem_post(&mutex_dw);
		
		if(str == NULL) continue;
		if(strcmp(str, END_WORD) != 0 ) 
			encode(str);
		//sending to socket...
		write(sock, str,strlen(str));
		
		printf("%s\n",str);
		if(strcmp(str, END_WORD) == 0 ) break;
	}
	return NULL;
}

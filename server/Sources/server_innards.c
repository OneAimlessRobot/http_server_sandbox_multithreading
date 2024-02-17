#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/resource_consts.h"
#include "../Includes/http_req_parser.h"
#include "../Includes/handlecustom.h"
#include "../Includes/client.h"
#include "../Includes/server_vars.h"
#include "../Includes/send_resource_func.h"
#include "../Includes/load_logins.h"
#include "../Includes/server_innards.h"
#include "../Includes/sock_ops.h"
#include "../Includes/session_ops.h"
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#define SEND_SOCK_BUFF_SIZE 10000000
static socklen_t socklenpointer;
static int server_socket,numOfClients,currNumOfClients;

static struct sockaddr_in server_address, clientAddress;

static fd_set readfds;

static char addressContainer[INET_ADDRSTRLEN];

client* getClientArrCopy(void){
	
	client* result= malloc(currNumOfClients*sizeof(client));
	for(int i=0;i<currNumOfClients;i++){
		
		memset(result[i].username,0,FIELDSIZE);
		result[i].socket=clients[i].socket;
		result[i].isAdmin=clients[i].isAdmin;
		result[i].logged_in=clients[i].logged_in;
		result[i].running_time=clients[i].running_time;
		strncpy(result[i].username,clients[i].username,FIELDSIZE);
		memcpy(&result[i].client_addr,&clients[i].client_addr,sizeof(clients[i].client_addr));
	}
	return result;

}

client* getFullClientArrCopy(void){
	
	client* result= malloc(numOfClients*sizeof(client));
	for(int i=0;i<numOfClients;i++){
		
		memset(result[i].username,0,FIELDSIZE);
		result[i].socket=clients[i].socket;
		result[i].isAdmin=clients[i].isAdmin;
		result[i].logged_in=clients[i].logged_in;
		result[i].running_time=clients[i].running_time;
		strncpy(result[i].username,clients[i].username,FIELDSIZE);
		memcpy(&result[i].client_addr,&clients[i].client_addr,sizeof(clients[i].client_addr));
	}
	return result;

}

int getCurrNumOfClients(void){
	
	return currNumOfClients;


}
int getMaxNumOfClients(void){
	
	return numOfClients;


}
static void sigint_handler(int signal){
	signal=0;
	
pthread_mutex_lock(&serverRunningMtx);
	serverOn=0;

pthread_mutex_unlock(&serverRunningMtx);
	close(server_socket);
	if(clients){
	for(int i=0;i<numOfClients;i++){
		pthread_join(clients[i].threadid,NULL);
		pthread_mutex_lock(&socketMtx);
		if(clients[i].socket){

		close(clients[i].socket);

		}
		pthread_mutex_unlock(&socketMtx);
	}
	free(clients);
	}
	if(logging){
	fprintf(logstream,"Adeus! Server out...\n");
	}
	fclose(logstream);
	if(currLogins){
	
	freeLogins(&currLogins);


	}
	if(currAdmins){
	
	freeLogins(&currAdmins);


	}
	printf("Adeus! Server out...\n");
	exit(-1+signal);

}
static void handleIncommingConnections(void){
	while(serverOn){
	
	FD_ZERO(&readfds);
        FD_SET(server_socket,&readfds);
	int client_socket;
	int activity = select( server_socket + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }
	if(FD_ISSET(server_socket,&readfds)){

		
		if ((client_socket = accept(server_socket,(struct sockaddr*)(&clientAddress),&socklenpointer))<0)
		{
		if(logging){
                fprintf(logstream,"accept error!!!!: %s\n",strerror(errno));
        	}
		raise(SIGINT);
		
        	}
		currNumOfClients++;
		setNonBlocking(client_socket);
		//setLinger(client_socket,0,0);
		getpeername(client_socket , (struct sockaddr*)&clientAddress , (socklen_t*)&socklenpointer);
                if(logging){
		fprintf(logstream,"Client connected , ip %s , port %d \n" ,inet_ntoa(clientAddress.sin_addr) , ntohs(clientAddress.sin_port));
        	}
		//setSocketSendBuffSize(client_socket,SEND_SOCK_BUFF_SIZE);
	    	for(int i=0;i<numOfClients;i++){
		client* c=&(clients[i]);
		pthread_mutex_lock(&socketMtx);
		if(!c->socket){
		pthread_mutex_unlock(&socketMtx);
		c->socket=client_socket;
		c->running_time=0.0;
		memset(c->username,0,FIELDSIZE);
		memcpy(&c->client_addr,&clientAddress,sizeof(struct sockaddr_in));
		c->isAdmin=0;
		c->logged_in=0;
		memset(c->peerbuff,0,PAGE_DATA_SIZE);
		pthread_create(&c->threadid,NULL,runClientConnection,(void*)c);
		    if(logging){
		    fprintf(logstream,"Adding to list of sockets as socket no: %d\n" , c->socket);
                    }
		    break;
                }
		pthread_mutex_unlock(&socketMtx);
		}
		}
            			
	}

}

static void mainLoop(void){
	
	
	while(serverOn){
	
	
	handleIncommingConnections();
	
    	}

}

void initializeServer(int max_quota,int logs){
	
/*	if(!(logstream=fopen(logfpath,"w"))){
	
		perror("logs will be made to stdout!!!! could not create log file\n");
		logstream=stdout;
	}
*/	
	logstream=stdout;
	serverOn=1;
	logging=logs;
	beeping=0;
	currNumOfClients=0;
	numOfClients=max_quota;
	signal(SIGINT,sigint_handler);
	clients=malloc(sizeof(client)*max_quota);
	memset(clients,0,sizeof(client)*max_quota);
	for(int i=0;i<max_quota;i++){

		clients[i].socket=0;
		clients[i].running_time=0.0;
		memset(clients[i].username,0,FIELDSIZE);
		memset(&clients[i].client_addr,0,sizeof(struct sockaddr_in));
		clients[i].isAdmin=0;
		clients[i].logged_in=0;
		memset(clients[i].peerbuff,0,PAGE_DATA_SIZE);
		

	}
	server_socket= socket(AF_INET,SOCK_STREAM,0);
        int ptr=1;
        setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
        if(server_socket==-1){
                raise(SIGINT);

        }

	
	memset(currDir,0,PATHSIZE);
	
	getcwd(currDir,PATHSIZE);

        server_address.sin_family=AF_INET;
        server_address.sin_port= htons(PORT);
        server_address.sin_addr.s_addr = INADDR_ANY;
        socklen_t socklength=sizeof(server_address);
        
	bind(server_socket,(struct sockaddr*)(&server_address),socklength);

        listen(server_socket,MAX_CLIENTS_HARD_LIMIT);
	
        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&server_address;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        
	inet_ntop( AF_INET, &ipAddr, addressContainer, INET_ADDRSTRLEN );
	if(logging){
	fprintf(logstream,"Server spawnado @ %s\n",inet_ntoa(server_address.sin_addr));
	}
	socklenpointer=sizeof(clientAddress);
	loadLogins();
	loadAdmins();
	mainLoop();
	freeLogins(&currLogins);
}


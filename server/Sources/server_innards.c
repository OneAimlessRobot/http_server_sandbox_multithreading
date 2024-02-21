#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/resource_consts.h"
#include "../Includes/http_req_parser.h"
#include "../Includes/handlecustom.h"
#include "../Includes/client.h"
#include "../Includes/cfg_loading.h"
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
static int server_socket,currNumOfClients;

static struct sockaddr_in server_address, clientAddress;

static fd_set readfds;
static pthread_t con_thread;
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
	
	client* result= malloc(quota*sizeof(client));
	for(int i=0;i<quota;i++){
		
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
	
	return quota;


}
static void sigint_afterint(int signal){

	printf("%d\n",signal);
}
void cleanup(void){

	signal(SIGINT,sigint_afterint);
	close(server_socket);
	if(clients){
	for(int i=0;i<quota;i++){
		int c_socket=0;
		pthread_mutex_lock(&socketMtx);
		if(clients[i].socket){
		c_socket=clients[i].socket;
		clients[i].socket=0;
		close(c_socket);
		pthread_mutex_unlock(&socketMtx);
		pthread_join(clients[i].threadid,NULL);
		}
		else{
		pthread_mutex_unlock(&socketMtx);

		}
	}
	free(clients);
	}
	if(logging){
	fprintf(logstream,"Adeus! Server out...\n");
	}
	if(currLogins){
	
	freeLogins(&currLogins);


	}
	if(cfgs){
	
	freeCFGs(&cfgs);


	}
	printf("Adeus! Server out...\n");
	fclose(logstream);
	exit(-1);

}
static void sigint_handler(int arg){
	
pthread_mutex_lock(&serverRunningMtx);
	serverOn=0;

pthread_mutex_unlock(&serverRunningMtx);
pthread_join(con_thread,NULL);
	cleanup();
}
static void handleIncommingConnections(void){
	
	FD_ZERO(&readfds);
        FD_SET(server_socket,&readfds);
	int client_socket;
		struct timeval tv;
                tv.tv_sec=bigtimeoutsecs;
                tv.tv_usec=bigtimeoutusecs;
 
	int activity = select( server_socket + 1 , &readfds , NULL , NULL , &tv);
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
		getpeername(client_socket , (struct sockaddr*)&clientAddress , (socklen_t*)&socklenpointer);
                if(logging){
		fprintf(logstream,"Client connected , ip %s , port %d \n" ,inet_ntoa(clientAddress.sin_addr) , ntohs(clientAddress.sin_port));
        	}
		for(int i=0;i<quota;i++){
		client* c=&(clients[i]);
		pthread_mutex_lock(&socketMtx);
		if(!c->socket){
		pthread_mutex_unlock(&socketMtx);
		c->socket=client_socket;
		memcpy(&c->client_addr,&clientAddress,sizeof(struct sockaddr_in));
		pthread_create(&c->threadid,NULL,runClientConnection,(void*)c);
	 	//pthread_detach(c->threadid);
		if(logging){
		    fprintf(logstream,"Adding to list of sockets as socket no: %d\n" , c->socket);
                    }
		    break;
                }
		pthread_mutex_unlock(&socketMtx);
		}
		}
           if(!activity){
		if(logging){
			fprintf(logstream,"No new connections\n");
		}
		
		}
	

}

static void* mainLoop(void* args){
	
	pthread_mutex_lock(&serverRunningMtx);
	while(serverOn){
	pthread_mutex_unlock(&serverRunningMtx);
	
	
	handleIncommingConnections();
	pthread_mutex_lock(&serverRunningMtx);
    	}
	
	pthread_mutex_unlock(&serverRunningMtx);
	if(logging){
		fprintf(logstream,"Connection thread out!\n");
	}
	return NULL;
}

void initializeConstants(void){

	strncpy(incorrectLoginTarget,find_value_in_cfg_arr("incorrectLoginTarget",cfgs),FIELDSIZE-1);
	strncpy(defaultLoginTarget,find_value_in_cfg_arr("defaultLoginTarget",cfgs),FIELDSIZE-1);
	strncpy(doubleSessionTarget,find_value_in_cfg_arr("defaultSessionTarget",cfgs),FIELDSIZE-1);
	strncpy(defaultTarget,find_value_in_cfg_arr("defaultTarget",cfgs),FIELDSIZE-1);
	logging=atoi(find_value_in_cfg_arr("logging",cfgs));
	smalltimeoutsecs=atoi(find_value_in_cfg_arr("smalltimeoutsecs",cfgs));
	
	smalltimeoutusecs=atoi(find_value_in_cfg_arr("smalltimeoutusecs",cfgs));
	
	bigtimeoutsecs=atoi(find_value_in_cfg_arr("bigtimeoutsecs",cfgs));
	
	bigtimeoutusecs=atoi(find_value_in_cfg_arr("bigtimeoutusecs",cfgs));
	
	port=atoi(find_value_in_cfg_arr("port",cfgs));
	
	quota=atoi(find_value_in_cfg_arr("quota",cfgs));
	
	use_fd=atoi(find_value_in_cfg_arr("use_fd",cfgs));
	
	compression=atoi(find_value_in_cfg_arr("compression",cfgs));
	
	compression_level=atoi(find_value_in_cfg_arr("compression_level",cfgs));
	if(quota<=0){
		
		quota=500;

	}
	if(smalltimeoutsecs<0){
		
		printf("Config error: timeout must be a non negative 32 bit integer!\n");
		exit(-1);
	}
	if(smalltimeoutusecs<0){
		
		printf("Config error: timeout must be a non negative 32 bit integer!\n");
		exit(-1);
	}
	if(bigtimeoutsecs<0){
		
		printf("Config error: timeout must be a non negative 32 bit integer!\n");
		exit(-1);
	}
	if(bigtimeoutusecs<0){
		
		printf("Config error: timeout must be a non negative 32 bit integer!\n");
		exit(-1);
	}
	if(compression_level<0 || compression_level > 9){
		
		printf("Config error: compression level must be an integer between 0 and 9!\n");
		exit(-1);
	}

}
static void initEmptyClient(client* result){
		result->socket=0;
		result->running_time=0.0;
		memset(result->username,0,FIELDSIZE);
		memset(&result->client_addr,0,sizeof(struct sockaddr_in));
		result->isAdmin=0;
		result->logged_in=0;
		memset(result->peerbuff,0,PAGE_DATA_SIZE);
		

}
void initializeServer(void){

	
	memset(currDir,0,PATHSIZE);
	
	getcwd(currDir,PATHSIZE);

	loadCfg();
	initializeConstants();
	
	logstream=stdout;
	serverOn=1;
	currNumOfClients=0;
	signal(SIGINT,sigint_handler);
	clients=malloc(sizeof(client)*quota);
	memset(clients,0,sizeof(client)*quota);
	for(int i=0;i<quota;i++){

		initEmptyClient(&(clients[i]));

	}
	server_socket= socket(AF_INET,SOCK_STREAM,0);
        int ptr=1;
        setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
        if(server_socket==-1){
                raise(SIGINT);

        }

	

        server_address.sin_family=AF_INET;
        server_address.sin_port= htons(port);
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
	servComp=gzip;
	pthread_create(&con_thread,NULL,mainLoop,NULL);
	pthread_join(con_thread,NULL);
}


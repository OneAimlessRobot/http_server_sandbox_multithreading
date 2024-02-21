
#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/resource_consts.h"
#include "../Includes/http_req_parser.h"
#include "../Includes/handlecustom.h"
#include "../Includes/server_vars.h"
#include "../Includes/client.h"
#include "../Includes/send_resource_func.h"
#include "../Includes/load_logins.h"
#include "../Includes/load_html.h"
#include "../Includes/server_vars.h"
#include "../Includes/sock_ops.h"
#include "../Includes/session_ops.h"
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
//#define READ_FUNC_TO_USE readall
#define READ_FUNC_TO_USE timedreadall
socklen_t socklenpointer;


static int sendMediaData(client*c,char* buff,char* mimetype,int compress){

	return sendResource(c,buff,mimetype,compress);
}

static void dropConnection(client*c){
getpeername(c->socket , (struct sockaddr*)&c->client_addr , (socklen_t*)&socklenpointer);
                    if(logging){
			fprintf(logstream,"Host disconnected. Cliente com ip %s , port %d \n" ,inet_ntoa(c->client_addr.sin_addr) , ntohs(c->client_addr.sin_port));
                   
                    }
                        //Close the socket and mark as 0 in lis>

                    close(c->socket);
}
void handleDisconnect(client* c){
		pthread_mutex_lock(&socketMtx);
		dropConnection(c);
		//Close the socket and mark as 0 in list for reuse
               
                    c->socket = 0;
		    c->running_time=0.0;
		pthread_mutex_unlock(&socketMtx);
}

static void handleCurrentActivity(client*c,http_request req){
	int compress=0;
	
	char targetinout[PATHSIZE]={0};
	http_header header=*(req.header);

	if(!strcmp(header.target,"/")){

		strcpy(header.target,defaultLoginTarget);
	}
	if(logging){
		fprintf(logstream,"%s\n",header.target);
	}
	if(findInStringArr(header.split_encoding,servComp.encodingExt)>=0){
		compress=compression;
	}
	switch(header.type){
	case GET:
			if(isCustomGetReq(header.target)){


				handleCustomGetReq(c,header.target,req.data,targetinout);
				int result=sendMediaData(c,targetinout,defaultMimetype,compress);
				if(result<0){

					send(c->socket,notFoundHeader,strlen(notFoundHeader),0);
				}
			}
			else{
				int isDir=sendMediaData(c,header.target,header.mimetype,compress);
				if(isDir<0){
					send(c->socket,notFoundHeader,strlen(notFoundHeader),0);
				
				}
				else if(isDir>0){
					sendMediaData(c,generateDirListing(header.target),defaultMimetype,compress);
					deleteDirListingHTML();
				}
			}
	break;
	case POST:
			if(isCustomPostReq(header.target)){

				handleCustomPostReq(c,header.target,req.data,targetinout);
				int result=sendMediaData(c,targetinout,defaultMimetype,compress);
				if(result<0){
					send(c->socket,notFoundHeader,strlen(notFoundHeader),0);

				}
			}
			else{
				int result=sendMediaData(c,header.target,header.mimetype,compress);
				if(result<0){
					send(c->socket,notFoundHeader,strlen(notFoundHeader),0);

				}
				else if(result>0){
					sendMediaData(c,generateDirListing(header.target),defaultMimetype,compress);
					deleteDirListingHTML();
				}
			}
	break;
	default:
		
		sendMediaData(c,header.target,defaultMimetype,compress);
	break;
	}
	
}

void handleConnection(client* c){
 			memset(c->peerbuff,0,PAGE_DATA_SIZE);
			http_request req;
			http_header header;
			if(READ_FUNC_TO_USE(c,c->peerbuff,PAGE_DATA_SIZE-1)!=-2){
                  	if(errno == ECONNRESET){
				handleDisconnect(c);
			}
			else if(strlen(c->peerbuff)){
				spawnHTTPRequest(c->peerbuff,&header,&req);
				if(logging){
				fprintf(logstream,"Recebemos request!!!:\n");
				
				print_http_req(logstream,req);
				}
				if(!strlen(req.data)&&(req.header->content_length>0)){
				free(req.data);
				req.data=malloc(req.header->content_length+1);
				memset(req.data,0,req.header->content_length+1);
				if(READ_FUNC_TO_USE(c,req.data,req.header->content_length)!=-2){

				

					fprintf(logstream,"Recebemos dados!!!: %s\n",req.data);
				
				}
				}
				handleCurrentActivity(c,req);
				free(req.data);
			}
			}
			else{
			
			handleDisconnect(c);
		
			}
			
}


static void initializeClient(client* c,double running_time){

FD_ZERO(&c->readfd);

        if(c->logged_in){
	double client_run_time=c->running_time;
        c->running_time=client_run_time+running_time;	
	if(c->running_time>session_time_usecs){

		c->logged_in=0;
	}
	}
pthread_mutex_lock(&socketMtx);
         int sd = c->socket;
	if(sd > 0)
                FD_SET( sd , &c->readfd);
pthread_mutex_unlock(&socketMtx);
            
        

}

static void sigpipe_handler(int signal){
	signal=0;
	if(logging){
	fprintf(logstream,"SIGPIPE!!!!!\n");
	}
}

void* runClientConnection(void* args){
struct timeval start, end;
   	long secs_used;
	signal(SIGPIPE,sigpipe_handler);

double running_time=0.0;
	client* c= (client*)args;
pthread_mutex_lock(&serverRunningMtx);
pthread_mutex_lock(&socketMtx);
while(serverOn&&c->socket){
pthread_mutex_unlock(&serverRunningMtx);
pthread_mutex_unlock(&socketMtx);
	gettimeofday(&start, NULL);
	initializeClient(c,running_time);
	if(c->socket&&FD_ISSET(c->socket,&c->readfd)){
		handleConnection(c);
	}

gettimeofday(&end, NULL);
secs_used=(end.tv_sec - start.tv_sec);

running_time=(double) (((secs_used*1000000.0) + end.tv_usec) -(double) (start.tv_usec));

pthread_mutex_lock(&serverRunningMtx);
pthread_mutex_lock(&socketMtx);
}
pthread_mutex_unlock(&serverRunningMtx);
pthread_mutex_unlock(&socketMtx);
if(logging){

	fprintf(logstream,"Client stopped!\n");
}
return NULL;
}

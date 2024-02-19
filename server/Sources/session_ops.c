#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../Includes/buffSizes.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/client.h"
#include "../Includes/server_vars.h"
#include "../Includes/server_innards.h"
#include "../Includes/sock_ops.h"
#include "../Includes/load_logins.h"
#include "../Includes/resource_consts.h"

static socklen_t socklenpointer;

static int clientIsRepeated(char username[FIELDSIZE]){
	client* arr=getFullClientArrCopy();
	int quota= getMaxNumOfClients();
	for(int i=0;i<quota;i++){
		if(stringsAreEqual(arr[i].username,username)){

			free(arr);
			return 1;
		}

	}
	free(arr);
	return 0;
}
int printAllClients(void){
	client* arr=getFullClientArrCopy();
	int quota= getMaxNumOfClients();
	for(int i=0;i<quota;i++){
		client *c = &(arr[i]);
		getpeername(c->socket , (struct sockaddr*)&c->client_addr , (socklen_t*)&socklenpointer);
        	printf("Cliente numero: %d , ip %s\n" ,i,inet_ntoa(c->client_addr.sin_addr));
		printf("Tamanho de socket: recv %d; send %d;\n" ,getSocketRecvBuffSize(c->socket) , getSocketSendBuffSize(c->socket));
		printf("Username: %s\n" ,c->username);
		printf("Curr session time (secs): %lf\n" ,(c->running_time)/1000000);
		printf("Logged in?: %d\n" ,c->logged_in);
		if(c->isAdmin){
		
			printf("\nADMIN\n");

		}
	}
	free(arr);
	return 0;
}

void handleLogin(client* c,char* fieldmess,char targetinout[PATHSIZE]){
	
	char* argv2[ARGVMAX]={0};
	int size=makeargvdelim(fieldmess,"&",argv2,ARGVMAX);
	if(!size){

	memcpy(targetinout,defaultLoginTarget,strlen(defaultLoginTarget));
	return;
	}
		getpeername(c->socket , (struct sockaddr*)&c->client_addr , (socklen_t*)&socklenpointer);
		char* username[2]={0};
		splitString(argv2[0],"=",username);
		if(!strlen(username[1])){

			memcpy(targetinout,defaultLoginTarget,strlen(defaultLoginTarget));
			return;
		}
		char* password[2]={0};
		char* correctPassword;
		splitString(argv2[1],"=",password);
		memset(c->username,0,FIELDSIZE);
		int clientExists=clientIsRepeated(username[1]);
			if(strcmp((correctPassword=find_login_pw_in_login_arr(username[1],currLogins)),"NO_SUCH_LOGIN")){
				
				if(stringsAreEqual(correctPassword,password[1])&&!clientExists){
					strncpy(c->username,username[1],FIELDSIZE);
					c->running_time=0.0;
					c->logged_in=1;
					c->isAdmin=0;
					memcpy(targetinout,defaultTarget,strlen(defaultTarget));
					printf("Bem vindo %s\n",username[1]);
					return;
				}
				
			}
	if(clientExists){
		memset(c->username,0,FIELDSIZE);
		c->isAdmin=0;
		c->running_time=0.0;
		c->logged_in=0;
		printf("Cliente repetido ok %s?\n",username[1]);
		memcpy(targetinout,doubleSessionTarget,strlen(doubleSessionTarget));
		return;
	}
	else{
	memcpy(targetinout,incorrectLoginTarget,strlen(incorrectLoginTarget));
	}
}
void handleLogout(client* c,char targetinout[PATHSIZE]){

		memset(c->username,0,FIELDSIZE);
		c->isAdmin=0;
		c->running_time=0.0;
		c->logged_in=0;
		memcpy(targetinout,defaultLoginTarget,strlen(defaultLoginTarget));

}
int clientIsLoggedIn(client* c){

	client* result= getFullClientArrCopy();
	int quota= getMaxNumOfClients();
	for(int i=0;i<quota;i++){
		getpeername(result[i].socket , (struct sockaddr*)&result[i].client_addr , (socklen_t*)&socklenpointer);
        	getpeername(c->socket , (struct sockaddr*)&c->client_addr , (socklen_t*)&socklenpointer);
        		if(stringsAreEqual(inet_ntoa(result[i].client_addr.sin_addr),inet_ntoa(c->client_addr.sin_addr))){
				if(result[i].logged_in){
					free(result);
					return 1;
					
				}



		}


	}
	free(result);
	return 0;
}
int kickClient(char ip_addr_str[FIELDSIZE]){
	int result=0;
	int quota= getMaxNumOfClients();
	for(int i=0;i<quota;i++){

        	client* c= &(clients[i]);
		getpeername(c->socket , (struct sockaddr*)&c->client_addr , (socklen_t*)&socklenpointer);
        	if(stringsAreEqual(inet_ntoa(c->client_addr.sin_addr),ip_addr_str)){
				memset(c->username,0,FIELDSIZE);
				c->isAdmin=0;
				c->socket=0;
				c->running_time=0.0;
				c->logged_in=0;
				result++;
		}
	}
	return result;
}

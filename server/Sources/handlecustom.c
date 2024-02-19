
#include "../Includes/preprocessor.h"
#include <errno.h>
#include "../Includes/buffSizes.h"
#include "../Includes/client.h"
#include "../Includes/server_vars.h"
#include "../Includes/server_innards.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/resource_consts.h"
#include "../Includes/io_ops.h"
#include "../Includes/load_logins.h"
#include "../Includes/handlecustom.h"
#include "../Includes/sock_ops.h"
#include "../Includes/session_ops.h"

const char* clientHTMLEntryStyle="<style>\n"
				"p:{\n"
				"\n"
				"font-size: 10px;\n"
				"}\n"
				"p.ADMIN:{\n"
				"color: red;\n"
				"\n"
				"}\n"
				"</style>\n";

static socklen_t socklenpointer;


char* tmpOne="/.tmp.html",* tmpTwo="/.tmp1.html";
char* tmpClients="/.tmpC.html";;
static int currvent=0;

char* customgetreqs[]={WRITE_VENT_REQ,SEE_FILES_REQ,SIGN_IN_REQ,NULL};

char* custompostreqs[]={WRITE_VENT_REQ,SIGN_IN_REQ,SEE_CLIENTS_REQ,SIGN_OUT_REQ,KICK_CLIENT_REQ,WRITE_VIDEO_REQ,NULL};


char* tmpDir=NULL;
char* tmpDir2=NULL;
char* currSearchedDir=NULL;

static void generateDirListingPrimitive(char* path){
	tmpDir=malloc(PATHSIZE);
	memset(tmpDir,0,PATHSIZE);
	tmpDir2=malloc(PATHSIZE);
	memset(tmpDir2,0,PATHSIZE);
	currSearchedDir=malloc(PATHSIZE);
	memset(currSearchedDir,0,PATHSIZE);
	snprintf(tmpDir,PATHSIZE,"%s%s",currDir,tmpOne);
	snprintf(tmpDir2,PATHSIZE,"%s%s",currDir,tmpTwo);
	int outfd= open(tmpDir,O_TRUNC|O_WRONLY|O_CREAT,0777);
	char* cmd= malloc(PATHSIZE);
	memset(cmd,0,PATHSIZE);
	snprintf(currSearchedDir,PATHSIZE,"%s%s",currDir,path);
	snprintf(cmd,PATHSIZE,"ls -1 %s > %s",currSearchedDir,tmpDir);
	system(cmd);
	free(cmd);
	close(outfd);
}
char* generateDirListing(char* dir){
	
	generateDirListingPrimitive(dir);
	int fd=	open(tmpDir2,O_TRUNC|O_WRONLY|O_CREAT,0777);
	
	if(!tmpDir||!tmpDir2){

		return NULL;
	}
	if(fd<0){
		if(logging){
		fprintf(logstream,"ERRO NAS DIRETORIAS %s\n",tmpDir2);
		}
		return NULL;
	}
	FILE* fstream;
	if(!(fstream=fopen(tmpDir,"r"))){
		if(logging){
		fprintf(logstream,"ERRO NAS DIRETORIAS %s\n",tmpDir);
		}
		return NULL;
	}
	remove(tmpDir);
	int pathsize=strlen(dir);
	if(pathsize&&!strcmp(dir +pathsize-1,"/")){

		dir[pathsize-1]=0;
	}
	int noRoot=strcmp(dir,"/resources");
	char* currListing=malloc(BUFFSIZE);
	memset(currListing,0,BUFFSIZE);
	dprintf(fd,"<!DOCTYPE html>\n<html>\n<head>\n<base href=''>\n</head>\n<body>");
	if(!noRoot){

		dprintf(fd,"\n<h1>DIRETORIA ROOT DO SERVER!!!!!</h1>");
	}
	else{
		dprintf(fd,"\n<br>\n<h2>INDEX OF: %s</h2><br>\n<br>\n",dir);
		dprintf(fd,"\n<br>\n<a href='%s/..'>Prev</a><br><br><br>\n",dir);
	
	}
	while(fgets(currListing,BUFFSIZE,fstream)){
		currListing[strlen(currListing)-1]=0;
		dprintf(fd,"<a href='%s/%s'>%s</a><br>\n",dir,currListing,currListing);
	
	}
	
	dprintf(fd,"</body>\n</html>\n");
	free(currListing);
	fclose(fstream);
	close(fd);
	free(tmpDir);
	free(tmpDir2);
	free(currSearchedDir);
	return tmpTwo;

}
static void printClientHTMLEntry(int id,client* c,int fd){

	getpeername(c->socket , (struct sockaddr*)&c->client_addr , (socklen_t*)&socklenpointer);
        dprintf(fd,"\n<p>Cliente numero: %d , ip %s , port %d\n" ,id,inet_ntoa(c->client_addr.sin_addr) , ntohs(c->client_addr.sin_port));
	dprintf(fd,"Tamanho de socket: recv %d; send %d;</p>\n" ,getSocketRecvBuffSize(c->socket) , getSocketSendBuffSize(c->socket));
	dprintf(fd,"\n<p> Username: %s</p>\n" ,c->username);
	dprintf(fd,"\n<p> Curr session time (secs): %lf</p>\n" ,(c->running_time)/1000000);
	dprintf(fd,"\n<p> Logged in?: %d</p>\n" ,c->logged_in);
	dprintf(fd,"\n<p> Socket no: %d</p>\n" ,c->socket);
	if(c->isAdmin){
		
		dprintf(fd,"\n<p class='ADMIN'>ADMIN</p>\n");

	}
	

}
static void generateClientListing(char targetinout[PATHSIZE]){
	char path[PATHSIZE]={0};
	snprintf(path,PATHSIZE,"%s%s",currDir,tmpClients);
	int fd=	open(path,O_TRUNC|O_WRONLY|O_CREAT,0777);
	client* clients=getFullClientArrCopy();
	int maxQuota=getMaxNumOfClients();
	int currQuota=getCurrNumOfClients();
	
	
	dprintf(fd,"<!DOCTYPE html>\n<html>\n<head>\n");
	dprintf(fd,"%s",clientHTMLEntryStyle);
	dprintf(fd,"\n<base href=''>\n</head>\n<body>\n");
	dprintf(fd,"<br>\n<a href='%s'>Go back</a>\n<br>\n",defaultTargetAdmin);
	dprintf(fd,"<br>\n<form id='submitbutton' method='POST' action='/seeclients'>\n<input type='submit' name='button' value='REFRESH'>\n</form>\n<br>\n");
	dprintf(fd,"\n<br>\n<h1>Ocupacao: %d/%d</h1>\n<br>\n<br>\n<br>" ,currQuota,maxQuota);
  	
	for(int i=0;i<maxQuota;i++){
	if(clients[i].socket){
		printClientHTMLEntry(i,&(clients[i]),fd);
	}
	else{
	dprintf(fd,"\n<p>Posicao %d desocupada</p>\n<br>\n" ,i);
	}
	}
	dprintf(fd,"<br>\n");
	dprintf(fd,"</body>\n</html>\n");
	close(fd);
	free(clients);
	memcpy(targetinout,tmpClients,strlen(tmpClients));

}

static void handleVentReq(char* fieldmess,char targetinout[PATHSIZE]){
	char* pathbuff=malloc(PATHSIZE*2);
	memset(pathbuff,0,PATHSIZE*2);
	char* argv2[ARGVMAX]={0};
	snprintf(pathbuff,PATHSIZE*2,"%s/resources/vents/ventNumber%d",currDir,currvent);
	FILE* stream=NULL;
	if(!(stream=fopen(pathbuff,"wb+"))){

		fprintf(logstream,"ERRO A ABRIR VENT!!!!!\n%s\n",strerror(errno));
		return;
	}
	int size=makeargvdelim(fieldmess,"&",argv2,ARGVMAX);
	for(int i=0;i<size;i++){
		char* pair[2]={0};
		splitString(argv2[i],"=",pair);
		if(!strcmp(pair[0],"venttitle")||!strcmp(pair[0],"ventcontent")){
		replaceStringCharacter(pair[1],'+',' ');
		fwrite(pair[0], 1, strlen(pair[0]),stream);
		fwrite(": ",1,2,stream);
		fwrite(pair[1],1,strlen(pair[1]),stream);
		fwrite("\n",1,1,stream);
		}
		
	}
	free(pathbuff);
	memcpy(targetinout,defaultTarget,strlen(defaultTarget));
	fclose(stream);
	currvent++;
}

static void handleVideoPostReq(char* fieldmess,char targetinout[PATHSIZE]){
	
	printf("Video posted!!!!\n");

}

int isCustomGetReq(char* nulltermedtarget){
	int result=0;
	char* targetcopy= strdup(nulltermedtarget);
	char* argv2[2];
	memset(argv2,0,2*sizeof(char*));
	splitString(targetcopy,"?",argv2);
	
	result=(findInStringArr(customgetreqs,argv2[0])>=0);
	//printf("Sera que %s e um get req custom? A resposta a isso e %d\n",argv2[0],result);
	free(targetcopy);
	return result;
}

int isCustomPostReq(char* nulltermedtarget){
	int result=0;
	result=(findInStringArr(custompostreqs,nulltermedtarget)>=0);
	return result;
}

static void kickClientHandler(char* contents,char targetinout[PATHSIZE]){

char* argv2[ARGVMAX]={0};
makeargvdelim(contents,"&",argv2,ARGVMAX);
char* pair[2]={0};
splitString(argv2[0],"=",pair);
	fprintf(logstream,"Adeus |%s|!\n",pair[1]);
	
if(kickClient(pair[1])){
	
	//if(logging){

		fprintf(logstream,"Adeus %s!\n",pair[1]);
	//}

}

strncpy(targetinout,defaultTargetAdmin,PATHSIZE);

}

void handleCustomGetReq(client* c,char*target,char* body,char targetinout[PATHSIZE]){
	char* targetcopy= strdup(target);
	char* argv2[2];
	memset(argv2,0,2*sizeof(char*));
	splitString(targetcopy,"?",argv2);
	
	if(!strcmp(argv2[0],SIGN_IN_REQ)){
		handleLogin(c,body,targetinout);

	}
	/*
	else if(!strcmp(nulltermedtarget,writeventreq)){


	}
	else if(!strcmp(nulltermedtarget,writeventreq)){


	}
	else if(!strcmp(nulltermedtarget,writeventreq)){


	}*/
	free(targetcopy);
	
}
void handleCustomPostReq(client*c,char* target,char* contents,char targetinout[PATHSIZE]){
	if(!strcmp(target,WRITE_VENT_REQ)){
		handleVentReq(contents,targetinout);
		
	}
	else if(!strcmp(target,SIGN_IN_REQ)){
		handleLogin(c,contents,targetinout);
		
	}
	else if(!strcmp(target,SEE_CLIENTS_REQ)){
		generateClientListing(targetinout);

	}
	else if(!strcmp(target,SIGN_OUT_REQ)){
		handleLogout(c,targetinout);

	}
	else if(!strcmp(target,KICK_CLIENT_REQ)){
		
		kickClientHandler(contents,targetinout);

	}
	else if(!strcmp(target,WRITE_VIDEO_REQ)){
		
		handleVideoPostReq(contents,targetinout);

	}
	/*else if(!strcmp(nulltermedtarget,writeventreq)){


	}
	else if(!strcmp(nulltermedtarget,writeventreq)){


	}
	else if(!strcmp(nulltermedtarget,writeventreq)){


	}
	else if(!strcmp(nulltermedtarget,writeventreq)){


	}*/
}

void deleteDirListingHTML(void){
char buff[PATHSIZE*2]={0};
snprintf(buff,PATHSIZE*2,"%s%s",currDir,tmpTwo);
remove(buff);

}
void deleteClientListingHTML(void){
char buff[PATHSIZE*2]={0};
snprintf(buff,PATHSIZE*2,"%s%s",currDir,tmpClients);
remove(buff);

}

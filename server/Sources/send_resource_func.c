#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/resource_consts.h"
#include "../Includes/http_req_parser.h"
#include "../Includes/handlecustom.h"
#include "../Includes/server_vars.h"
#include "../Includes/send_resource_func.h"
#include "../Includes/server_innards.h"
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
static char* compressionCmd = "%s -c %s";
//static char* compressionCmd = "gzip -c %s > %s.gzip";
static char inpathbuff[PATHSIZE*2];
static char outpathbuff[PATHSIZE*2];
static char cmd[PATHSIZE*6];
static int compressFile(char* target,int enable,int result[2]){
	memset(inpathbuff,0,PATHSIZE*2);
	snprintf(inpathbuff,(PATHSIZE*2)-1,"%s%s",currDir,target);
	if(enable){
	memset(outpathbuff,0,PATHSIZE*2);
	snprintf(outpathbuff,(PATHSIZE*2)-1,"%s.%s",inpathbuff,servComp.fileExt);
	memset(cmd,0,PATHSIZE*6);
	int fd=result[0]=open(outpathbuff,O_RDWR|O_CREAT|O_TRUNC,0777);
	if(result[0]<0){
			if(logging){
			fprintf(logstream,"Invalid filepath: %s\n%s\n",outpathbuff,strerror(errno));
			}
			return -1;
	}
	snprintf(cmd,(PATHSIZE*6)-1,compressionCmd,servComp.cmdTool,inpathbuff);
	printf("%s\n",cmd);
	char* argv[ARGVMAX]={0};
	makeargv(cmd,argv);
	int pid=fork();
	switch(pid){
		case -1:
			close(fd);
			return -1;
		case 0:
			dup2(fd,1);
			execvp(argv[0],argv);
			perror("exec na compressao\n");
			exit(-1);

		default:
			wait(NULL);
			break;


	}
	result[1]=1;
	
	}
	else{
	result[0]=open(inpathbuff,O_RDONLY,0777);
	result[1]=0;
	
	if(result[0]<0){
		if(logging){
			fprintf(logstream,"Invalid filepath: %s\n%s\n",inpathbuff,strerror(errno));
		}
		return -1;

	}
	}
	return 0;
}

int sendResource(client* c,char* resourceTarget,char* mimetype,int usefd,int compress){

	page p;
	memset(p.pagepath,0,PATHSIZE);
	char* ptr= p.pagepath;
	ptr+=snprintf(ptr,PATHSIZE,"%s%s",currDir,resourceTarget);
	int result[2]={0};
	DIR* directory=opendir(p.pagepath);
	if(directory){
		closedir(directory);
		if(logging){
			fprintf(logstream,"Diretoria encontrada!!: %s\n",p.pagepath);
		}
		return 1;

	}
	errno=0;
	if(logging){
	fprintf(logstream,"Fetching %s...\n",p.pagepath);
	}
	if(usefd){
	if(compressFile(resourceTarget,compress,result)<0){

		if(logging){

			fprintf(logstream,"Erro na compressão %s...\n",strerror(errno));
		}
		return -1;
	}
	
	}
	if(logging){
	fprintf(logstream,"Done!\n");
	}
	if(usefd){
		lseek(result[0],0,SEEK_END);
		p.data_size=lseek(result[0],0,SEEK_CUR)+1;
		lseek(result[0],0,SEEK_SET);
		
	}
	char headerBuff[PATHSIZE]={0};
	int needsDelete=0;
	if(result[1]){
	needsDelete=1;
	
		fillUpChunkedHeaderComp(headerBuff,chunkedHeaderComp,p.data_size,mimetype,servComp.fileExt);
	
	}
	else{
	
		fillUpChunkedHeader(headerBuff,chunkedHeaderComp,p.data_size,mimetype);
		
	}
		p.header_size=strlen(headerBuff);
			if(send(c->socket,headerBuff,p.header_size,0)!=(p.header_size)){
				if(logging){
				fprintf(logstream,"ERRO NO SEND!!!! O GET TEM UM ARGUMENTO!!!!:\n%s\n",strerror(errno));
				}
			}

	if(usefd){
		sendallchunkedfd(c,result[0]);
		close(result[0]);
		if(needsDelete){
			remove(outpathbuff);
		}
	}
	return 0;
}


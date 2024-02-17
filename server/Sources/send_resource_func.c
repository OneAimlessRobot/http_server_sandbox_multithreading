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
#include <dirent.h>
#include <sys/types.h>


int sendResource(client* c,char* resourceTarget,char* mimetype,int usefd){

	page p;
	memset(p.pagepath,0,PATHSIZE);
	char* ptr= p.pagepath;
	p.headerFillFunc=&fillUpChunkedHeader;
	//p.headerFillFunc=&fillUpNormalHeader;
	ptr+=snprintf(ptr,PATHSIZE,"%s%s",currDir,resourceTarget);
	
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
	if((p.pagefd=open(p.pagepath,O_RDONLY,0777))<0){
			if(logging){
			fprintf(logstream,"Invalid filepath: %s\n%s\n",p.pagepath,strerror(errno));
			}
			return -1;
	}
	}
	if(logging){
	fprintf(logstream,"Done!\n");
	}
	if(usefd){
		lseek(p.pagefd,0,SEEK_END);
		p.data_size=lseek(p.pagefd,0,SEEK_CUR)+1;
		lseek(p.pagefd,0,SEEK_SET);
		
	}
	char headerBuff[PATHSIZE]={0};
		p.headerFillFunc(headerBuff,chunkedHeader,p.data_size,mimetype);
		//p.headerFillFunc(headerBuff,normalHeader,p.data_size,mimetype);
		
		p.header_size=strlen(headerBuff);
		
			if(send(c->socket,headerBuff,p.header_size,0)!=(p.header_size)){
				if(logging){
				fprintf(logstream,"ERRO NO SEND!!!! O GET TEM UM ARGUMENTO!!!!:\n%s\n",strerror(errno));
				}
			}

	if(usefd){
		sendallchunkedfd(c,p.pagefd);
		close(p.pagefd);
	}
	return 0;
}


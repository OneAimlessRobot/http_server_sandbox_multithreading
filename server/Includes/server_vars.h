#ifndef SERVER_VARS_H
#define SERVER_VARS_H
#define COMPRESSION 0
typedef struct compMethod{
	char * cmdTool,*fileExt,*encodingExt;
	

}compMethod;
extern int logging,beeping;
extern client* clientToDrop;
extern client* clients;
extern char currDir[PATHSIZE];
extern int serverOn;
extern FILE* logstream;
extern pthread_mutex_t socketMtx,clientMtx,serverRunningMtx;
extern compMethod br,gzip,servComp;
#endif

#ifndef SERVER_VARS_H
#define SERVER_VARS_H
extern int logging,beeping;
extern client* clientToDrop;
extern client* clients;
extern char currDir[PATHSIZE];
extern int serverOn;
FILE* logstream;
extern pthread_mutex_t socketMtx,clientMtx,serverRunningMtx;
#endif

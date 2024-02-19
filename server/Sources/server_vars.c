#include "../Includes/buffSizes.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include "../Includes/client.h"
#include "../Includes/cfg_loading.h"
#include "../Includes/server_vars.h"

int logging,beeping;

client* clientToDrop;
client* clients=NULL;
char currDir[PATHSIZE];
int serverOn=0;
FILE* logstream=NULL;
pthread_mutex_t socketMtx=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t serverRunningMtx=PTHREAD_MUTEX_INITIALIZER;
compMethod br = {"brotli","br"," br"},gzip = {"gzip","gzip"," gzip"};
compMethod servComp;


char defaultTarget[FIELDSIZE]={0};
char defaultLoginTarget[FIELDSIZE]={0};
char incorrectLoginTarget[FIELDSIZE]={0};
char doubleSessionTarget[FIELDSIZE]={0};
char timeoutTarget[FIELDSIZE]={0};

char* defaultVersion="HTTP/1.1";
char* defaultMimetype="*/*";


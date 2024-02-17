#ifndef SERVER_INNARDS_H
#define SERVER_INNARDS_H
#define FIELDLENGTH 1024


void initializeServer(int max_quota,int logs);

client* getClientArrCopy(void);

client* getFullClientArrCopy(void);

int getMaxNumOfClients(void);

int getCurrNumOfClients(void);

#endif


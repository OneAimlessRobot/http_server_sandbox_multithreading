#ifndef SERVER_INNARDS_H
#define SERVER_INNARDS_H
#define FIELDLENGTH 1024


void initializeServer(void);

client** getClientArrCopy(void);

client** getFullClientArrCopy(void);

void freeClientArr(client*** arr);

int getMaxNumOfClients(void);

int getCurrNumOfClients(void);

void cleanup(void);
#endif


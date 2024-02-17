#ifndef CLIENT_H
#define CLIENT_H
typedef struct client{
	
	pthread_t threadid;
	fd_set readfd;
	char peerbuff[PAGE_DATA_SIZE];
	int socket;
	double running_time;
	char username[FIELDSIZE];
	int isAdmin;
	int logged_in;
	struct sockaddr_in client_addr;
}client;

client** initClients(void);

void* runClientConnection(void* args);

void freeClients(client***clients);
#endif

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

void* runClientConnection(void* args);
void handleDisconnect(client* c);

#endif

#ifndef CLIENT_H
#define CLIENT_H
typedef struct client{
	
	pthread_t threadid;
	fd_set readfd;
	char* peerbuff;
	int socket;
	char username[FIELDSIZE];
	int disconnected;
	pthread_mutex_t discon_mtx,sock_mtx;
	struct sockaddr_in client_addr;
	
}client;

void* runClientConnection(void* args);
void* runClientPConnection(void* args);
void handleDisconnect(client* c);

#endif

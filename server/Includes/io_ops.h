#ifndef IO_OPS_H
#define IO_OPS_H

int readall(client* c,char* buff,int64_t size);

int timedreadall(client* c,char* buff,u_int64_t size);

int sendallchunkedfd(client* c,int fd);

int sendnormalfd(client* c,int fd);

int send_in_chunks_chunked_chat_gpt(client*c, int fd);

#endif

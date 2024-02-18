#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../Includes/buffSizes.h"
#include "../Includes/http_response_aux.h"

char* chunkedHeaderComp= "HTTP/1.1 200 OK\r\n"
                          "Content-Type: %s\r\n"
                          "content-encoding: %s\r\n"
                          "Transfer-Encoding: chunked\r\n"
                          "\r\n";
char* chunkedHeader= "HTTP/1.1 200 OK\r\n"
                          "Content-Type: %s\r\n"
			  "Transfer-Encoding: chunked\r\n"
                          "\r\n";
char* normalHeader= "HTTP/1.1 200 OK\r\n"
                          "Content-Type: %s\r\n"
			  "Content-Length: %d\r\n"
 			  "\r\n";

char* redirectHeader= "HTTP/1.1 301 See Other\r\n"
				"Location: %s\r\n"
				"Content-Length: 0\r\n"
				"\r\n";

char* notFoundHeader= "HTTP/1.1 404 Not Found\r\n"
				"\n";

void fillUpRedirectHeader(char headerBuff[PATHSIZE],char* path){

	snprintf(headerBuff,PATHSIZE,redirectHeader,path);

}
//Remove annoying warning
void fillUpChunkedHeader(char headerBuff[PATHSIZE],char* headerTemplate,u_int64_t size,char* mimetype){
	size=0;
	snprintf(headerBuff,PATHSIZE,headerTemplate,mimetype+size);

}
void fillUpChunkedHeaderComp(char headerBuff[PATHSIZE],char* headerTemplate,u_int64_t size,char* mimetype,char* method){
	size=0;
	snprintf(headerBuff,PATHSIZE,headerTemplate,mimetype+size,method);

}
void fillUpNormalHeader(char headerBuff[PATHSIZE],char* headerTemplate,u_int64_t size,char* mimetype){

	snprintf(headerBuff,PATHSIZE,headerTemplate,mimetype,size);

}

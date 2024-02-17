#ifndef HTTP_RESPONSE_AUX_H
#define HTTP_RESPONSE_AUX_H

#define HTTP_OK_HEADER "HTTP/1.1 200 OK\r\n\n"
extern char* chunkedHeader,* normalHeader,* redirectHeader;
void fillUpRedirectHeader(char headerBuff[PATHSIZE],char* path);
void fillUpChunkedHeader(char headerBuff[PATHSIZE],char*headerTemplate,u_int64_t size,char* mimetype);
void fillUpNormalHeader(char headerBuff[PATHSIZE],char*headerTemplate,u_int64_t size,char* mimetype);
#endif

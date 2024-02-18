#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Includes/buffSizes.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/resource_consts.h"
#include "../Includes/http_req_parser.h"

static http_req_type getREQtypeFromString(char* str){

	if(!strcmp(str,"GET")){

		return GET;
	}
	if(!strcmp(str,"POST")){

		return POST;
	}
	if(!strcmp(str,"INFO")){

		return INFO;
	}
	return INVALID;
}
static char* getREQStringFromType(http_req_type type){

	switch(type){
	case GET:
	return "GET";
	case POST:
	return "POST";
	case INFO:
	return "INFO";
	default:
	return NULL;


	}
	return NULL;
}
void print_field_arr(FILE* fstream,http_header_field* fields){

	for (int i=0;fields[i].field[0];i++){
		
		fprintf(fstream,"Field %d:\nField type: %s\nField value: %s\n",i,fields[i].field[0],fields[i].field[1]);

	}

}
void print_http_req_header(FILE* fstream,http_header header){

		fprintf(fstream,"HTTP REQUEST HEADER:\n\nType: %s\nTarget: %s\nVersion: %s\nHost: %s\nmimetype: %s\nOS: %s\nContent-Length: %d\nAccepted-encoding: %s\nAccepted-encoding (split): \n",getREQStringFromType(header.type),header.target,header.version,header.host,header.mimetype,header.os,header.content_length,header.encodingcopy);
		print_string_arr(fstream,header.split_encoding);
}
static char* find_field_value_in_field_arr(char* fieldName,http_header_field* fields){

	for (int i=0;fields[i].field[0];i++){
		
		if(!strcmp(fields[i].field[0],fieldName)){
			return fields[i].field[1];
		}
	}
	return "";

}

http_header_field* make_http_header_field_arr(char *strarr[ARGVMAX]) {
    int size= get_string_arr_size(strarr)+1;
    http_header_field* arr=malloc(size*sizeof(http_header_field));
    memset(arr,0,size*sizeof(http_header_field));
    for(int i=0;strarr[i]&&i<size-1;i++){
	
                splitString(strarr[i],":",arr[i].field);


        }
        arr[size-1]=(http_header_field){{NULL,NULL}};
return arr;
}

static void spawnHTTPHeader(char* buff,http_header*result){
	result->type=INVALID;
	memset(result->target,0,FIELDSIZE);
	memset(result->host,0,FIELDSIZE);
	memset(result->mimetype,0,FIELDSIZE);
	memset(result->version,0,FIELDSIZE);
	memset(result->os,0,FIELDSIZE);
	char* headerArr[ARGVMAX];
	memset(headerArr,0,ARGVMAX*sizeof(char*));
	char test[strlen(buff)+1];
	memcpy(test,buff,strlen(buff)+1);
	make_str_arr(test,"\r\n",headerArr,ARGVMAX);
	http_header_field* fieldarr= make_http_header_field_arr(headerArr);
	int firstHeaderLength=strlen(fieldarr[0].field[0]);
	char firstHeaderBuff[firstHeaderLength+1];
	memset(firstHeaderBuff,0,firstHeaderLength+1);
	memcpy(firstHeaderBuff,fieldarr[0].field[0],firstHeaderLength);
	char* firstHeaderArr[ARGVMAX];
	memset(firstHeaderArr,0,ARGVMAX*sizeof(char*));
	make_str_arr(firstHeaderBuff," ",firstHeaderArr,ARGVMAX);
	result->type=getREQtypeFromString(firstHeaderArr[0]);
	if(get_string_arr_size(firstHeaderArr)==1){
		memset(result->target,0,FIELDSIZE);
		memset(result->version,0,FIELDSIZE);
		strcpy(result->target,defaultTarget);
		strcpy(result->mimetype,defaultMimetype);
	}
	else{
	strcpy(result->target,firstHeaderArr[1]);
	strcpy(result->version,firstHeaderArr[2]);
	strcpy(result->host,find_field_value_in_field_arr("Host",fieldarr));
	//strcpy(result.mimetype,find_field_value_in_field_arr("Accept",fieldarr));
	strcpy(result->mimetype,defaultMimetype);
	strcpy(result->os,find_field_value_in_field_arr("sec-ch-ua-platform",fieldarr));
	strcpy(result->encoding,find_field_value_in_field_arr("Accept-Encoding",fieldarr));
	strcpy(result->encodingcopy,result->encoding);
	make_str_arr(result->encoding,",",result->split_encoding,ARGVMAX);
	result->content_length=atoi(find_field_value_in_field_arr("Content-Length",fieldarr));
	}
	free(fieldarr);
	
}

void spawnHTTPRequest(char *buff,http_header* header,http_request* reqresult){
	char* splitRequest[2];
	splitString(buff,"\r\n\r\n",splitRequest);
	spawnHTTPHeader(splitRequest[0],header);
	reqresult->header=header;
	if(!splitRequest[1]){
	reqresult->data_size=1;
	}
	else{
	reqresult->data_size=strlen(splitRequest[1])+1;
	
	}
	reqresult->data=malloc(reqresult->data_size);
	memset(reqresult->data,0,reqresult->data_size);
	if(splitRequest[1]){
	memcpy(reqresult->data,splitRequest[1],reqresult->data_size-1);
	}

}


void print_http_req(FILE* fstream,http_request req){
	fprintf(fstream,"HTTP_REQUEST:\n");
	print_http_req_header(fstream,*(req.header));
	fprintf(fstream,"HTTP_REQUEST_CONTENTS=%s\n",req.data);

}

#ifndef RESOURCE_CONSTS_H
#define RESOURCE_CONSTS_H
typedef void (*header_fill_func_type)(char headerBuff[PATHSIZE],char* headerTemplate, u_int64_t size,char* ext);

typedef struct page{

	int pagefd;
	FILE* pagestream;
	char  pagepath[PATHSIZE];
	int header_size,data_size;
	header_fill_func_type headerFillFunc;
}page;
extern char* args[];
extern page mainpage,notfoundpage;

extern char* logfpath,* defaultTarget,
* defaultVersion,
* defaultMimetype,
*notFoundTarget,
*testScriptTarget,
*defaultTargetAdmin,
*defaultLoginTarget,
*doubleSessionTarget,
*timeoutTarget,
*incorrectLoginTarget;

extern FILE* logstream;

#endif

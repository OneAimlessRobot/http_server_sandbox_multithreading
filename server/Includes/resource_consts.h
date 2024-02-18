#ifndef RESOURCE_CONSTS_H
#define RESOURCE_CONSTS_H

typedef struct page{

	int pagefd;
	FILE* pagestream;
	char  pagepath[PATHSIZE];
	int header_size,data_size;
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

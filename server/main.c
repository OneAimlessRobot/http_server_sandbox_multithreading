#include "Includes/preprocessor.h"
#include "Includes/auxFuncs.h"
#include "Includes/handlecustom.h"
#include "Includes/server_innards.h"
#include "Includes/load_logins.h"

int main(int argc, char ** argv){

	if(argc !=(get_string_arr_size(args)+1)){

		fprintf(stderr,"Precisas de mais argumentos:");
		print_string_arr(stderr,args);
		fprintf(stderr,"\n");
		exit(-1);
	}
	int nclients=0;
	if((nclients=atoi(argv[1]))<=0){
	
		fprintf(stderr,"Numero invalido de clientes! (%d <= 0)\n",nclients);
		exit(-1);
	}

	//generateDirListing(argv[1]);
		//getcwd(currDir,PATHSIZE);
	initializeServer(nclients,atoi(argv[2]));
	return 0;
}

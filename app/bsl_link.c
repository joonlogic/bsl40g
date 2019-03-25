#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "bsl_type.h"
#include "bsl_system.h"
#include "bsl_api.h"
#include "bsl_ext.h"

const static char* this_version = __DATE__;
extern char* bsl_getErrorStr( EnumResultCode );

void help(char *progname)
{
	printf("%s Link status check program.\n", progname );
	printf("      -h : Help\n");
	printf("      -v : Version\n");
	printf("      -c : Cardid. default 0\n");
}

int main(int argc, char *argv[])
{
	int opt, opt_ok = 0;
	int opt_version = 0;
	int ret;
	int cardid=0;

	opt_ok = 1;

	while ((opt = getopt(argc, argv, "hvc:")) != -1) {
		switch (opt) {
			case 'h':
				opt_ok = 0;
				break;
			case 'v':
				opt_ok = 1;
				opt_version = 1;
				break;
			case 'c':
				opt_ok = 1;
				cardid = strtol( optarg, NULL, 10 );
				break;
		}
	}

	if (opt_ok != 1) {
		help(argv[0]);
		exit(0);
	}

	if( opt_version ) {
		fprintf( stderr, "%s : Build %s\n", argv[0], this_version );
		return 0;
	}

	T_BslCard card = {0,};
	ret = bsl_getLinkStatus( cardid, &card );
	if( ret != 0 ) {
		fprintf( stderr, "%s : ERROR, ret %d\n", argv[0], ret );
		return 0;
	}

	/*
	fprintf( stderr, "\n\n==============================================\n");
	if( card.port[0].opstate == LinkUp ) 
		fprintf( stderr, "Card[%d] Port[0] Link Up\n", cardid );
	else if( card.port[0].opstate == LinkDown ) 
		fprintf( stderr, "Card[%d] Port[0] Link Down\n", cardid );
	else
		fprintf( stderr, "Card[%d] Port[0] Link Unknown\n", cardid );


	fprintf( stderr, "==============================================\n\n\n\n");
	*/

	return 0;
}


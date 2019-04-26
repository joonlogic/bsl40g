#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
	printf("%s Chassis status check program.\n", progname );
	printf("      -h : Help\n");
	printf("      -v : Version\n");
}

int main(int argc, char *argv[])
{
	int opt, opt_ok = 0;
	int opt_version = 0;
	int ret;
	int i, j;
	T_BslSystem system = {0,};

	opt_ok = 1;

	while ((opt = getopt(argc, argv, "hv:")) != -1) {
		switch (opt) {
			case 'h':
				opt_ok = 0;
				break;
			case 'v':
				opt_ok = 1;
				opt_version = 1;
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

	ret = bsl_getCardConfig(&system, false);


	printf("\t\t==============================\n");
	printf("\tInstalled Cards : Total %d\n", system.ncards);

	for(i=0; i<system.ncards; i++) {
		T_BslCard* card = &system.card[i];
		printf("\t[%d] %s : Installed Ports (%d)\n",
		i,
		card->port[0].speed == LinkSpeed1Gbps ? "1G" :
		card->port[0].speed == LinkSpeed10Gbps ? "10G" :
		card->port[0].speed == LinkSpeed40Gbps ? "40G" :
		card->port[0].speed == LinkSpeed100Gbps ? "100G" :
		"Unknown",
		card->nports);

		bsl_getLinkStatus(i, card);
		for(j=0; j<card->nports; j++) {
			printf("\t\tPort[%d] Link Status : %s\n",
			j, 
			card->port[j].opstate == LinkUp ? "UP" : "DOWN");
		}
	}

	return ret;
}


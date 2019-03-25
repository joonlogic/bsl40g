#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "bsl_type.h"
#include "bsl_system.h"
#include "bsl_dbg.h"
#include "bsl_api.h"
#include "bsl_ctl.h"
#include "bsl_ext.h"

const static char* this_version = __DATE__;
extern char* bsl_getErrorStr( EnumResultCode );
static void get_version( int cardid, char* progname );

void help(char *progname)
{
	printf("%s 10G measurement main program.\n", progname );
	printf("      -h : Help\n");
	printf("      -v : Version\n");
	printf("      -c : Cardid. just to get version\n");
	printf("      -d : Daemon mode\n");
	printf("      -s : Start Service\n");
	printf("      -p : Server TCP Port\n");
}

int main(int argc, char *argv[])
{
	int opt, opt_ok = 0;
	int opt_version = 0;
	int isdaemon = 0;
	int cardid=0;
	int socketport=TCP_PORT_LISTEN; //default 7788

	opt_ok = 0;

	while ((opt = getopt(argc, argv, "hvsdc:p:")) != -1) {
		switch (opt) {
			case 'h':
				break;
			case 'v':
				opt_ok = 1;
				opt_version = 1;
				break;
			case 's':
				opt_ok = 1;
				break;
			case 'd':
				opt_ok = 1;
				isdaemon = 1;
				break;
			case 'c':
				opt_ok = 1;
				cardid = strtol( optarg, NULL, 10 );
				break;
			case 'p':
				opt_ok = 1;
				socketport = strtol( optarg, NULL, 10 );
				break;
		}
	}

	if (opt_ok != 1) {
		help(argv[0]);
		exit(0);
	}

	if( opt_version ) {
		get_version( cardid, argv[0] );
		return 0;
	}

	bsl_clearDmaBuffer( cardid );

	if( isdaemon ) 
		daemon(0,0);

	fprintf( stderr, "%s: opening msg_listener\n", __func__ );
	//bsl_open_msg_listener(); //deprecated
	do {
		bsl_msgif_listen_loop(socketport);
	} while(1);

	return 0;
}

void get_version( int cardid, char* progname )
{
	EnumResultCode result;
	T_Version ver;
	result = bsl_getVersionInfo( cardid, &ver );

	fprintf( stderr, "%s : Version %s\n", progname, this_version );
	if( result != ResultSuccess ) {
		fprintf( stderr, "\tbsl_getVersionInfo() Error %s\n",
				bsl_getErrorStr( result ) );
		return;
	}

	fprintf( stderr, "%s : board  %s\n", progname, ver.board );
	fprintf( stderr, "%s : fpga   %s\n", progname, ver.fpga );
	fprintf( stderr, "%s : driver %s\n", progname, ver.driver );
	fprintf( stderr, "%s : api    %s\n", progname, ver.api );

	return;
}


/********************************************************************
 *  FILE   : bsl_api.h
 *
 *  Library : 
 *  Export Function :
 *
 ********************************************************************
 *                    Change Log
 *
 *    Date			Author			Note
 *  -----------   -----------  ----------------------------------   
 *  2013.11.13       joon        This File is written first.
 *  2016.04.09       joon        01.00.00 release first
 *  2016.04.17       joon        01.00.01 add message 101
 *  2016.05.10       joon        01.00.02 touch message 108 wmcr
 *								         
 ********************************************************************/

#ifndef BSL_API_H
#define BSL_API_H

#include "bsl_type.h"
#include "bsl_proto.h"

#ifdef _TARGET_
#define OPEN_DEVICE(cardid)                   \
	cardid == 0 ?                             \
	open("/dev/bsl_ctl", O_RDWR, (mode_t)0600) : \
	cardid == 1 ?                             \
	open("/dev/bsl_ctl-1", O_RDWR, (mode_t)0600) : \
	cardid == 2 ?                             \
	open("/dev/bsl_ctl-2", O_RDWR, (mode_t)0600) : \
	cardid == 3 ?                             \
	open("/dev/bsl_ctl-3", O_RDWR, (mode_t)0600) : -1
#else
#define OPEN_DEVICE(cardid)             100
#endif

#define BSL_CHECK_DEVICE(fd)            \
	if( fd < 0 ) { \
		fprintf( stderr, "%s> device file open failure. fd %d\n", \
				__func__, fd ); \
		return -1; \
	}

#endif //BSL_API_H

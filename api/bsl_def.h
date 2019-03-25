/********************************************************************
 *  FILE   : bsl_def.h
 ********************************************************************/

#ifndef BSL_DEF_H
#define BSL_DEF_H

#define SIZE_STRING_STREAM_NAME         64
#define SIZE_MAX_STREAM                 256
#define SIZE_MAX_PORT                   4  //equal to MAX_NPORTS
#define SIZE_MAX_CARD                   4
#define SIZE_MAX_PORTID                 3  //From 0
#define SIZE_MAX_CARDID                 3  //From 0
#define SIZE_MAX_PAYLOAD                65536
#define SIZE_MAX_VERSION_STRING         64        

#define SIZE_NFRAME_RANDOM              32
#define SIZE_NFRAME_RANDOM_40G          128

#define ID_CARD_0                       0
#define ID_CARD_1                       1
#define ID_CARD(ID)                     (ID)

#define ID_PORT_0                       0
#define ID_PORT_1                       1
#define ID_PORT(ID)                     (ID)

#ifdef _TARGET_
#define TCP_PORT_LISTEN                 7788
#define TCP_PORT_SEND                   7789
#else
#define TCP_PORT_LISTEN                 8900
#define TCP_PORT_SEND                   8901
#endif
#define IP_LOCALHOST                    "0.0.0.0"

#define SIZE_MSGIF_BUF                  1024
#define SIZE_MSGIF_HEADER               20

#define TIMEOUT_SEC_MSGIF               0

#define SIZE_FRAME_MIN                  60
#define SIZE_FRAME_MAX                  1514

//UTIL                                                                                      
#ifdef WORDS_BIGENDIAN
#define htonll(x)   (x)
#define ntohll(x)   (x)
#else
#define htonll(x) ((((unsigned long long)htonl(x)) << 32) + \
		htonl((unsigned long long)x >> 32))
#define ntohll(x) ((((unsigned long long)ntohl(x)) << 32) + \
		ntohl((unsigned long long)x >> 32))
#endif                                                                                      
#define BSLSWAP32(val) \
	    ((unsigned int)((((unsigned int)(val) & (unsigned int)0x000000ffU) << 24) | \
		(((unsigned int)(val) & (unsigned int)0x0000ff00U) <<  8) | \
		(((unsigned int)(val) & (unsigned int)0x00ff0000U) >>  8) | \
		(((unsigned int)(val) & (unsigned int)0xff000000U) >> 24)))

//BSLSWAP40
//INPUT  : AABBCCDD_EEFFFFFF
//OUTPUT : EEDDCCBB_AAFFFFFF
#define BSLSWAP40(val) \
	    ((unsigned long long)( \
		(((unsigned long long)(val) & (unsigned long long)0x00000000FF000000ULL) << 32) | \
		(((unsigned long long)(val) & (unsigned long long)0x000000FF00000000ULL) << 16) | \
		(((unsigned long long)(val) & (unsigned long long)0x0000FF0000000000ULL) << 0 ) | \
		(((unsigned long long)(val) & (unsigned long long)0x00FF000000000000ULL) >> 16) | \
		(((unsigned long long)(val) & (unsigned long long)0xFF00000000000000ULL) >> 32) | \
		(((unsigned long long)(val) & (unsigned long long)0x0000000000FFFFFFULL) )))

#define BSLSWAP64(val) \
	    ((unsigned long long)((((unsigned long long)(val) & (unsigned long long)0x000000ffULL) << 56) | \
		(((unsigned long long)(val) & (unsigned long long)0x0000ff00ULL) << 40) | \
		(((unsigned long long)(val) & (unsigned long long)0x00ff0000ULL) << 24) | \
		(((unsigned long long)(val) & (unsigned long long)0xff000000ULL) << 8 ) | \
		(((unsigned long long)(val) & (unsigned long long)0xff00000000ULL) >> 8) | \
		(((unsigned long long)(val) & (unsigned long long)0xff0000000000ULL) >> 24) | \
		(((unsigned long long)(val) & (unsigned long long)0xff000000000000ULL) >> 40) | \
		(((unsigned long long)(val) & (unsigned long long)0xff00000000000000ULL) >> 56)))

#ifdef _TARGET_

#define READ32(base, offset)            (*(unsigned int *)(base+(offset)))
#define WRITE32(base, offset, value)    (*(unsigned int *)(base+(offset))=value)
#define READ64(base, offset)            (*(unsigned long long*)(base+((offset)<<3)))
#define WRITE64(base, offset, value)    (*(unsigned long long*)(base+((offset)<<3))=value)
#define READ64_ZEROSHIFT(base, offset)            (*(unsigned long long*)(base+(offset)))
#define WRITE64_ZEROSHIFT(base, offset, value)    (*(unsigned long long*)(base+(offset))=value)
//WRITE64_EXT is for 40G. 
//It needs writing for additional 3 internal ports.
#define WRITE64_EXT(base, offset, value) \
	do { \
		WRITE64(base, offset, value); \
		link40G ? \
			WRITE64(base, OFFSET_REGISTER_PORT(ID_PORT(1))+offset, value), \
			WRITE64(base, OFFSET_REGISTER_PORT(ID_PORT(2))+offset, value), \
			WRITE64(base, OFFSET_REGISTER_PORT(ID_PORT(3))+offset, value) : \
			value; \
	} while(0)

#else //_TARGET_
#define READ32(base, offset)            0
#define WRITE32(base, offset, value)    
#define READ64(base, offset)            0ll
#define WRITE64(base, offset, value)    (void)(offset)
#endif //_TARGET_


#endif //BSL_DEF_H

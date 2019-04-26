/********************************************************************
 *  FILE   : bsl_api.c
 ********************************************************************/

#include <stdbool.h>
#include <arpa/inet.h>
#include "bsl_type.h"
#include "bsl_def.h"
#include "bsl_dbg.h"
#include "bsl_system.h"
#include "bsl_msgif.h"
#include "bsl_api.h"
#include "bsl_proto.h"
#include "bsl_ext.h"

#include "../module/bsl_ctl.h"

static void makeHeaderEthernet( T_Protocol* proto );
static void makeHeaderVLAN( T_Protocol* proto );
static void makeHeaderISL( T_Protocol* proto );
static void makeHeaderMPLS( T_Protocol* proto );
static void makeHeaderIp4( T_Protocol* proto );
static void makeHeaderIp6( T_Protocol* proto );
static void makeHeaderIp4OverIp6( T_Protocol* proto );
static void makeHeaderIp6OverIp4( T_Protocol* proto );
static void makeHeaderArp( T_Protocol* proto );
static void makeHeaderTcp( T_Protocol* proto );
static void makeHeaderUdp( T_Protocol* proto );
static void makeHeaderIcmp( T_Protocol* proto );
static void makeHeaderIgmpv2( T_Protocol* proto );
static void makeHeaderUDF( T_Protocol* proto );

typedef void (*makeHeader_f_ptr_type) ( T_Protocol* proto );
static makeHeader_f_ptr_type makeHeader_f_ptr[SIZE_MAX_PROTOCOL_ID] = {
	[0] = NULL,
	[21] = (makeHeader_f_ptr_type)&makeHeaderEthernet,
	[22] = (makeHeader_f_ptr_type)&makeHeaderVLAN,
	[23] = (makeHeader_f_ptr_type)&makeHeaderISL,
	[24] = (makeHeader_f_ptr_type)&makeHeaderMPLS, 
	[31] = (makeHeader_f_ptr_type)&makeHeaderIp4,
	[32] = (makeHeader_f_ptr_type)&makeHeaderIp6,
	[33] = (makeHeader_f_ptr_type)&makeHeaderIp4OverIp6,
	[34] = (makeHeader_f_ptr_type)&makeHeaderIp6OverIp4,
	[35] = (makeHeader_f_ptr_type)&makeHeaderArp,
	[41] = (makeHeader_f_ptr_type)&makeHeaderTcp,
	[42] = (makeHeader_f_ptr_type)&makeHeaderUdp,
	[43] = (makeHeader_f_ptr_type)&makeHeaderIcmp,
	[44] = (makeHeader_f_ptr_type)&makeHeaderIgmpv2,
	[51] = (makeHeader_f_ptr_type)&makeHeaderUDF,
};
static EnumResultCode bsl_makeHeader( T_Protocol* proto );

//
//makeupPdr fuction pointers for making up 40G internal ports 0-3
//
static void 
makeupPdrEthernet(int portid, T_Protocol* proto),
makeupPdrVLAN(int portid, T_Protocol* proto),
makeupPdrISL(int portid, T_Protocol* proto),
makeupPdrMPLS(int portid, T_Protocol* proto),
makeupPdrIp4(int portid, T_Protocol* proto),
makeupPdrIp6(int portid, T_Protocol* proto),
makeupPdrIp4OverIp6(int portid, T_Protocol* proto),
makeupPdrIp6OverIp4(int portid, T_Protocol* proto),
makeupPdrArp(int portid, T_Protocol* proto),
makeupPdrTcp(int portid, T_Protocol* proto),
makeupPdrUdp(int portid, T_Protocol* proto),
makeupPdrIcmp(int portid, T_Protocol* proto),
makeupPdrIgmpv2(int portid, T_Protocol* proto),
makeupPdrUDF(int portid, T_Protocol* proto);

typedef void (*makeupPdr_f_ptr_type) (int portid, T_Protocol* proto);
static makeupPdr_f_ptr_type makeupPdr_f_ptr[SIZE_MAX_PROTOCOL_ID] = {
	[0] = NULL,
	[21] = (makeupPdr_f_ptr_type)&makeupPdrEthernet,
	[22] = (makeupPdr_f_ptr_type)&makeupPdrVLAN,
	[23] = (makeupPdr_f_ptr_type)&makeupPdrISL,
	[24] = (makeupPdr_f_ptr_type)&makeupPdrMPLS, 
	[31] = (makeupPdr_f_ptr_type)&makeupPdrIp4,
	[32] = (makeupPdr_f_ptr_type)&makeupPdrIp6,
	[33] = (makeupPdr_f_ptr_type)&makeupPdrIp4OverIp6,
	[34] = (makeupPdr_f_ptr_type)&makeupPdrIp6OverIp4,
	[35] = (makeupPdr_f_ptr_type)&makeupPdrArp,
	[41] = (makeupPdr_f_ptr_type)&makeupPdrTcp,
	[42] = (makeupPdr_f_ptr_type)&makeupPdrUdp,
	[43] = (makeupPdr_f_ptr_type)&makeupPdrIcmp,
	[44] = (makeupPdr_f_ptr_type)&makeupPdrIgmpv2,
	[51] = (makeupPdr_f_ptr_type)&makeupPdrUDF,
};
static EnumResultCode bsl_makeupPdr(int portid, T_Protocol* proto);
static EnumResultCode bsl_makeupFrame(int portid, T_Frame* frame);


char* tostring_bslsystem(T_BslSystem* bsl)
{
	BSL_CHECK_NULL(bsl, NULL);

	char* sbsl = malloc(128);
	sprintf(sbsl, "Installed Cards (%d)\n", bsl->ncards);
	for(int i=0; i<bsl->ncards; i++) {
		sprintf(sbsl+strlen(sbsl), "[%d] %s : Installed Ports (%d)\n", 
				i, 
				bsl->card[i].port[0].speed == LinkSpeed1Gbps ? "1G" :
				bsl->card[i].port[0].speed == LinkSpeed10Gbps ? "10G" :
				bsl->card[i].port[0].speed == LinkSpeed40Gbps ? "40G" :
				bsl->card[i].port[0].speed == LinkSpeed100Gbps ? "100G" :
				"Unknown",
				bsl->card[i].nports);
	}

	return sbsl; 
}

EnumResultCode 
bsl_getCardConfig(T_BslSystem* bsl, bool verbose)
{
	int fd = 0;
	int ret = 0;
	BSL_CHECK_NULL( bsl, ResultNullArg );

	fd = OPEN_DEVICE(0);
	BSL_CHECK_DEVICE( fd );

	ret = ioctl( fd, CMD_GET_BSL_CONFIG, bsl );
	BSL_CHECK_IOCTL( ret, ResultIoctlFailure );

	if(verbose) {
		char* sbsl = tostring_bslsystem(bsl);
		BSL_INFO(("%s> %s\n", __func__, sbsl));
		if(sbsl) free(sbsl);
	}

	close( fd );
	return ResultSuccess;
}

EnumResultCode 
bsl_getNumberOfCards( int* nCards )
{
	int fd = 0;
	int ret = 0;
	BSL_CHECK_NULL( nCards, ResultNullArg );

	fd = OPEN_DEVICE(0);
	BSL_CHECK_DEVICE( fd );

	ret = ioctl( fd, CMD_GET_DEV_COUNT, nCards );
	BSL_CHECK_IOCTL( ret, ResultIoctlFailure );

#ifndef _TARGET_
	*nCards = 2;
#endif
	BSL_INFO(("%s> number of cards installed %d\n", __func__, *nCards ));

	close( fd );
	return ResultSuccess;
}

EnumResultCode 
bsl_getVersionInfo( int cardid, T_Version* ver )
{
	int fd = 0;
	int ret = 0;
	BSL_CHECK_NULL( ver, ResultNullArg );
	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );

	fd = OPEN_DEVICE(cardid);
	BSL_CHECK_DEVICE( fd );

	//FPGA
	ret = ioctl( fd, CMD_UTIL_VERSION_FPGA_GET, ver->fpga );
	BSL_CHECK_IOCTL( ret, ResultIoctlFailure );

	//DRIVER
	ret = ioctl( fd, CMD_UTIL_VERSION_SW_GET, ver->driver );
	BSL_CHECK_IOCTL( ret, ResultIoctlFailure );

	//API
	strcpy( ver->api, VERSION_BSL_API_STR );

	//TODO: BOARD
	strcpy( ver->board, "Unknown" );

	BSL_INFO(("%s> FPGA %s DRIVER %s API %s\n", 
				__func__, ver->fpga, ver->driver, ver->api ));

	close( fd );
	return ResultSuccess;
}

EnumResultCode 
bsl_getChassis( T_Chassis* chassis )
{
	int fd = 0;
	int ret = 0;
	int linkstatus = 0;
	int cardid = 0;
	int nCards = 0;

	BSL_CHECK_NULL( chassis, ResultNullArg );

	bsl_getNumberOfCards( &nCards );

	for( cardid=0; cardid < nCards; cardid++ ) {
		fd = OPEN_DEVICE( cardid );
		if( fd < 0 ) {
			fprintf( stderr, "%s: device file open error. fd %d cardid %d\n",
					__func__, fd, cardid );
			continue;
		}

		//TODO: status of chassis
		chassis->status = ChassisStatusActive;
		//TODO: status of card
		chassis->card[cardid].status = CardStatusActive;
		//TODO: status of ports
		chassis->card[cardid].port[0].status = PortStatusActive;
		chassis->card[cardid].port[0].linkType = LinkType10GE;

		chassis->card[cardid].port[1].status = PortStatusActive;
		chassis->card[cardid].port[1].linkType = LinkType10GE;

		//TODO: operState
		ret = ioctl( fd, CMD_UTIL_PORT_LINK_GET, &linkstatus );
		BSL_CHECK_IOCTL( ret, ResultIoctlFailure );

		chassis->card[cardid].port[0].link.operState = 
			linkstatus & (1<<(ID_PORT_0)) ? LinkUp : LinkDown;
		chassis->card[cardid].port[1].link.operState = 
			linkstatus & (1<<(ID_PORT_1)) ? LinkUp : LinkDown;

		close( fd );

		BSL_INFO(("%s> port[0] %s port[1] %s\n",
					__func__, linkstatus & (1<<(ID_PORT_0)) ? "LinkUp" : "LinkDown",
					linkstatus & (1<<(ID_PORT_1)) ? "LinkUp" : "LinkDown" ));

	}
	return ResultSuccess;
}

EnumResultCode 
bsl_getLinkStatus( int cardid, T_BslCard* card )
{
	int fd = 0;
	int ret = 0;
	int linkstatus = 0;

	BSL_CHECK_NULL( card, ResultNullArg );

	fd = OPEN_DEVICE( cardid );
	if( fd < 0 ) {
		fprintf( stderr, "%s: device file open error. fd %d cardid %d\n",
				__func__, fd, cardid );
		return ResultOpenDeviceError;
	}

#if 0
	ret = ioctl( fd, CMD_UTIL_PORT_LINK_GET, &linkstatus );
	BSL_CHECK_IOCTL( ret, ResultIoctlFailure );
#else
	ret = bsl_device_getLinkStatus( fd, &linkstatus, &card->nports );
#endif
	close( fd );

	BSL_CHECK_RESULT( ret, ret );

	for(int i=0; i<card->nports; i++) {
		card->port[i].opstate = 
			linkstatus & (1<<i) ? LinkUp : LinkDown;

		BSL_INFO(("%s> port[%d] %s\n",
				__func__, i, card->port[i].opstate == LinkUp ? "LinkUp" : "LinkDown"));
	}

	return ResultSuccess;
}

EnumResultCode 
bsl_getLinkStats( int cardid, int portid, T_LinkStats* stat )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_NULL( stat, ResultNullArg );
	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );

	fd = OPEN_DEVICE(cardid);
	BSL_CHECK_DEVICE( fd );

	stat->portid = portid;
	ret = bsl_device_getStats( fd, stat );

	close( fd );

	BSL_CHECK_RESULT( ret, ret );
	BSL_INFO(("%s> port[%d] statistics ----\n"
				"timestamp %d framesSent %llu byteSent %llu\n"
				"validFrameReceived %llu validByteReceived %llu\n"
				"fragments %llu fragmentsByteReceived %llu crcErrors %llu\n"
				"vlanTaggedFrames %llu crcErrorByteReceived %llu\n",
				__func__, portid, (unsigned int)stat->timestamp, 
				stat->framesSent, 
				stat->byteSent, stat->validFrameReceived, 
				stat->validByteReceived, stat->fragments, stat->fragmentsByteReceived, 
				stat->crcErrors, stat->vlanTaggedFrames, 
				stat->crcErrorByteReceived ));

	return ResultSuccess;
}

EnumResultCode 
bsl_setPortMode( int cardid, int portid, EnumPortOpMode mode )
{
	int fd = 0;
	int ret = 0;

	BSL_INFO(("%s> card[%d] port[%d] mode[%d] \n",
				__func__, cardid, portid, mode ));

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	ret = bsl_device_setPortMode( fd, portid, mode );
	close( fd );

	BSL_INFO(("%s> card[%d] port[%d] mode[%d] ret %d\n",
				__func__, cardid, portid, mode, ret ));

	BSL_CHECK_RESULT( ret, ret );

	return ResultSuccess;
}

EnumResultCode 
bsl_setPortActive( int cardid, int portid, EnumPortActive enable )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	ret = bsl_device_setPortActive( fd, portid, enable );
	close( fd );

	BSL_INFO(("%s> card[%d] port[%d] enable[%d] ret %d\n",
				__func__, cardid, portid, enable, ret ));

	BSL_CHECK_RESULT( ret, ret );

	return ResultSuccess;
}

EnumResultCode 
bsl_setLatency( int cardid, int portid, int latency_enable, int sequence_enable, int signature_enable )	
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

//	ret = bsl_device_setLatency( fd, portid, latency_enable, sequence_enable );
	ret = bsl_device_setLatency( fd, portid, latency_enable, sequence_enable, signature_enable );							// 2016.12.31 by dgjung
	close( fd );

	BSL_CHECK_RESULT( ret, ret );

	return ResultSuccess;
}


EnumResultCode 
bsl_setCaptureStartStop( 
		int cardid, 
		int portid, 
		EnumCaptureMode mode, 
		unsigned int size, 
		int start )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	ret = bsl_device_setCapture( fd, cardid, portid, mode, size, start );
	//close( fd ); //should be closed at bsl_device_setCapture()

	BSL_INFO(("%s> card[%d] port[%d] mode %d start %d ret %d\n",
				__func__, cardid, portid, mode, start, ret ));

	BSL_CHECK_RESULT( ret, ret );

	return ResultSuccess;
}

EnumResultCode 
bsl_setControlCommand( 
		int portsel, 
		int cardid, 
		int portid, 
		int streamid, 
		EnumCommand command, 
		unsigned long long clocks,
		unsigned int netmode,
		unsigned long long mac,
		unsigned int ip )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	ret = bsl_device_setControlCommand( fd, portsel, portid, streamid, command, clocks, netmode, mac, ip );
	close( fd );

	BSL_INFO(("%s> card[%d] portsel[%d], port[%d] stream[%d] command[%d] clocks[%llu] ret %d\n",
				__func__, cardid, portsel, portid, streamid, command, clocks, ret ));

	BSL_CHECK_RESULT( ret, ret );
	return ret;
}

EnumResultCode 
bsl_enableStream( int cardid, int portid, int streamid, int enable )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	ret = bsl_device_enableStream( fd, portid, streamid, enable );
	close( fd );

	BSL_INFO(("%s> card[%d] port[%d] stream[%d] enable %d\n",
				__func__, cardid, portid, streamid, enable )); 

	BSL_CHECK_RESULT( ret, ret );
	return ret;
}


EnumResultCode 
bsl_setStreamDetail( 
		int cardid, 
		int portid, 
		int streamid, 
		int groupid, 
		T_Stream* stream, 
		T_Protocol* proto )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_NULL( proto, ResultNullArg );
	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );
	BSL_CHECK_EXP( ((portid<0)||(portid>SIZE_MAX_PORTID)), ResultOutOfRange );

	bool link40G = stream->enable == EnableStream40G ? true : false;
	BSL_CHECK_EXP((link40G && portid > 0), ResultOutOfRange);

	//for 40G
	T_Protocol protoext[MAX_NPORTS];
	T_Stream streamext[MAX_NPORTS];

	if(link40G) {
		for(int i=0; i<MAX_NPORTS; i++) {
			memcpy(&protoext[i], proto, sizeof(T_Protocol));
			memcpy(&streamext[i], stream, sizeof(T_Stream));

			bsl_makeupPdr(i, &protoext[i]);
			bsl_makeupFrame(i, &protoext[i].fi);
			bsl_makeHeader(&protoext[i]);
		} 
	}
	else {
		bsl_makeHeader( proto ); 
	}

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	if(link40G) {
		int i=0;
		do {
			ret = bsl_device_setStreamDetail(fd, i, streamid, groupid, &streamext[i], &protoext[i]);
		} while((ret == ResultSuccess) && (++i < MAX_NPORTS));
	}
	else {
		ret = bsl_device_setStreamDetail( fd, portid, streamid, groupid, stream, proto );
	}
	close( fd );

	BSL_INFO(("%s> card[%d] port[%d] stream[%d] groupid[%d] information ----\n",
				__func__, cardid, portid, groupid, streamid ));				
	BSL_INFO((  "specFrameSize %d\n"
				"sizeOrStep %d sizeMin %d sizeMax %d\n"
				"payloadType %d payloadOffset %d\n"
				"payloadValidSize %d\n",
				proto->fi.framesize.fsizeSpec, proto->fi.framesize.sizeOrStep,
				proto->fi.framesize.fsizeMin, proto->fi.framesize.fsizeMax,
				proto->fi.payloadType, proto->fi.pattern.payloadOffset,
				proto->fi.pattern.validSize ));

	BSL_CHECK_RESULT( ret, ret );
	return ResultSuccess;
}

char* bsl_getErrorStr( EnumResultCode code )
{
	if( code == ResultGeneralError ) return "GeneralError";
	else if( code == ResultSuccess ) return "Success";
	else if( code == ResultAlreadyExist ) return "AlreadyExist";
	else if( code == ResultNoSuchInstance ) return "NoSuchInstance";
	else if( code == ResultOutOfRange ) return "ResultOutOfRange";
	else if( code == ResultNullArg ) return "NullArg";
	else if( code == ResultSocketError ) return "SocketError";
	else if( code == ResultBindError ) return "BindError";
	else if( code == ResultIoctlFailure ) return "IoctlFailure";
	else if( code == ResultPthreadFailure ) return "PthreadFailure";
	else if( code == ResultMmapFailure ) return "MmapFailure";
	else if( code == ResultMunmapFailure ) return "MunmapFailure";
	else if( code == ResultMsgIfDelimError ) return "MsgIfDelimError";
	else if( code == ResultMsgIfSendError ) return "MsgIfSendError";
	else if( code == ResultMsgIfMsgHandleNYI ) return "MsgIfMsgHandleNYI";
	else if( code == ResultMsgIfMsgHandleError ) return "MsgIfMsgHandle";
	else if( code == ResultMallocError ) return "MallocError";
	else return "Unknown";
}

EnumResultCode 
bsl_makeHeader( T_Protocol* proto )
{
	BSL_CHECK_NULL( proto, ResultNullArg );

	if( proto->l2tag.protocol == ProtocolISL ) {
		if( makeHeader_f_ptr[proto->l2tag.protocol] ) 
			makeHeader_f_ptr[proto->l2tag.protocol]( proto );
	}

	if( makeHeader_f_ptr[proto->l2.protocol] ) 
		makeHeader_f_ptr[proto->l2.protocol]( proto );
	
	if( ( proto->l2tag.protocol != ProtocolISL ) && 
			( proto->l2tag.protocol != ProtocolUnused ) ) {
		if( makeHeader_f_ptr[proto->l2tag.protocol] ) 
			makeHeader_f_ptr[proto->l2tag.protocol]( proto );
	}

	if( makeHeader_f_ptr[proto->l3.protocol] ) 
		makeHeader_f_ptr[proto->l3.protocol]( proto );
	
	if( makeHeader_f_ptr[proto->l4.protocol] ) 
		makeHeader_f_ptr[proto->l4.protocol]( proto );
	
	bsl_swap64( proto->header, proto->headerLength ); 

	do {
		unsigned char* ptr = (unsigned char*)proto->header;
		int i;
		BSL_INFO(("HEADER DUMP (headerlen %d) ----------------------- \n", proto->headerLength));

		for( i=0; i<proto->headerLength; i+=8 ) {
			if( i%16 == 0 ) printf("\n[%04d] ", i );
			BSL_INFO(("%016llX ", *(unsigned long long*)(ptr+i) ));
		}
		printf("\n");
	} while(0);

	return ResultSuccess; 
}

static void makeHeaderEthernet( T_Protocol* proto )
{
	T_PDR_Ethernet* pdr = NULL; 
	void* hp = NULL;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_Ethernet*)proto->l2.pdr;
	hp = (void*)proto->header + proto->headerLength;

	proto->l2.offset = proto->headerLength;

	//dest mac
	memcpy( hp, pdr->dest.addr, SIZE_ETHER_ADDR );
	hp += SIZE_ETHER_ADDR;
	//src mac
	memcpy( hp, pdr->src.addr, SIZE_ETHER_ADDR );
	hp += SIZE_ETHER_ADDR;
// 2017.4.4 by dgjung
	if ( pdr->type >> 16 ) {
		proto->headerLength += ( LENGTH_HEADER_BASE_ETHERNET - SIZE_ETHER_TYPE );
	}
	else {
		//ether type
		*(unsigned short*)hp = ntohs(pdr->type);
		hp += SIZE_ETHER_TYPE;

		proto->headerLength += LENGTH_HEADER_BASE_ETHERNET;
	}
}

static void makeHeaderVLAN( T_Protocol* proto )
{
	T_PDR_802_1Q_VLAN* pdr = NULL; 
	T_802_1Q_VLANTag* hp = NULL;
	unsigned short etype = 0; //for tagging 

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_802_1Q_VLAN*)proto->l2tag.pdr;
	hp = (T_802_1Q_VLANTag*)((void*)proto->header + proto->headerLength - SIZE_ETHER_TYPE);
	etype = *(unsigned short*)hp;

	proto->l2tag.offset = proto->headerLength - SIZE_ETHER_TYPE;

	hp->priority = pdr->vlan.priority & 0x7;
	hp->cfi = pdr->vlan.cfi & 0x1;
	hp->vlanid = pdr->vlan.vlanid & 0xFFF;
	hp->tpid = (unsigned short)pdr->vlan.tagProtocolId; 

	proto->headerLength += sizeof(T_802_1Q_VLANTag);

	//For endian change
	hp->tpid = ntohs( hp->tpid );
	*(unsigned short*)(&hp->tpid+1) = ntohs((((unsigned short)hp->priority) << 13) | 
			( (unsigned short)hp->cfi << 12 ) | hp->vlanid );

	*(unsigned short*)(hp+1) = etype; //tagging

	if( pdr->mode == VlanModeSingle ) return;

	//VLAN Stacking
	hp++;
	hp->priority = pdr->vlanDup->priority & 0x7;
	hp->cfi = pdr->vlanDup->cfi & 0x1;
	hp->vlanid = pdr->vlanDup->vlanid & 0xFFF;
	hp->tpid = (unsigned short)pdr->vlanDup->tagProtocolId; 

	proto->headerLength += sizeof(T_802_1Q_VLANTag);
	
	//For endian change
	hp->tpid = ntohs( hp->tpid );
	*(unsigned short*)(&hp->tpid+1) = ntohs((((unsigned short)hp->priority) << 13) | 
			( (unsigned short)hp->cfi << 12 ) | hp->vlanid );

	*(unsigned short*)(hp+1) = etype; //tagging
}

static void makeHeaderMPLS( T_Protocol* proto )
{
	T_PDR_MPLS* pdr = NULL; 
	T_Mpls* hp = NULL;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_MPLS*)proto->l2tag.pdr;
	hp = (T_Mpls*)((void*)proto->header + proto->headerLength );

	hp->label = pdr->mpls.label & 0x000FFFFF; //20bits
	hp->exp = pdr->mpls.exp & 0x07; //3bits
	hp->s = pdr->mpls.s & 0x01;     //1bit
	hp->ttl = pdr->mpls.ttl & 0xFF; //8bits

	proto->headerLength += sizeof(T_Mpls);

	//For endian change
	*(unsigned int*)hp = ntohl( (hp->label << 12) | (hp->exp << 9) | (hp->s << 8) | hp->ttl );

	if( pdr->type == MplsTypeUnicast ) return;

	hp++;
	hp->label = pdr->mplsDup->label & 0x000FFFFF; //20bits
	hp->exp = pdr->mplsDup->exp & 0x07; //3bits
	hp->s = pdr->mplsDup->s & 0x01;     //1bit
	hp->ttl = pdr->mplsDup->ttl & 0xFF; //8bits

	proto->headerLength += sizeof(T_Mpls);

	//For endian change for additional Label
	*(unsigned int*)hp = ntohl( (hp->label << 12) | (hp->exp << 9) | (hp->s << 8) | hp->ttl );
}


static void makeHeaderIp4FromPdr( 
		T_Protocol* proto, 
		T_PDR_Ip4* pdr, 
		unsigned int l3length )
{
	T_Ip4* hp = NULL;

	hp = (T_Ip4*)(proto->header + proto->headerLength);

	hp->version = pdr->version & 0x0F;
	hp->hlen = pdr->hlen & 0x0F;
	hp->tos = pdr->tos;
	hp->tlen = pdr->tlen;
	hp->id = pdr->id.value;
	hp->flags = pdr->flags & 0x07;
	hp->fragoffset = pdr->fragoffset & 0x1FFF;
	hp->ttl = pdr->ttl;
	hp->proto = pdr->proto;
	hp->checksum = pdr->checksum.value; 
	hp->sip = pdr->sip.addr;
	hp->dip = pdr->dip.addr;

	//option
	if( pdr->hlen > 0x0F ) {
		fprintf( stderr, "%s: Error. pdr->hlen %d\n", __func__, pdr->hlen );
		pdr->hlen = 5; //for header length
	}
	else if( pdr->hlen > 0x05 ) {
		T_PDR_Ip4Options* pdr_op = (T_PDR_Ip4Options* )( pdr+1 );
		
		if( pdr_op->length != ( pdr->hlen - 0x05 ) * 4 ) //check sanity
			fprintf( stderr, "%s: Error. pdr->hlen %d\n", __func__, pdr->hlen );
		else {
			unsigned int* op_pval = pdr_op->pval;
			for( int len=0; len<pdr_op->length; len+=4 ) {
				*op_pval = ntohl( *op_pval );
				op_pval++;
			}
			memcpy( (void*)(hp+1), pdr_op->pval, pdr_op->length );
		}
	}

	proto->headerLength += ( ( pdr->hlen ) * 4 );

	//For endian change
	*(unsigned char*)hp = ( (unsigned char)(hp->version)<<4 ) | ( (unsigned char)(hp->hlen) << 0 );
	hp->tlen = ntohs( hp->tlen );
	hp->id = ntohs( hp->id );
	*(unsigned short*)((&hp->id)+1) = ntohs((((unsigned short)hp->flags) << 13) | hp->fragoffset );
	hp->checksum = ntohs( hp->checksum );
	hp->sip = ntohl( hp->sip );
	hp->dip = ntohl( hp->dip );
}


static void makeHeaderIp4( T_Protocol* proto )
{
	T_PDR_Ip4* pdr = NULL; 

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_Ip4*)proto->l3.pdr;
	proto->l3.offset = proto->headerLength;
	makeHeaderIp4FromPdr( proto, pdr, proto->l3.length );
}

static void makeHeaderIp6FromPdr( 
		T_Protocol* proto, 
		T_PDR_Ip6* pdr, 
		unsigned int l3length )
{
	T_Ip6* hp = NULL;
	T_PDR_Ip6NextHeader* nhpdr = NULL;
	void* nhp = NULL;

	hp = (T_Ip6*)(proto->header + proto->headerLength);

	hp->version = pdr->version & 0x0F;
	hp->tclass = pdr->tclass & 0xFF;
	hp->flabel = pdr->flabel;
	hp->plen = pdr->plen;
	hp->nextheader = pdr->nextheader;
	hp->hoplimit = pdr->hoplimit;

	memcpy( hp->sip, pdr->sip.addr, 16 );
	memcpy( hp->dip, pdr->dip.addr, 16 );

	proto->headerLength += sizeof( T_Ip6 );

	if( pdr->nextheader != IP6_NO_NEXT_HEADER ) {
		int length = sizeof( T_PDR_Ip6 );
		nhpdr = (T_PDR_Ip6NextHeader*)(pdr+1);
		nhp = (void*)(hp+1);

		while( l3length > length ) {
			memcpy( nhp, nhpdr->pval, nhpdr->length );
			bsl_swap32( nhp, nhpdr->length );
			proto->headerLength += nhpdr->length;

			length += nhpdr->length + sizeof(nhpdr->length);
			nhpdr = ((void*)nhpdr) + nhpdr->length + sizeof(nhpdr->length);
			nhp += nhpdr->length;
		}
	}

	//For endian change
	*(unsigned int*)hp = ntohl(( (unsigned int)hp->version<<28 ) | 
			( (unsigned int)hp->tclass<<20 ) | (unsigned int)hp->flabel );
	hp->plen = ntohs( hp->plen );
}

static void makeHeaderIp6( T_Protocol* proto ) 
{
	T_PDR_Ip6* pdr = NULL; 

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_Ip6*)proto->l3.pdr;
	proto->l3.offset = proto->headerLength;
	makeHeaderIp6FromPdr( proto, pdr, proto->l3.length );
}

	
static void makeHeaderIp4OverIp6( T_Protocol* proto ) 
{
	T_PDR* ip6pdr_msgp = NULL;
	T_PDR* ip4pdr_msgp = NULL;

	T_PDR_Ip4* ip4pdr = NULL;
	T_PDR_Ip6* ip6pdr = NULL;

	unsigned int ip4pdrlen = 0;
	unsigned int ip6pdrlen = 0;

	BSL_CHECK_NULL( proto, );

	ip6pdr_msgp = (T_PDR*)(proto->l3.pdr);
	ip6pdrlen = ip6pdr_msgp->length;
	ip6pdr = (T_PDR_Ip6*)ip6pdr_msgp->pinfo;
	makeHeaderIp6FromPdr( proto, ip6pdr, ip6pdrlen );

	ip4pdr_msgp = (T_PDR*)( ((void*)ip6pdr) + ip6pdrlen );
	ip4pdrlen = ((T_PDR*)ip4pdr_msgp)->length;
	ip4pdr = (T_PDR_Ip4*)ip4pdr_msgp->pinfo;
	makeHeaderIp4FromPdr( proto, ip4pdr, ip4pdrlen );
}

static void makeHeaderIp6OverIp4( T_Protocol* proto )
{
	T_PDR* ip6pdr_msgp = NULL;
	T_PDR* ip4pdr_msgp = NULL;

	T_PDR_Ip4* ip4pdr = NULL;
	T_PDR_Ip6* ip6pdr = NULL;

	unsigned int ip4pdrlen = 0;
	unsigned int ip6pdrlen = 0;

	BSL_CHECK_NULL( proto, );
	proto->l3.offset = proto->headerLength;

	ip4pdr_msgp = (T_PDR*)proto->l3.pdr;
	ip4pdrlen = ((T_PDR*)ip4pdr_msgp)->length;
	ip4pdr = (T_PDR_Ip4*)ip4pdr_msgp->pinfo;
	makeHeaderIp4FromPdr( proto, ip4pdr, ip4pdrlen );

	ip6pdr_msgp = (T_PDR*)( ((void*)ip4pdr) + ip4pdrlen );
	ip6pdrlen = ((T_PDR*)ip6pdr_msgp)->length;
	ip6pdr = (T_PDR_Ip6*)ip6pdr_msgp->pinfo;
	makeHeaderIp6FromPdr( proto, ip6pdr, ip6pdrlen );
}

static void makeHeaderArp( T_Protocol* proto )
{
	T_Arp* hp = NULL;
	T_PDR_ARP* pdr = NULL;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_ARP*)proto->l3.pdr;
	proto->l3.offset = proto->headerLength;
	hp = (T_Arp*)(proto->header + proto->headerLength);

	hp->hwtype = pdr->hwtype;
	hp->protocoltype = pdr->protocoltype;
	hp->halen = pdr->halength;
	hp->palen = pdr->palength;
	hp->operation = pdr->operation;
	memcpy( hp->senderMac, pdr->senderMac.addr, SIZE_ETHER_ADDR );
	hp->senderIp = pdr->senderIp.addr;
	memcpy( hp->targetMac, pdr->targetMac.addr, SIZE_ETHER_ADDR );
	hp->targetIp = pdr->targetIp.addr;

	proto->headerLength += sizeof( T_Arp );

	//For endian change
	hp->hwtype = ntohs( hp->hwtype );
	hp->protocoltype = ntohs( hp->protocoltype );
	hp->operation = ntohs( hp->operation );
	hp->senderIp = ntohl( hp->senderIp );
	hp->targetIp = ntohl( hp->targetIp );
}

static void makeHeaderTcp( T_Protocol* proto )
{
	T_Tcp* hp = NULL;
	T_PDR_TCP* pdr = NULL;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_TCP*)proto->l4.pdr;
	proto->l4.offset = proto->headerLength;
	hp = (T_Tcp*)(proto->header + proto->headerLength);

	hp->sport = pdr->sport.value;
	hp->dport = pdr->dport.value;
	hp->seqnum = pdr->seqnum;
	hp->acknum = pdr->acknum;
	hp->offset = pdr->offset;
	hp->flag_res = pdr->flag_res;
	hp->flag_urg = pdr->flag_urg ? 1 : 0;
	hp->flag_ack = pdr->flag_ack ? 1 : 0;
	hp->flag_psh = pdr->flag_psh ? 1 : 0;
	hp->flag_rst = pdr->flag_rst ? 1 : 0;
	hp->flag_syn = pdr->flag_syn ? 1 : 0;
	hp->flag_fin = pdr->flag_fin ? 1 : 0;
	hp->windows = pdr->windows;
	hp->checksum = pdr->checksum.value;
	hp->urgent = pdr->urgent;

	proto->headerLength += hp->offset * 4;

	//For endian change
	hp->sport = ntohs( hp->sport );
	hp->dport = ntohs( hp->dport );
	hp->seqnum = ntohl( hp->seqnum );
	hp->acknum = ntohl( hp->acknum );
	*(unsigned short*)(&hp->acknum + 1 ) = ntohs( 
		( hp->offset << 12 ) | ( hp->flag_res << 6 ) | ( hp->flag_urg << 5 ) | 
		( hp->flag_ack << 4 ) | ( hp->flag_psh << 3 ) | ( hp->flag_rst << 2 ) | 
		( hp->flag_syn << 1 ) | ( hp->flag_fin << 0 ) );
	hp->windows = ntohs( hp->windows );
	hp->checksum = ntohs( hp->checksum );
	hp->urgent = ntohs( hp->urgent );

	T_PDR_TcpOptions* pdr_op = (T_PDR_TcpOptions* )( &pdr->optionLength );
		
	unsigned int* op_pval = pdr_op->pval;
	for(int len=0; len<pdr_op->length; len+=4 ) {
		*op_pval = ntohl( *op_pval );
		op_pval++;
	}
	memcpy( (void*)(hp+1), pdr_op->pval, pdr_op->length );
}

static void makeHeaderUdp( T_Protocol* proto )
{
	T_Udp* hp = NULL;
	T_PDR_UDP* pdr = NULL;

	BSL_CHECK_NULL( proto, );
	pdr = (T_PDR_UDP*)proto->l4.pdr;

	proto->l4.offset = proto->headerLength;
	hp = (T_Udp*)(proto->header + proto->headerLength);

	hp->sport = pdr->sport.value;
	hp->dport = pdr->dport.value;
	hp->length = pdr->val; //TODO: check length
	hp->checksum = pdr->checksum.value;

	proto->headerLength += sizeof( T_Udp );

	//For endian change
	hp->sport = ntohs( hp->sport );
	hp->dport = ntohs( hp->dport );
	hp->length = ntohs( hp->length );
	hp->checksum = ntohs( hp->checksum );
}

static void makeHeaderIcmp( T_Protocol* proto )
{
	T_Icmp* hp = NULL;
	T_PDR_ICMP* pdr = NULL;
	bool valid_data12 = false;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_ICMP*)proto->l4.pdr;
	proto->l4.offset = proto->headerLength;
	hp = (T_Icmp*)(proto->header + proto->headerLength);

	hp->type = pdr->type;
	hp->code = pdr->code;
	hp->checksum = pdr->checksum.value;
	hp->data1 = pdr->data1;
	hp->data2 = pdr->data2;

	valid_data12 = //0, 8, 13, 14, 15, 16, 17, 18
		((hp->type == 0) || (hp->type == 8) || 
		((hp->type >= 13) && (hp->type <= 18))) ? true : false;

	proto->headerLength += sizeof( T_Icmp ) - 
		(valid_data12 ? 0 : sizeof(hp->data1) + sizeof(hp->data2));

	//For endian change
	hp->checksum = ntohs( hp->checksum );
	hp->data1 = ntohs( hp->data1 );
	hp->data2 = ntohs( hp->data2 );
}

static void makeHeaderIgmpv2( T_Protocol* proto )
{
	T_Igmpv2* hp = NULL;
	T_PDR_IGMPv2* pdr = NULL;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_IGMPv2*)proto->l4.pdr;
	proto->l4.offset = proto->headerLength;
	hp = (T_Igmpv2*)(proto->header + proto->headerLength);

	hp->ver = pdr->ver & 0x0F;
	hp->type = pdr->type & 0x0F;
	hp->maxRespTime = pdr->maxRespTime;
	hp->checksum = pdr->checksum.value;
	hp->group = pdr->group.addr;

	proto->headerLength += sizeof( T_Igmpv2 );

	//For endian change
	*(unsigned char*)hp = ( (unsigned char)(hp->ver)<<4 ) | ( (unsigned char)(hp->type) << 0 );
	hp->checksum = ntohs( hp->checksum );
	hp->group = ntohl( hp->group );
}

static void makeHeaderUDF( T_Protocol* proto )
{
	//NOT YET IMPLEMENTED
}
static void makeHeaderISL( T_Protocol* proto )
{
	T_Isl* hp = NULL;
	T_PDR_ISL* pdr = NULL;

	BSL_CHECK_NULL( proto, );

	pdr = (T_PDR_ISL*)proto->l2tag.pdr;
	proto->l2tag.offset = 0;
	hp = (T_Isl*)(proto->header + proto->headerLength);

	memcpy( hp->dest, pdr->dest, SIZE_ISL_ADDR );
	hp->type = pdr->type;
	hp->user = pdr->user;
	memcpy( hp->src, pdr->src, SIZE_ETHER_ADDR );
	hp->length = pdr->length; 
	hp->snap[0] = 0xAA; hp->snap[1] = 0xAA; hp->snap[2] = 0x03; 
	memcpy( hp->hsa, hp->src, SIZE_ISL_HSA );
	hp->vlanid = pdr->vlanid & 0x7FFF;
	hp->bpduInd = pdr->bpduInd & 0x1;
	hp->index = pdr->index;
	hp->reserved = pdr->reserved;

	proto->headerLength += sizeof( T_Isl );

	//For endian change
	*(unsigned char*)(hp->dest+SIZE_ISL_ADDR) = 
		( (unsigned char)(hp->type)<<4 ) | ( (unsigned char)(hp->user) << 0 );
	hp->length = ntohs( hp->length );
	*(unsigned short*)(hp->hsa+SIZE_ISL_HSA) = ntohs( 
		( (unsigned short)(hp->vlanid)<<1 ) | ( (unsigned short)(hp->bpduInd) << 0 ) );
	hp->index = ntohs( hp->index );
	hp->reserved = ntohs( hp->reserved );
}

// ****************************************
// makeupPdr functions for 40G
// ****************************************
EnumResultCode 
bsl_makeupPdr(int portid, T_Protocol* proto)
{
	BSL_CHECK_NULL( proto, ResultNullArg );

	if(makeupPdr_f_ptr[proto->l2.protocol])
		makeupPdr_f_ptr[proto->l2.protocol](portid, proto);

	if(makeupPdr_f_ptr[proto->l2tag.protocol]) 
		makeupPdr_f_ptr[proto->l2tag.protocol](portid, proto);
	
	if(makeupPdr_f_ptr[proto->l3.protocol]) 
		makeupPdr_f_ptr[proto->l3.protocol](portid, proto);
	
	if(makeupPdr_f_ptr[proto->l4.protocol]) 
		makeupPdr_f_ptr[proto->l4.protocol](portid, proto);
	
	return ResultSuccess; 
}

static void get_firstmac( 
		int portid,
		T_EtherAddrTuple* tuple)
{	
	bool israndom = tuple->mode == EtherAddrModeRandom ? true : false;

	if(israndom) {
		if(portid == 0) {
			unsigned int seed = htonl(*(unsigned int*)tuple->addr);
			srand(seed);
		}
		else {
			*(unsigned int*)tuple->addr = rand();
			*(unsigned short*)(tuple->addr+4) = rand();
		}

		return;
	}

	//reuse get_lastmac
	T_EtherAddrTuple stuple;
	memcpy(&stuple, tuple, sizeof(stuple));

	stuple.repeatCount = tuple->repeatCount < MAX_NPORTS ?
		portid % tuple->repeatCount :
		portid % MAX_NPORTS;

	bool isdecre = 
		((tuple->mode == EtherAddrModeDecrement) ||
		(tuple->mode == EtherAddrModeContDecrement)) ? true : false;

	stuple.mode = 
		isdecre ? EtherAddrModeDecrement : EtherAddrModeIncrement;

	get_lastmac(&stuple, tuple->addr);
}


static void makeupTupleEtherAddr(int portid, T_EtherAddrTuple* tuple)
{
	if( (tuple->mode == EtherAddrModeIncrement) ||
		(tuple->mode == EtherAddrModeContIncrement) ||
		(tuple->mode == EtherAddrModeDecrement) ||
		(tuple->mode == EtherAddrModeContDecrement) ||
		(tuple->mode == EtherAddrModeRandom)) 
		;
	else  //no change
		return; 

	//1. addr
	get_firstmac(portid, tuple);

	if(tuple->mode == EtherAddrModeRandom) return;

	//2. step
	tuple->step *= MAX_NPORTS;

	//3. count
	tuple->mode = tuple->repeatCount > MAX_NPORTS ? tuple->mode : 
		tuple->repeatCount == 3 ? tuple->mode :
		EtherAddrModeFixed; // in case of 1,2,4
}

static void get_firstCustomInteger(
		int portid, 
		T_CustomIntegerTuple* tuple)
{
	bool israndom = tuple->mode == CustomIntegerModeRandom ? true : false;

	if(israndom) {
		if(portid == 0) {
			srand(tuple->value);
		}
		else {
			tuple->value = rand() % (1 << 16);
		}
		return;
	}

	bool isdecre = 
		(tuple->mode == CustomIntegerModeDecrement) ||
		(tuple->mode == CustomIntegerModeContDecrement) ? true : false; 

	int step = isdecre ? -tuple->step : tuple->step;

	tuple->value += 
		step * 
		(portid % (tuple->repeat < MAX_NPORTS ? tuple->repeat : MAX_NPORTS));
}

static void makeupTupleCustomInteger(
		int portid, 
		T_CustomIntegerTuple* tuple) 
{
	if( (tuple->mode == CustomIntegerModeIncrement) ||
		(tuple->mode == CustomIntegerModeContIncrement) ||
		(tuple->mode == CustomIntegerModeDecrement) ||
		(tuple->mode == CustomIntegerModeContDecrement) ||
		(tuple->mode == CustomIntegerModeRandom)) 
		;
	else  //no change
		return; 

	//1. start
	get_firstCustomInteger(portid, tuple);

	if(tuple->mode == CustomIntegerModeRandom) return;

	//2. step
	tuple->step *= MAX_NPORTS;

	//3. mode
	tuple->mode = tuple->repeat > MAX_NPORTS ? tuple->mode : 
		tuple->repeat == 3 ? tuple->mode :
		CustomIntegerModeFixed; // in case of 1,2,4

}

static void get_firstip4( 
		int portid,
		T_Ip4AddrTuple* tuple)
{	
	bool israndom = tuple->mode == IpAddrModeRandom ? true : false;

	if(israndom) {
		if(portid == 0) {
			srand(tuple->addr);
		}
		else {
			tuple->addr = rand();
		}

		return;
	}

	//reuse get_lastip4
	T_Ip4AddrTuple stuple;
	memcpy(&stuple, tuple, sizeof(stuple));

	stuple.repeat = tuple->repeat < MAX_NPORTS ?
		portid % tuple->repeat :
		portid % MAX_NPORTS;

	bool ishost = 
		((tuple->mode == IpAddrModeIncrementHost) ||
		(tuple->mode == IpAddrModeDecrementHost) ||
		(tuple->mode == IpAddrModeContIncrementHost) ||
		(tuple->mode == IpAddrModeContDecrementHost)) ? true : false;

	bool isdecre = 
		((tuple->mode == IpAddrModeDecrementHost) ||
		(tuple->mode == IpAddrModeContDecrementHost) ||
		(tuple->mode == IpAddrModeDecrementNetwork) ||
		(tuple->mode == IpAddrModeContDecrementNetwork)) ? true : false;

	stuple.mode = 
		isdecre && ishost ? IpAddrModeDecrementHost :
		isdecre && (!ishost) ? IpAddrModeDecrementNetwork :
		(!isdecre) && ishost ? IpAddrModeIncrementHost :
		(!isdecre) && (!ishost) ? IpAddrModeIncrementNetwork : IpAddrModeUnknown;

	tuple->addr = get_lastip4(&stuple);
}

static void makeupTupleIp4Addr(
		int portid, 
		T_Ip4AddrTuple* tuple)
{
	if( (tuple->mode == IpAddrModeIncrementHost) ||
		(tuple->mode == IpAddrModeDecrementHost) ||
		(tuple->mode == IpAddrModeContIncrementHost) ||
		(tuple->mode == IpAddrModeContDecrementHost) ||
	    (tuple->mode == IpAddrModeIncrementNetwork) ||
		(tuple->mode == IpAddrModeDecrementNetwork) ||
		(tuple->mode == IpAddrModeContIncrementNetwork) ||
		(tuple->mode == IpAddrModeContDecrementNetwork) ||
		(tuple->mode == IpAddrModeRandom)) 
		;
	else  //no change
		return; 

	get_firstip4(portid, tuple);

	if(tuple->mode == IpAddrModeRandom) return;

	tuple->mode = tuple->repeat > MAX_NPORTS ? tuple->mode : 
		tuple->repeat == 3 ? tuple->mode :
		IpAddrModeFixed; // in case of 1,2,4
}

static void get_firstip6(
		int portid,
		T_Ip6AddrTuple* tuple)
{
	//Don't support IpAddrModeRandom

	//reuse get_lastip6
	T_Ip6AddrTuple stuple;
	memcpy(&stuple, tuple, sizeof(stuple));

	stuple.repeat = tuple->repeat < MAX_NPORTS ?
		portid % tuple->repeat :
		portid % MAX_NPORTS;

	bool ishost = 
		((tuple->mode == IpAddrModeIncrementHost) ||
		(tuple->mode == IpAddrModeDecrementHost) ||
		(tuple->mode == IpAddrModeContIncrementHost) ||
		(tuple->mode == IpAddrModeContDecrementHost)) ? true : false;

	bool isdecre = 
		((tuple->mode == IpAddrModeDecrementHost) ||
		(tuple->mode == IpAddrModeContDecrementHost) ||
		(tuple->mode == IpAddrModeDecrementNetwork) ||
		(tuple->mode == IpAddrModeContDecrementNetwork)) ? true : false;

	stuple.mode = 
		isdecre && ishost ? IpAddrModeDecrementHost :
		isdecre && (!ishost) ? IpAddrModeDecrementNetwork :
		(!isdecre) && ishost ? IpAddrModeIncrementHost :
		(!isdecre) && (!ishost) ? IpAddrModeIncrementNetwork : IpAddrModeUnknown;

	get_lastip6(&stuple, tuple->addr);
}

static void makeupTupleIp6Addr(
		int portid, 
		T_Ip6AddrTuple* tuple)
{
	if( (tuple->mode == IpAddrModeIncrementHost) ||
		(tuple->mode == IpAddrModeDecrementHost) ||
		(tuple->mode == IpAddrModeContIncrementHost) ||
		(tuple->mode == IpAddrModeContDecrementHost) ||
	    (tuple->mode == IpAddrModeIncrementNetwork) ||
		(tuple->mode == IpAddrModeDecrementNetwork) ||
		(tuple->mode == IpAddrModeContIncrementNetwork) ||
		(tuple->mode == IpAddrModeContDecrementNetwork))
		;
	else  //no change
		return; 

	//1. addr
	get_firstip6(portid, tuple);

	//2. step
	tuple->step *= MAX_NPORTS;

	//3. mode
	tuple->mode = tuple->repeat > MAX_NPORTS ? tuple->mode : 
		tuple->repeat == 3 ? tuple->mode :
		IpAddrModeFixed; // in case of 1,2,4
}


static void makeupPdrEthernet(int portid, T_Protocol* proto)
{
	T_PDR_Ethernet* pdr = (T_PDR_Ethernet*)proto->l2.pdr;

	makeupTupleEtherAddr(portid, &pdr->dest);
	makeupTupleEtherAddr(portid, &pdr->src);
}

static void makeupPdrVLAN(int portid, T_Protocol* proto )
{
	//Nothing to do.
}

static void makeupPdrMPLS(int portid, T_Protocol* proto )
{
	//Nothing to do.
}

static void makeupPdrIp4(int portid, T_Protocol* proto)
{
	T_PDR_Ip4* pdr = (T_PDR_Ip4*)proto->l3.pdr;

	makeupTupleCustomInteger(portid, &pdr->id);
	makeupTupleIp4Addr(portid, &pdr->sip);
	makeupTupleIp4Addr(portid, &pdr->dip);
}

static void makeupPdrIp6(int portid, T_Protocol* proto) 
{
	T_PDR_Ip6* pdr = (T_PDR_Ip6*)proto->l3.pdr;

	makeupTupleIp6Addr(portid, &pdr->sip);
	makeupTupleIp6Addr(portid, &pdr->dip);
}

static void makeupPdrIp4OverIp6(int portid, T_Protocol* proto ) 
{
	T_PDR* ip6pdr_msgp = NULL;
	T_PDR* ip4pdr_msgp = NULL;

	T_PDR_Ip4* ip4pdr = NULL;
	T_PDR_Ip6* ip6pdr = NULL;

	unsigned int ip6pdrlen = 0;

	BSL_CHECK_NULL( proto, );

	ip6pdr_msgp = (T_PDR*)(proto->l3.pdr);
	ip6pdrlen = ip6pdr_msgp->length;
	ip6pdr = (T_PDR_Ip6*)ip6pdr_msgp->pinfo;

	makeupTupleIp6Addr(portid, &ip6pdr->sip);
	makeupTupleIp6Addr(portid, &ip6pdr->dip);

	ip4pdr_msgp = (T_PDR*)( ((void*)ip6pdr) + ip6pdrlen );
	ip4pdr = (T_PDR_Ip4*)ip4pdr_msgp->pinfo;

	makeupTupleCustomInteger(portid, &ip4pdr->id);
	makeupTupleIp4Addr(portid, &ip4pdr->sip);
	makeupTupleIp4Addr(portid, &ip4pdr->dip);
}

static void makeupPdrIp6OverIp4(int portid, T_Protocol* proto)
{
	T_PDR* ip6pdr_msgp = NULL;
	T_PDR* ip4pdr_msgp = NULL;

	T_PDR_Ip4* ip4pdr = NULL;
	T_PDR_Ip6* ip6pdr = NULL;

	unsigned int ip4pdrlen = 0;

	BSL_CHECK_NULL( proto, );

	ip4pdr_msgp = (T_PDR*)proto->l3.pdr;
	ip4pdrlen = ((T_PDR*)ip4pdr_msgp)->length;
	ip4pdr = (T_PDR_Ip4*)ip4pdr_msgp->pinfo;

	makeupTupleCustomInteger(portid, &ip4pdr->id);
	makeupTupleIp4Addr(portid, &ip4pdr->sip);
	makeupTupleIp4Addr(portid, &ip4pdr->dip);

	ip6pdr_msgp = (T_PDR*)( ((void*)ip4pdr) + ip4pdrlen );
	ip6pdr = (T_PDR_Ip6*)ip6pdr_msgp->pinfo;

	makeupTupleIp6Addr(portid, &ip6pdr->sip);
	makeupTupleIp6Addr(portid, &ip6pdr->dip);
}

static void makeupPdrArp(int portid, T_Protocol* proto )
{
	T_PDR_ARP* pdr = (T_PDR_ARP*)proto->l3.pdr;

	makeupTupleEtherAddr(portid, &pdr->senderMac);
	makeupTupleIp4Addr(portid, &pdr->senderIp);
	makeupTupleEtherAddr(portid, &pdr->targetMac);
	makeupTupleIp4Addr(portid, &pdr->targetIp);
}

static void makeupPdrTcp(int portid, T_Protocol* proto )
{
	T_PDR_TCP* pdr = (T_PDR_TCP*)proto->l4.pdr;

	makeupTupleCustomInteger(portid, &pdr->sport);
	makeupTupleCustomInteger(portid, &pdr->dport);
}

static void makeupPdrUdp(int portid, T_Protocol* proto )
{
	T_PDR_UDP* pdr = (T_PDR_UDP*)proto->l4.pdr;

	makeupTupleCustomInteger(portid, &pdr->sport);
	makeupTupleCustomInteger(portid, &pdr->dport);
}

static void makeupPdrIcmp(int portid, T_Protocol* proto )
{
	//Nothing to do.
}

static void makeupPdrIgmpv2(int portid, T_Protocol* proto )
{
	T_PDR_IGMPv2* pdr = (T_PDR_IGMPv2*)proto->l4.pdr;

	makeupTupleIp4Addr(portid, &pdr->group);
}

static void makeupPdrUDF(int portid, T_Protocol* proto )
{
	//NOT YET IMPLEMENTED
}
static void makeupPdrISL(int portid, T_Protocol* proto )
{
	//Nothing to do.
}

// ****************************************
// makeupFrame functions for 40G
// ****************************************
EnumResultCode 
bsl_makeupFrame(int portid, T_Frame* frame)
{
	T_FrameSize* fsize = &frame->framesize;
	if(fsize->fsizeSpec == FrameSizeFixed) ;
	else if(fsize->fsizeSpec == FrameSizeRandom) {
		memcpy(fsize->fsizeValueRand, 
				&fsize->fsizeValueRand[portid*SIZE_NFRAME_RANDOM], 
				SIZE_NFRAME_RANDOM);
		memcpy(fsize->fsizeValueRandDiff, 
				&fsize->fsizeValueRandDiff[portid*SIZE_NFRAME_RANDOM], 
				SIZE_NFRAME_RANDOM);
	}
	else if(fsize->fsizeSpec == FrameSizeIncrement) {
		fsize->fsizeMin += portid * fsize->sizeOrStep;

		if(fsize->fsizeMin > fsize->fsizeMax) {
			fsize->fsizeMin = fsize->fsizeMax; //TODO: Check!!
		}
		fsize->sizeOrStep *= MAX_NPORTS;

		unsigned int newmax;
		unsigned int newstep = fsize->sizeOrStep;
		unsigned int newmin = fsize->fsizeMin;

		newmax = ((fsize->fsizeMax - newmin) / newstep) * newstep + newmin;
		fsize->fsizeMax = newmax;

		BSL_TRACE(("%s: (SizeIncrement)\n", __func__));
		BSL_TRACE(("\t[%d] Min %d Step %d Max %d\n", \
					portid, newmin, newstep, newmax));
	}
	return ResultSuccess;
}
	

EnumResultCode 
bsl_setRegister( int cardid, EnumCommandReg command, unsigned int addr, unsigned long long* value )
{
	int fd = 0;
	int ret = 0;

	BSL_CHECK_EXP( ((cardid<0)||(cardid>SIZE_MAX_CARDID)), ResultOutOfRange );

	fd = OPEN_DEVICE( cardid );
	BSL_CHECK_DEVICE( fd );

	ret = bsl_device_setRegister( fd, command, addr, value );
	close( fd );

	BSL_CHECK_RESULT( ret, ret );

	return ResultSuccess;
}

EnumResultCode
bsl_clearDmaBuffer(int cardid)
{
	int fd = 0;
    int ret = 0;

    fd = OPEN_DEVICE( cardid );
    BSL_CHECK_DEVICE( fd );

    ret = bsl_device_clearDmaBuffer(fd, cardid);
    close( fd );

    BSL_CHECK_RESULT( ret, ret );
    return ResultSuccess;
}


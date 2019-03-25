/********************************************************************
 *  FILE   : bsl_ext.h
 *  Author : joon
 *
 *  Extern functions
 *
 ********************************************************************/

#ifndef BSL_EXT_H
#define BSL_EXT_H

#include "bsl_type.h"
#include "bsl_proto.h"

extern EnumResultCode
bsl_getNumberOfCards(
		int* nCards),
bsl_getChassis( 
		T_Chassis* chassis),
bsl_getVersionInfo(
		int cardid, 
		T_Version* ver),
bsl_getLinkStatus(
		int cardid, 
		T_BslCard* card),
bsl_getLinkStats(
		int cardid, 
		int portid, 
		T_LinkStats* stat),
bsl_setPortMode(
		int cardid, 
		int portid, 
		EnumPortOpMode mode),
bsl_setPortActive(
		int cardid, 
		int portid, 
		EnumPortActive enable),
bsl_setLatency(
		int cardid, 
		int portid, 
		int latency_enable, 
		int sequence_enable, 
		int signature_enable),
bsl_setCaptureStartStop(
		int cardid, 
		int portid, 
		EnumCaptureMode mode, 
		unsigned int size, 
		int start),
bsl_setControlCommand(
		int portsel, 
		int cardid, 
		int portid, 
		int streamid, 
		EnumCommand command, 
		unsigned long long clocks, 
		unsigned int netmode, 
		unsigned long long mac, 
		unsigned int ip),
bsl_enableStream(
		int cardid, 
		int portid, 
		int streamid, 
		int enable),
bsl_setRegister(
		int cardid, 
		EnumCommandReg command, 
		unsigned int addr, 
		unsigned long long* value),
bsl_setStreamDetail(
		int cardid, 
		int portid, 
		int streamid, 
		int groupid,T_Stream* stream, 
		T_Protocol* proto),
bsl_clearDmaBuffer(
		int cardid);


extern int bsl_toPcap(
		char* filename, 
		int cardid, 
		int portid);

extern void toPacketTcp(
		T_Protocol* proto, 
		T_Tcp* hp);

extern EnumResultCode
bsl_device_setPortMode( 
		int fd, 
		int portid, 
		EnumPortOpMode mode),
bsl_device_getStats( 
		int fd, 
		T_LinkStats* stat),
bsl_device_getLinkStatus(
		int fd,
		int* linkstatus,
		int* nports), 
bsl_device_setLatency( 
		int fd, 
		int portid, 
		int latency, 
		int sequence, 
		int signature),
bsl_device_setRegister( 
		int fd, 
		EnumCommandReg command, 
		unsigned int addr, 
		unsigned long long* value),
bsl_device_setPortActive(
		int fd, 
		int portid, 
		EnumPortActive enable),
bsl_device_setCapture( 
		int fd, 
		int cardid, 
		int portid, 
		EnumCaptureMode mode, 
		unsigned int size, 
		int start),
bsl_device_setControlCommand(
		int fd, 
		int portsel, 
		int portid, 
		int streamid, 
		EnumCommand command, 
		unsigned long long clocks, 
		unsigned int, 
		unsigned long long, 
		unsigned int ),
bsl_device_setStreamDetail( 
		int fd, 
		int portid, 
		int streamid, 
		int groupid, 
		T_Stream* stream, 
		T_Protocol* proto ),                           
bsl_device_enableStream( 
		int fd, 
		int portid, 
		int streamid, 
		int enable ),
bsl_device_clearDmaBuffer(
		int fd, 
		int cardid);

extern void
get_lastmac( 
		T_EtherAddrTuple* tuple, 
		unsigned char* lastaddr),
get_lastip6( 
		T_Ip6AddrTuple* iptuple, 
		unsigned char* lastaddr);

extern unsigned int 
get_lastip4( 
		T_Ip4AddrTuple* iptuple);

extern unsigned short
getLastTcpUdpPort(
		T_CustomIntegerTuple* tuple);

extern void bsl_swap32(void* ptr, int length);
extern void bsl_swap64(void* ptr, int length);

extern void bsl_msgif_listen_loop( int socketport );

#endif //BSL_EXT_H

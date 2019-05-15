#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "bsl_type.h"
#include "bsl_dbg.h"
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

	//0. Prepare 
	T_Stream stream = {0,};
	T_Protocol proto = {0.};

	//1. Stream info
	stream.portid = 0;          //should be 0 for 40G
	stream.streamid = 5;
	stream.groupid = 0;
	stream.enable = 3;          //40G enable

	stream.control.control = StreamContinuousPacket; 
	stream.control.returnToId = 0;    
	stream.control.loopCount = 0;    
	stream.control.pktsPerBurst = 0;    
	stream.control.burstsPerStream = 0;    
	stream.control.rateControl = RateControlPercent;    
	stream.control.rateControlIntPart = 10;    
	stream.control.rateControlFracPart = 0;    
	stream.control.startTxDelay = 0;    
	stream.control.interBurstGapIntPart = 0;    
	stream.control.interBurstGapFracPart = 0;    
	stream.control.interStreamGapIntPart = 0;    
	stream.control.interStreamGapFracPart = 0;    
	stream.control.ifg = 0;

	//2. Protocol info
	proto.streamid = stream.streamid;

	//2.1 Ethernet as Layer 2
	proto.l2.protocol = ProtocolEthernet;  
	proto.l2.length = sizeof(T_PDR_Ethernet);
	proto.l2.pdr = malloc(proto.l2.length);
	BSL_CHECK_NULL(proto.l2.pdr, -1);

	T_PDR_Ethernet* pdr_ether = (T_PDR_Ethernet*)proto.l2.pdr;
	pdr_ether->dest.mode = EtherAddrModeFixed;
	pdr_ether->dest.addr[0] = 0x64;
	pdr_ether->dest.addr[1] = 0xE5;
	pdr_ether->dest.addr[2] = 0x99;
	pdr_ether->dest.addr[3] = 0x12;
	pdr_ether->dest.addr[4] = 0x3F;
	pdr_ether->dest.addr[5] = 0xD1;
	pdr_ether->dest.repeatCount = 0;
	pdr_ether->dest.step = 0;
	pdr_ether->src.mode = EtherAddrModeFixed;
	pdr_ether->src.addr[0] = 0xB8;
	pdr_ether->src.addr[1] = 0x27;
	pdr_ether->src.addr[2] = 0xEB;
	pdr_ether->src.addr[3] = 0xBE;
	pdr_ether->src.addr[4] = 0x9C;
	pdr_ether->src.addr[5] = 0xA6;
	pdr_ether->src.repeatCount = 0;
	pdr_ether->src.step = 0;
	pdr_ether->type = 0x0800;      //ether_type = IP

	//2.2 IP4 as Layer 3
	proto.l3.protocol = ProtocolIP4;  
	proto.l3.length = sizeof(T_PDR_Ip4);
	proto.l3.pdr = malloc(proto.l3.length);
	BSL_CHECK_NULL(proto.l3.pdr, -1);

	T_PDR_Ip4* pdr_ip4 = (T_PDR_Ip4*)proto.l3.pdr;
	pdr_ip4->version = 4;        
	pdr_ip4->hlen = 5;
	pdr_ip4->tos = 0x10;
	pdr_ip4->tlen = 0x84;
	pdr_ip4->id.mode = CustomIntegerModeFixed;
	pdr_ip4->id.value = 0x9CE8;
	pdr_ip4->id.step = 0;
	pdr_ip4->id.repeat = 0;
	pdr_ip4->flags = 2;           
	pdr_ip4->fragoffset = 0;
	pdr_ip4->ttl = 0x40;
	pdr_ip4->proto = 0x06;        
	pdr_ip4->checksum.type = ChecksumValid;
	pdr_ip4->checksum.value = 0;    //autofill if ChecksumValid
	pdr_ip4->sip.addr = 0xC0A80099; //192.168.0.99
	pdr_ip4->sip.mask = 0xFFFFFF00;
	pdr_ip4->sip.repeat = 0;
	pdr_ip4->sip.mode = IpAddrModeFixed;
	pdr_ip4->dip.addr = 0xC0A80008; //192.168.0.8
	pdr_ip4->dip.mask = 0xFFFFFF00;
	pdr_ip4->dip.repeat = 0;
	pdr_ip4->dip.mode = IpAddrModeFixed;

	//2.3 TCP as Layer 4
	proto.l4.protocol = ProtocolTCP;  
	proto.l4.length = sizeof(T_PDR_TCP);
	proto.l4.pdr = malloc(proto.l4.length);
	BSL_CHECK_NULL(proto.l4.pdr, -1);

	T_PDR_TCP* pdr_tcp = (T_PDR_TCP*)proto.l4.pdr;
	pdr_tcp->sport.mode = CustomIntegerModeFixed;
	pdr_tcp->sport.value = 22;
	pdr_tcp->sport.step = 0;
	pdr_tcp->sport.repeat = 0;
	pdr_tcp->dport.mode = CustomIntegerModeFixed;
	pdr_tcp->dport.value = 2690;
	pdr_tcp->dport.step = 0;
	pdr_tcp->dport.repeat = 0;
	pdr_tcp->seqnum = 0x2F5A88E4;
	pdr_tcp->acknum = 0x3175AcA0;
	pdr_tcp->offset = 5;            //TCP header length
	pdr_tcp->flag_res = 0;
	pdr_tcp->flag_urg = 0;
	pdr_tcp->flag_ack = 1;
	pdr_tcp->flag_psh = 1;
	pdr_tcp->flag_rst = 0;
	pdr_tcp->flag_syn = 0;
	pdr_tcp->windows = 310;
	pdr_tcp->checksum.type = ChecksumValid;
	pdr_tcp->checksum.value = 0;    //autofill if ChecksumValid
	pdr_tcp->urgent = 0;
	pdr_tcp->optionLength = 0;


	//3. Payload
	proto.fi.framesize.fsizeSpec = FrameSizeFixed;
	proto.fi.framesize.sizeOrStep = 146;      //Without FCS
	proto.fi.framesize.fsizeMin = 0;
	proto.fi.framesize.fsizeMax = 0;
	proto.fi.framesize.fsizeValueRand[0] = 0;
	proto.fi.framesize.fsizeValueRandDiff[0] = 0;
	proto.fi.crc = FrameCrcNoError;
	proto.fi.payloadType = FrameDataTypeIncByte;
	proto.fi.pattern.payloadOffset = 0;
	proto.fi.pattern.validSize = 4;                      //valid pattern size
	proto.fi.pattern.payload[0] = 0x00;
	proto.fi.pattern.payload[1] = 0x01;
	proto.fi.pattern.payload[2] = 0x02;
	proto.fi.pattern.payload[3] = 0x03;

		
	//4. call api
	ret = bsl_setStreamDetail(
			cardid, 
			stream.portid,
			stream.streamid,
			stream.groupid,
			&stream,
			&proto
			);

	if(ret == ResultSuccess) {
		ret = bsl_enableStream(
				cardid,
				stream.portid,
				stream.streamid,
				stream.enable
				);
	}

	if(ret == ResultSuccess) {
		//Tx single packet
		ret = bsl_setControlCommand(
				0,                  //portsel
				cardid,
				0,                  //portid
				stream.streamid,
				CommandSingleStep,
				0,                  //timesec
				0,                  //netmode
				0,                  //mac
				0                   //ip
				);

	}


	//5. closing
	free(proto.l2.pdr);
	free(proto.l3.pdr);
	free(proto.l4.pdr);

	if( ret != 0 ) {
		fprintf( stderr, "%s : ERROR, ret %d\n", argv[0], ret );
	}

	return 0;
}


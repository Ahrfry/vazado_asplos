/*******************************************************************************
Vendor: Mellanox
Associated Filename: stream_transfer.cpp
Purpose: Example for Innova I/F
Revision History: March 28, 2016 - initial release
                                                *
*******************************************************************************
#-  (c) Copyright 2016 Mellanox Technologies Ltd. All rights reserved.
#-
#-
#- ************************************************************************ */
/*
 *
 */

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.hpp"
#include "example.hpp"
#include "topic.h"
#include <pcap.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <netinet/if_ether.h>
//#include "AESfunctions.h"
//#include "AEStables.h"

using namespace std;

int filter_count=0, pub_count=0, sub_count = 0;

struct UDP_hdr {
	u_short	uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	u_short	uh_ulen;		/* datagram length */
	u_short	uh_sum;			/* datagram checksum */
};
/*
unsigned short Nk = 4; // 4 or 6 or 8 [32-bit words] columns in cipher key
unsigned short CipherKeyLenghth = Nk * rows; // bytes
unsigned short Nr = (Nk > Nb) ? Nk + 6 : Nb + 6; // = 10, 12 or 14 rounds
unsigned short ExtdCipherKeyLenghth = (Nr + 1) * stt_lng; // bytes in the expanded cipher key
unsigned char expandedKey[ExtdCipherKeyLenghth_max]= {0x0 ,0x1 ,0x2 ,0x3 ,0x4 ,0x5 ,0x6 ,0x7 ,0x8 ,0x9 ,0xA ,0xB ,0xC ,0xD ,0xE ,0xF ,0xD6 ,0xAA ,0x74 ,0xFD ,0xD2 ,0xAF ,0x72 ,0xFA ,0xDA ,0xA6 ,0x78 ,0xF1 ,0xD6 ,0xAB ,0x76 ,0xFE ,0xB6 ,0x92 ,0xCF ,0xB ,0x64 ,0x3D ,0xBD ,0xF1 ,0xBE ,0x9B ,0xC5 ,0x0 ,0x68 ,0x30 ,0xB3 ,0xFE ,0xB6 ,0xFF ,0x74 ,0x4E ,0xD2 ,0xC2 ,0xC9 ,0xBF ,0x6C ,0x59 ,0xC ,0xBF ,0x4 ,0x69 ,0xBF ,0x41 ,0x47 ,0xF7 ,0xF7 ,0xBC ,0x95 ,0x35 ,0x3E ,0x3 ,0xF9 ,0x6C ,0x32 ,0xBC ,0xFD ,0x5 ,0x8D ,0xFD ,0x3C ,0xAA ,0xA3 ,0xE8 ,0xA9 ,0x9F ,0x9D ,0xEB ,0x50 ,0xF3 ,0xAF ,0x57 ,0xAD ,0xF6 ,0x22 ,0xAA ,0x5E ,0x39 ,0xF ,0x7D ,0xF7 ,0xA6 ,0x92 ,0x96 ,0xA7 ,0x55 ,0x3D ,0xC1 ,0xA ,0xA3 ,0x1F ,0x6B ,0x14 ,0xF9 ,0x70 ,0x1A ,0xE3 ,0x5F ,0xE2 ,0x8C ,0x44 ,0xA ,0xDF ,0x4D ,0x4E ,0xA9 ,0xC0 ,0x26 ,0x47 ,0x43 ,0x87 ,0x35 ,0xA4 ,0x1C ,0x65 ,0xB9 ,0xE0 ,0x16 ,0xBA ,0xF4 ,0xAE ,0xBF ,0x7A ,0xD2 ,0x54 ,0x99 ,0x32 ,0xD1 ,0xF0 ,0x85 ,0x57 ,0x68 ,0x10 ,0x93 ,0xED ,0x9C ,0xBE ,0x2C ,0x97 ,0x4E ,0x13 ,0x11 ,0x1D ,0x7F ,0xE3 ,0x94 ,0x4A ,0x17 ,0xF3 ,0x7 ,0xA7 ,0x8B ,0x4D ,0x2B ,0x30 ,0xC5

};
*/
/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
/*
void AES_Full(bool mode_cipher, bool mode_inverse_cipher,
		unsigned char data_in[stt_lng],
		unsigned char expandedKey[ExtdCipherKeyLenghth_max], unsigned short Nr,
		unsigned char data_out[stt_lng]);
*/
int main()
{

	pcap_t *descr;


	char errbuf[PCAP_ERRBUF_SIZE];

	// open capture file for offline processing
	descr = pcap_open_offline("/home/ahrfry/compress/vasado/one_param.pcap", errbuf);
	if (descr == NULL) {
		std::cout << "pcap_open_live() failed: " << errbuf << std::endl;
		return 1;
	}

	// start packet processing loop, just like live capture
	if (pcap_loop(descr, 0, packet_handler, NULL) < 0) {
		std::cout << "pcap_loop() failed: " << pcap_geterr(descr);
		return 1;
	}



	std::cout << "capture finished" << std::endl;

	printf("Success: HW and SW results match\n");
	return 0;
}

/* Returns a string representation of a timestamp. */
const char *timestamp_string(struct timeval ts);

/* Report a problem with dumping the packet with the given timestamp. */
void problem_pkt(struct timeval ts, const char *reason);

/* Report the specific problem of a packet being too short. */
void too_short(struct timeval ts, const char *truncated_hdr);

/* Callback function invoked by libpcap for every incoming packet */
void packet_parser(const unsigned char *packet, struct timeval ts,
		unsigned int capture_len)
{

		struct ip *ip;
		struct UDP_hdr *udp;
		unsigned int IP_header_length;

		/* For simplicity, we assume Ethernet encapsulation. */

		if (capture_len < sizeof(struct ether_header))
			{
			/* We didn't even capture a full Ethernet header, so we
			 * can't analyze this any further.
			 */
			too_short(ts, "Ethernet header");
			return;
			}

		/* Skip over the Ethernet header. */
		packet += sizeof(struct ether_header);
		capture_len -= sizeof(struct ether_header);

		if (capture_len < sizeof(struct ip))
			{ /* Didn't capture a full IP header */
			too_short(ts, "IP header");
			return;
			}

		ip = (struct ip*) packet;
		IP_header_length = ip->ip_hl * 4;	/* ip_hl is in 4-byte words */

		if (capture_len < IP_header_length)
			{ /* didn't capture the full IP header including options */
			too_short(ts, "IP header with options");
			return;
			}

		if (ip->ip_p != IPPROTO_UDP)
			{

			problem_pkt(ts, "non-UDP packet");
			return;
			}

		/* Skip over the IP header to get to the UDP header. */
		packet += IP_header_length;
		capture_len -= IP_header_length;

		if (capture_len < sizeof(struct UDP_hdr))
			{
			too_short(ts, "UDP header");
			return;
			}

		udp = (struct UDP_hdr*) packet;
		/*
		printf("%s UDP src_port=%d dst_port=%d length=%d\n",
			timestamp_string(ts),
			ntohs(udp->uh_sport),
			ntohs(udp->uh_dport),
			ntohs(udp->uh_ulen));
		*/
		char *payload; /* Packet payload */
		payload = (char *)(packet + 8);

		//cout<<"packet payload "<<payload[0]<<endl;

		if(payload[0] == 's'){
			sub_count++;
		}else if(payload[0] == 'f'){
			filter_count++;
		}else {
			pub_count++;
		}


}


void read_out(stream<stream256Word_t> &Out , const char *path){
	stream256Word_t tmp;
	int counter = 0;

	if(!Out.empty()){
		cout<<"writing to "<<path<<endl;
		pcap_t *dead = pcap_open_dead(DLT_EN10MB, 65535);
		if (!dead) {
			cout<<("pcap_open_dead failed")<<endl;

		}

		pcap_dumper_t *output = pcap_dump_open(dead, path);
		if (!output) {
			cout<<"pcap_dump_open failed"<<endl;

		}

		u_char buffer[65535];
		pcap_pkthdr h = {};
		h.len = 0;


		while(!Out.empty()){
			Out.read(tmp);



			stream256Word_t w = tmp;

			for (int byte = 0; byte < w.data.width / 8; ++byte)
				buffer[h.len + byte] = w.data(w.data.width - 1 - byte * 8,
											  w.data.width - 8 - byte * 8);
			h.len += w.data.width / 8;
			if (w.last) {
				cout<<"writing last fleet to "<<path<<endl;
				for (int i = 0; i < w.data.width / 8; ++i)
					if (!w.keep(i, i))
						--h.len;
					else
						break;

				h.caplen = h.len;

				pcap_dump((u_char *)output, &h, buffer);
				h.len = 0;
				counter++;

			}




		}

		int ret = pcap_dump_flush(output);
		if (ret) {
			cout<<("pcap_dump_flush returned error")<<endl;
		}

		const unsigned char *packet;
		pcap_close(dead);

		struct pcap_pkthdr header;
		//reopen file for testing
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_t *pcap;
		dead = pcap_open_offline(path, errbuf);
		if (dead == NULL){
			fprintf(stderr, "error reading pcap file: %s\n", errbuf);
			exit(1);
		}

		/* Now just loop through extracting packets as long as we have
		 * some to read.
		 */
		while((packet = pcap_next(dead, &header)) != NULL)
			packet_parser(packet, header.ts, header.caplen);

	}

}

unsigned int countZetBits(ap_uint<32> n)
{
    unsigned int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}


/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *packet)
{
	int i = 0;

	static const int width = 256;
	const int b = MLX_AXI4_WIDTH_BYTES;
	typedef ap_uint<MLX_AXI4_WIDTH_BITS> word;

	stream_dat_t expected;
	stream256Word_t tmp,tmp1[DPT],tmp2[DPT];
	stream<stream256Word_t> Ain("Ain");
	stream<stream256Word_t> Bin,Cin;


	#pragma HLS STREAM variable=Ain	depth=16


	stream<stream256Word_t> Aout,Bout,Cout;
	stream256Word_t fleet_holder[10];

	//stream256Word_t Ain[DPT], Bin[DPT], Cin[DPT];
	//stream256Word_t Aout[DPT], Bout[DPT], Cout[DPT];

	//tmp.data = 12345;
	tmp.keep = 0xFFFFFFFF;
	tmp.last = false;
	tmp.id  = 1;
	tmp.user =10;
	int number_of_fleets = 0;
	stream256Word_t arr_tmp[5];
	const char pass[] = "/home/ahrfry/vasado/passthrough.pcap";
	const char reply[] = "/home/ahrfry/vasado/reply.pcap";
	volatile mm_dat_t mem[16];


	for (unsigned word = 0; word < ALIGN(header->len, b); word += b) {

		for (unsigned byte = 0; byte < b && word + byte < header->len; ++byte){
			tmp.data(tmp.data.width - 1 - 8 * byte, tmp.data.width - 8 - 8 * byte) = packet[word + byte];


		}

		if ((word + b) >= header->len) {

			tmp.keep = keep_bytes(header->len - word - b);

			tmp.last = true;


		}

		arr_tmp[number_of_fleets++]= tmp;

	}


	for(int k = 0; k < 10; k++){
		cout<<"Number of Fleets  "<<number_of_fleets<<endl;
		for(int j=0; j < (number_of_fleets); j++){


			//cout<<"Sending Fleet "<<j<<endl;
			/*
			if(j == 1){
				// create a test input data (plaintext) (ABCDEFGHIJKLMNOP)

				unsigned char plaintext[stt_lng] = { '1', '.', '0', '1', 'E', 'F', 'G', 'H',
						'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P' };
				// encrypt
				unsigned char ciphertext[stt_lng];
				//AES_Full(true, false, plaintext, expandedKey, Nr, ciphertext);
				cout << "ciphertext = ";
				for (unsigned short i = 0; i < stt_lng; i++) {
					tmp.data(256 -i*8 -1, 256-8*(i+1)) = ciphertext[i];
					printf("%X ", ciphertext[i]);
				}

				cout << " <=> ";
				unsigned char a;
				for (unsigned short i = 0; i < stt_lng; i++) {
					a = tmp.data(256 -i*8 -1, 256-8*(i+1));
					printf("%X ", a);
				}

				cout << endl;

			}

			if(j==1){
				arr_tmp[j].data(175, 168) = 0;
				arr_tmp[j].data(167, 160) = 1;
				arr_tmp[j].data(159, 152) = 2;
				arr_tmp[j].data(151, 144) = k;

			}*/



			Ain.write(arr_tmp[j]);
			//Bin.write(tmp);
			//Cin.write(tmp);
			example(Ain,Bin,Cin,Aout,Bout,Cout, mem);
		}
		read_out(Aout , pass);
		read_out(Bout , reply);
	}


}

/* Note, this routine returns a pointer into a static buffer, and
 * so each call overwrites the value returned by the previous call.
 */
const char *timestamp_string(struct timeval ts)
	{
	static char timestamp_string_buf[256];

	sprintf(timestamp_string_buf, "%d.%06d",
		(int) ts.tv_sec, (int) ts.tv_usec);

	return timestamp_string_buf;
	}

void problem_pkt(struct timeval ts, const char *reason)
	{
	fprintf(stderr, "%s: %s\n", timestamp_string(ts), reason);
	}

void too_short(struct timeval ts, const char *truncated_hdr)
	{
	fprintf(stderr, "packet with timestamp %s is truncated and lacks a full %s\n",
		timestamp_string(ts), truncated_hdr);
	}


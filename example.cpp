/*******************************************************************************
Vendor: Mellanox:
Associated Filename: example.cpp
Purpose: Example for Innova I/F
Revision History: March 28, 2016 - initial release
                                                *
*******************************************************************************
#-  (c) Copyright 2016 Mellanox Technologies Ltd. All rights reserved.
#-
#-
#- ************************************************************************ */
/*
 * This file contains an example
 */

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "globals.hpp"
#include "example.hpp"
//#include "topic.h"
#include "hash.h"
#include "pred.h"
#include "filters.h"
#include "filter_stage.h"
#include <hls_math.h>

#include <stdio.h>
#include <string.h>
#include <ap_fixed.h>
#include <ap_int.h>
#include "queue.h"
#include "pipeline.hpp"
#include <endian.h>



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using namespace hls;
using namespace std;




void axi_stream_pass_alt(stream<stream256Word_t>& In, stream<stream256Word_t> & Out){
	stream256Word_t buff;
	if (!In.empty()) {

		In.read(buff);
		Out.write(buff);
	}
}

ap_uint<32> swap_this(ap_uint<32> num){
	ap_uint<32> b0,b1,b2,b3;
	ap_uint<32> res;

	b0 = (num & 0x000000ff) << 24u;
	b1 = (num & 0x0000ff00) << 8u;
	b2 = (num & 0x00ff0000) >> 8u;
	b3 = (num & 0xff000000) >> 24u;

	res = b0 | b1 | b2 | b3;
	return res;
}

ap_fixed<32 , 16> to_fixed(ap_uint<32> temp){
	temp = swap_this(temp);
	ap_fixed<32,16> param = reinterpret_cast<float &>(temp);
	return param;
}

float atof(char str[10]){

	float acc = 0;
	int comma_index = 0;
	int base10 = 1000000000;
	float temp2;
	for(int i = 0; i < 10; i++){
		if(str[i] != 46){
			temp2 = str[i] - 48;
			acc = acc + temp2*base10;
			base10 = base10/10;

		}else{
			comma_index = i;
		}


	}

	temp2 = acc / (100000000);

	return temp2;
}

unsigned int countSetBits(ap_uint<32> n)
{
    unsigned int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}



void mem_stream(stream<stream256Word_t>& MemIn, stream<stream512Word_t>& MemOut , volatile mm_dat_t *MEMA){
#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush



	static int sent_count = 1;

	mm_dat_t mem[MEMSIZE];
	static enum ipState {IDLE = 0 , STREAM} DetectState;

	static stream256Word_t buff;
	stream512Word_t mem_buff;

	switch (DetectState)
	{
		case IDLE:
		{

			if (!MemIn.empty()) {

				MemIn.read(buff);
				//memcpy(mem,(const mm_dat_t*)MEMA,3*sizeof(mm_dat_t));

				int nkeys = buff.data(255, 248);
				nkeys = 3;

				for(int i =0; i < nkeys; i++){
		#pragma HLS pipeline II=1
					//mem[i] = MEMA[buff.data(247 - i*16, 247 -i*16 - 15)];
					//mem[i] = MEMA[i];
					//mem[i](15,0)= i;
					cout<<"Mem key "<< i <<" "<<buff.data(247 - i*16, 247 -i*16 - 15)<<endl;

				}

				DetectState = STREAM;

			}
			break;
		}
		case STREAM:
		{
			//mem_buff.data = mem[sent_count];
			mem_buff.data(255,0) = buff.data(255,0);
			mem_buff.last = true;
			mem_buff.data(511, 496) = sent_count;
			mem_buff.data(495, 488) = sent_count;
			mem_buff.data(487, 480) = 0;
			/*
			if( sent_count == 3){
				DetectState = IDLE;
				mem_buff.last = true;
				sent_count = 1;
			}
			sent_count++;
			*/
			MemOut.write(mem_buff);
		}
	}



}


void axi_stream_pass_nw(stream<stream256Word_t>& In, stream<stream256Word_t> & HashOut ,stream<stream256Word_t> & Host)
{
#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush

		static int flit = 0;

		static enum ipState {IDLE = 0 ,VASADO, VASADO_STREAM ,STREAM} DetectState;
		static ap_uint<32> count = 0;
		stream256Word_t buff;

		switch (DetectState)
		{
			case IDLE:
			{
				if (!In.empty()) {

					In.read(buff);

					if(buff.data(159 , 144) == 0x0800 && buff.data(71 , 64) == 0x11){
					   cout<<"Network IDLE "<<buff.data(127 , 127 - 15)<<endl;

					   HashOut.write(buff);
					   DetectState = VASADO;

				   }else{
					   Host.write(buff);
					   DetectState = STREAM;
				   }

				}
				break;
			}
			case VASADO:
			{
				if (!In.empty()) {
					cout<<"Network Vasado"<<endl;
					In.read(buff);
					HashOut.write(buff);

					if(buff.last){

						DetectState = IDLE;

					}else{
						DetectState = VASADO_STREAM;
					}
				}

				break;
			}
			case VASADO_STREAM:
			{

				if (!In.empty()) {
					In.read(buff);
					cout<<"Network Vasado Stream"<<endl;
					HashOut.write(buff);

					if(buff.last){

						DetectState = IDLE;

					}
				}
				break;
			}



			case STREAM:
			{

				if (!In.empty()) {
					In.read(buff);

					Host.write(buff);

					if(buff.last){

						DetectState = IDLE;

					}
				}
				break;
			}


		}



}




void axi_stream_pass_host(stream<stream256Word_t>& In, stream<stream256Word_t> & Network , stream<stream256Word_t> & Host)
{
#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush



	static enum ipState {IDLE = 0 ,VASADO ,STREAM} DetectState;
	static ap_uint<32> count = 0;
	stream256Word_t buff;

	static bool reply = false;


	switch (DetectState)
	{
		case IDLE:
		{
			if (!In.empty()) {

				In.read(buff);

				//will go back to network
				if(buff.data(71 , 64) != 0x11){
					cout<<"HOST PASS TO NETWORK"<<endl;
				   //traffic to host doesn't change

				   Network.write(buff);

				   DetectState = STREAM;
				}else{

					cout<<"HOST PASS TO HOST"<<endl;
					Host.write(buff);
					DetectState = VASADO;

				}



			}
			break;
		}
		case VASADO:
		{
			if (!In.empty()) {
				In.read(buff);
				Host.write(buff);
				if(buff.last){

					DetectState = IDLE;

				}
			}

			break;
		}
		case STREAM:
		{

			if (!In.empty()) {
				In.read(buff);

				Network.write(buff);


				if(buff.last){

					DetectState = IDLE;

				}
			}
			break;
		}


	}
}


void merger_nw(stream<stream256Word_t> inData[2] , stream<stream256Word_t> &outData){

#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush

#pragma HLS ARRAY_PARTITION variable=inData complete

	static enum mState{M_IDLE = 0, M_STREAM} mergeState;
	static ap_uint<2>streamSource    = 0; // Denotes the stream from which the data will be read. 0 = ARP, 1 = ICMP, 2 = Loopback
	stream256Word_t inputWord;
	static bool flip = true;


	switch(mergeState)
	{
		case M_IDLE:
		{

			bool stream_empty[2];
#pragma HLS ARRAY_PARTITION variable=stream_empty complete
			// store all input stream empty states


			stream_empty[0] = inData[0].empty();
			stream_empty[1] = inData[1].empty();

			if (flip && !stream_empty[0]){
				streamSource = 0;
				flip = false;
			}else if(!stream_empty[1]){
				streamSource = 1;
				flip = true;
			}

			if(!stream_empty[streamSource])
			{

				inData[streamSource].read(inputWord);
				outData.write(inputWord);

				if (!inputWord.last){
					mergeState = M_STREAM;

				}

			}
			break;
		}
		case M_STREAM:
		{
			if (!inData[streamSource].empty()) {

				inData[streamSource].read(inputWord);
				outData.write(inputWord);


				if (inputWord.last) {

					mergeState = M_IDLE;
				}
			}
			break;
		}
	}

}

void merger_host(stream<stream256Word_t> inData[2] , stream<stream256Word_t> &outData){
#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush
#pragma HLS ARRAY_PARTITION variable=inData complete dim=1
	static enum mState{M_IDLE = 0, M_STREAM} mergeState;
	static ap_uint<2>streamSource    = 0; // Denotes the stream from which the data will be read. 0 = ARP, 1 = ICMP, 2 = Loopback

	stream256Word_t inputWord;
	static bool flip = true;

	switch(mergeState)
	{
		case M_IDLE:
		{
			bool stream_empty[2];
#pragma HLS ARRAY_PARTITION variable=stream_empty complete
			// store all input stream empty states


				stream_empty[0] = inData[0].empty();
				stream_empty[1] = inData[1].empty();

				if (flip && !stream_empty[0]){
					streamSource = 0;
					flip = false;
				}else if(!stream_empty[1]){
					streamSource = 1;
					flip = true;
				}

				if(!stream_empty[streamSource])
				{

					inData[streamSource].read(inputWord);
					outData.write(inputWord);

					if (!inputWord.last){
						mergeState = M_STREAM;

					}

				}


			break;
		}
		case M_STREAM:
		{
			if (!inData[streamSource].empty()) {

				inData[streamSource].read(inputWord);
				outData.write(inputWord);
				cout<<"FOUND A SOURCE not empty STREAMING"<<endl;

				if (inputWord.last) {

					mergeState = M_IDLE;
				}
			}
			break;
		}
	}

}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void example(   stream<stream256Word_t> &prt_nw2sbu, // network to fpga
				stream<stream256Word_t> &prt_cx2sbu, // NIC to fpga
				stream<stream256Word_t> &mlx2sbu,    // control channel in from the NIC
				stream<stream256Word_t> &sbu2prt_cx, // fpga to NIC
				stream<stream256Word_t> &sbu2prt_nw, // fpga to network
				stream<stream256Word_t> &sbu2mlx,   // control channel out to the NIC
				volatile mm_dat_t *mema
)
{



#pragma HLS DATAFLOW
//#pragma HLS pipeline II=1 enable_flush rewind

#pragma HLS INTERFACE axis port=prt_nw2sbu
#pragma HLS INTERFACE axis port=prt_cx2sbu
#pragma HLS INTERFACE axis port=sbu2prt_nw
#pragma HLS INTERFACE axis port=sbu2prt_cx
#pragma HLS INTERFACE axis port=sbu2mlx
#pragma HLS INTERFACE axis port=mlx2sbu




#pragma HLS INTERFACE m_axi depth=M_AXI_DEPTH port=mema

#pragma HLS interface s_axilite port=return bundle=CONTROL



static	stream<stream256Word_t> merger_network[2];
static	stream<stream256Word_t> merger_host_arr[2];

static	stream<stream256Word_t> hashIn("hashIn");
static	stream<stream256Word_t> hashOut("hashOut");

static	stream<stream256Word_t> memIn("memIn");
static	stream<stream512Word_t> memOut("memOut");
static  stream<stream_awr_t> flowIndex("flowIndex");

#pragma HLS STREAM variable=flowIndex

#pragma HLS STREAM variable=hashIn depth=200
#pragma HLS STREAM variable=hashOut depth=200





#pragma HLS STREAM variable=merger_network depth=16
#pragma HLS STREAM variable=merger_host_arr depth=16


int tot = 1;

for(int i = 0; i<1; i++){
#pragma HLS Pipeline II=1
	axi_stream_pass_nw(prt_nw2sbu , hashIn, merger_host_arr[0]);
}

static Hash hash_table;

for (int i = 0; i<tot; i++){
#pragma HLS Pipeline II=1
	hash_table.interface(hashIn, memIn, memOut , hashOut, flowIndex);
}

for (int i = 0; i<tot; i++){
#pragma HLS Pipeline II=1
	mem_stream(memIn, memOut , mema);
}

static Pipeline pipeline;

for (int i = 0; i<tot; i++){
#pragma HLS Pipeline II=1
	pipeline.interface(hashOut , merger_network[0], flowIndex);
}



for (int i = 0; i<1; i++){
#pragma HLS Pipeline II=1
	axi_stream_pass_host(prt_cx2sbu ,  merger_network[1] , merger_host_arr[1]);
}

	merger_host(merger_host_arr , sbu2prt_cx);

	merger_nw(merger_network , sbu2prt_nw);
	//merger_nw(merger_network , sbu2prt_nw);


	axi_stream_pass_alt(mlx2sbu,sbu2mlx);


}

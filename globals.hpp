#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <stdint.h>
#include <cstdlib>

#include <ap_int.h>

#include <hls_stream.h>

#define ALIGN(x,a)              __ALIGN_MASK(x,(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define MLX_AXI4_WIDTH_BITS 256
#define MLX_AXI4_WIDTH_BYTES (MLX_AXI4_WIDTH_BITS / 8)

using namespace hls;

const ap_uint<48> IOT_MAC_ADDR = 0x248a07a8f9f6; // IOT machine
const ap_uint<48> SERVER_MAC_ADDR = 0x248a07a8f9f2;

static ap_uint<32> keep_bytes(const ap_uint<6>& valid_bytes)
{
	return 0xffffffff ^ ((1 << (32 - valid_bytes)) - 1);
}

// global constants for HLS interfaces directives
const int MEMSIZE = 8;
const int M_AXI_DEPTH = 8;

const int TCA_STREAM_SIZE =  32; // maximum packet stream size is  32 words of 256-bit
const int PUB =  112;
//const int GRID = 262;
//const int CITY = 480;
//const int TAXI = 404;
//const int FIT = 115;


const int large_size = 256;

const int MAX_PARAM = 13;
#define noOfTableEntries 10
#define noOfAreaSlotsMem 100
#define noOfAreaSlotsCache 64
#define slotSize = 512

#define noOfDevSlotsMem 1000
#define noOfDevSlotsCache 128

const int cam_size = 67;
const int cache_size_t = 1003;
const int flow_size = 20;

const int tot_flow = 100;


const int topic_depth = 4;
const int frame_padding = 1;

const int no_of_kernels = 5;

enum PipelineStages {IDLE = 0, ETL, FILTER, STATS, CONTROL, BROKER};

// data types
typedef  ap_uint<256> stream_dat_t;
typedef  ap_uint<512> mm_dat_t;
typedef	 ap_ufixed<32,16> iot_fixed;
typedef	 ap_fixed<32, 16> iot_param;



typedef struct context_buffer{
	ap_uint<16> frame_ids[6];
	ap_uint<16> PC;
	ap_uint<16> frame_index;
	PipelineStages stage;
	ap_uint<16> root;
	ap_uint<16> node_id;
	ap_uint<512> output;
	bool valid;
}context_buffer_t;

typedef struct stage_flow{
	context_buffer_t context_flow;
	ap_uint<512> cache_slots[6];
} stage_flow_t;


typedef struct cam_slot{
	ap_uint<512> frame;
	ap_uint<8> root_id;
	ap_uint<32> node_id;
	ap_uint<8> children[4];
	ap_uint<8> owner;
	ap_uint<2> permission;
	bool valid;
} _cam_slot_t;

typedef struct isa_read{
	ap_uint<3> op; //read,write,next_stage
	ap_uint<3> from; //from
	ap_uint<3> to;
	ap_uint<6> index; //index
	ap_uint<6> range; //range
}isa_read_t;

typedef struct isa_exec{
	ap_uint<3> op; //read,write,next_stage
	ap_uint<3> kernel_id; //from
	ap_uint<3> dependency;
	ap_uint<7> index; //index
	ap_uint<7> range; //range
}isa_exec_t;


typedef struct frame_slot{
	ap_uint<16> topic_hash;
	ap_uint<1024> buff;
} frame_slot_t;

typedef struct topic_extract{
	ap_uint<8> offset[7];
	ap_uint<8> size[7];
}_topic_extract;



typedef struct stream256Word // 256-bit interface
{
public:
   stream_dat_t  data;
   ap_uint<32>   keep;
   ap_uint<1>    last;
   ap_uint<3>    id;
   ap_uint<12>   user;
} stream256Word_t;

typedef struct stream512Word // 256-bit interface
{
public:
	mm_dat_t  data;
   ap_uint<64>   keep;
   ap_uint<1>    last;
   ap_uint<14>   hash_id;
   ap_uint<32>   node_id;
   ap_uint<3>    root;
   bool          in_context;
} stream512Word_t;



typedef struct stream_awr
{
	ap_uint<1>    rdy;
	ap_uint<1>    vld;
	ap_uint<64>   addr;
	ap_uint<3>    prot;
}stream_awr_t;

typedef struct stream_w
{
	ap_uint<1>    rdy;
	ap_uint<1>    vld;
	ap_uint<32>   data;
	ap_uint<4>    strobe;
}stream_w_t;

typedef struct stream_b
{
	ap_uint<1>    rdy;
	ap_uint<1>    vld;
	ap_uint<2>   resp;
}stream_b_t;

typedef struct stream_r
{
	ap_uint<1>    rdy;
	ap_uint<1>    vld;
	ap_uint<32>   data;
	ap_uint<2>    r_resp;
}stream_r_t;

typedef struct stream_mm
{
	ap_uint<64>  data;
	ap_uint<8>   keep;
	ap_uint<1>    last;
	ap_uint<3>    id;
	ap_uint<12>   user;
}stream_mm_t;


struct axiWord
{
   ap_uint<64>  data;
   ap_uint<8>   strb;
   ap_uint<128> user;
   ap_uint<1>   last;
};


#endif // GLOBALS_H_ not defined


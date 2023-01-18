#ifndef HASH_H_
#define HASH_H_

#include "queue.h"
#include "globals.hpp"


typedef struct hash_slot{
	//ap_uint<512> frame;
	//short flow_index;
	short node_id;
	//short root;
	//ap_uint<2> state;
	//bool valid;
} _hash_slot_t;

typedef struct active_flow {
	int root;
	int leaf;
	int frames[5];
}active_flow_t;


typedef struct mem_queue{
	int flow_index;
	int root;
	int node_id;
}mem_queue_t;

typedef struct flow_table_entry{
	ap_uint<24> tags[topic_depth];
	ap_uint<3> ntags;
}flow_table_entry_t;

class Hash {
   public:

		_hash_slot_t CACHE[cache_size_t];
		ap_uint<512> CACHE_FRAME[cache_size_t];
		Queue<int, flow_size> flow_queue;
		active_flow_t active_flow[flow_size];

		flow_table_entry_t flow[tot_flow];

		ap_uint<32> hf(ap_uint<160> data , int table_size, int offset);

		Hash();

		int isInContext(ap_uint<16> index);

		void interface(stream<stream256Word_t>& HashIn,stream<stream256Word_t>& MemOut, stream<stream512Word_t>& MemIn , stream<stream256Word_t>& HashOut, stream<stream_awr_t>& flow_index);

};

#endif


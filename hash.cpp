#include "globals.hpp"
#include "hash.h"


using namespace std;


Hash::Hash(){

	for(int i=1; i < cache_size_t; i++){

		CACHE[i-1].node_id = i;
		//CACHE[i-1].root = 0;
		//CACHE[i-1].state = 0;
		//CACHE[i-1].valid = true;
		//CACHE[i-1].flow_index = i;
		CACHE_FRAME[i-1](31, 0) = 10;
	}

	CACHE_FRAME[0](511, 504) = 1;
	CACHE_FRAME[0](509, 503) = 0;
	CACHE_FRAME[1](511, 510) = 1;
	CACHE_FRAME[1](509, 503) = 1;



	//CACHE[0].state = true;
	//CACHE[1].state = true;

	//CACHE[2].root = 1;
	//CACHE[3].root = 1;

	for(int i=0; i < flow_size; i++){
		this->flow_queue.insert(i);

	}

	for(int i=0; i < tot_flow; i++){
		for(int j=0;  j < topic_depth; j++){
			this->flow[i].tags[j] = j;
		}

	}



}

int Hash::isInContext(ap_uint<16> index){

	//if(CACHE[index].state > 0){
		//return CACHE[index].flow_index;

	//}


	return -1;
}

ap_uint<32> Hash::hf(ap_uint<160> data , int table_size, int offset){
#pragma HLS INLINE
	int i;

	ap_uint<8> hash = 11;
	for (i=159 - offset; i > 159 - offset-40; i = i-8){

#pragma HLS pipeline II=1

		char a = data(i , i-8+1);
		cout<<a;

		hash = (hash<<4)^(hash>>28)^data(i , i-8+1);
	}
	cout<<endl;

	ap_uint<32> key = hash%table_size;

	return key;

}


void Hash::interface(stream<stream256Word_t>& HashIn,stream<stream256Word_t>& MemOut, stream<stream512Word_t>& MemIn , stream<stream256Word_t>& HashOut, stream<stream_awr_t>& flow_index){
#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush



	static int packet_id = 0;


	mm_dat_t mem[MEMSIZE];

	static enum ipState {IDLE = 0 ,VASADO , MEMREAD ,STREAM} DetectState , PrevState;

	static bool mem_interrupt =  false;

	stream256Word_t buff;
	static stream256Word_t prev_buff;
	stream512Word_t mem_buff;

	stream512Word_t pipe_buff;


	switch (DetectState)
	{
		case IDLE:
		{
			if(!MemIn.empty()){
				cout<<"HASH IDLE TO MEMIN READ"<<endl;
				DetectState = MEMREAD;
				PrevState = IDLE;
			}else if (!HashIn.empty()) {
				cout<<"HASH IDLE"<<endl;
				HashIn.read(buff);
				prev_buff = buff;

				DetectState = VASADO;

			}
			break;
		}
		case MEMREAD:
		{
			if(!MemIn.empty()){

				MemIn.read(mem_buff);
				int index = mem_buff.data(511, 496);

				this->CACHE[index].node_id = mem_buff.data(495, 488);
				//this->CACHE[index].node_id = 1;
				cout<<"HASH ---MEMIN READ--- "<<index<<endl;
				CACHE_FRAME[index] = mem_buff.data;


				if(mem_buff.last){
					DetectState = PrevState;
				}
			}
			break;
		}
		case VASADO:
		{
			if(!MemIn.empty()){
				cout<<"HASH VASADO TO MEMIN READ"<<endl;
				DetectState = MEMREAD;
				PrevState = VASADO;
			}else if(!HashIn.empty()){
				cout<<"HASH VASADO STAY VASADO"<<endl;
				HashIn.read(buff);

				ap_uint<160> data = buff.data(159,0);
				int keys[topic_depth+1];
				keys[0] = 0;

#pragma HLS ARRAY_PARTITION variable=keys complete
#pragma HLS ARRAY_PARTITION variable=CACHE complete

				keys[1] = this->hf(data , cache_size_t, 40);
				keys[2] = this->hf(data , cache_size_t, 80);
				keys[3] = this->hf(data , cache_size_t, 120);



				active_flow_t curr_flow;
				int root = buff.data(175, 168) = 0;
				int leaf = buff.data(151, 144) = 2;

				int node_ids[topic_depth];
				int sub_flow = 0;

				curr_flow.root = buff.data(175, 168);
				curr_flow.leaf = buff.data(151, 144);


				//pop from available flow
				ap_int<8> flow_index = this->flow_queue.pop();
				//set flow
				this->active_flow[flow_index] = curr_flow;

				cout<<"current flow index "<<flow_index<<endl;

				stream256Word_t mem_keys_buff;

				int missed_count = 0;


				for(int i = 1; i < topic_depth; i++){
#pragma hls unroll
					//cout<<"index "<<keys[i]<<" cache root "<<this->CACHE[keys[i]].root<<" node id "<<this->CACHE[keys[i]].node_id<<endl;




					if(this->CACHE[keys[i]].node_id != this->flow[sub_flow].tags[i]){
						//mem_keys_buff.data(247 - i*16, 247 -i*16 - 15) = hf(data , cache_size_t, 8*i);
						missed_count++;
						//this->CACHE[keys[i]].flow_index = flow_index;
						//this->CACHE[keys[i]].state = 1;
						//this->CACHE[keys[i]].node_id = node_ids[i];
						cout<<"CACHE MISS"<<endl;
					}else{
						cout<<"CACHE HIT"<<endl;
					}
				}


				if(missed_count > 0){
					mem_keys_buff.data(255, 248) = missed_count;
					MemOut.write(mem_keys_buff);
				}
				cout<<"HASH VASADO"<<endl;
				HashOut.write(buff);

				if(buff.last){
					DetectState = IDLE;
				}else{
					DetectState = STREAM;
				}
			}
			break;
		}
		case STREAM:
		{
			if(!MemIn.empty()){
				cout<<"HASH STREAM TO MEMIN READ"<<endl;
				DetectState = MEMREAD;
				PrevState = STREAM;
			}else if (!HashIn.empty()) {
				cout<<"HASH STREAM"<<endl;
				HashIn.read(buff);
				HashOut.write(buff);

				if(buff.last){

					DetectState = IDLE;

				}
			}
			break;
		}

	}


}


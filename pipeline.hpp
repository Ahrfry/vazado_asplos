#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "globals.hpp"

class Pipeline {
   public:

		Pipeline();

		void interface(stream<stream256Word_t>& In, stream<stream256Word_t> & Out, stream<stream_awr_t>& flow_index);


};

Pipeline::Pipeline(){
	cout<<"Contructed the class"<<endl;
}

void Pipeline::interface(stream<stream256Word_t>& HashIn, stream<stream256Word_t> & Network, stream<stream_awr_t>& flow_index){
#pragma HLS INLINE off
#pragma HLS pipeline II=1 enable_flush



	static enum ipState {IDLE = 0 ,VASADO ,STREAM} DetectState;

	stream256Word_t buff;
	stream256Word_t hash_buff;

	switch (DetectState)
	{
		case IDLE:
		{
			if (!HashIn.empty()) {

				HashIn.read(buff);
				cout<<"PIPE - IDLE"<<endl;
				Network.write(buff);

				DetectState = VASADO;

			}
			break;
		}
		case VASADO:
		{
			if(!HashIn.empty()){

				HashIn.read(buff);
				cout<<"Pipe Vasado"<<endl;
				Network.write(buff);

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

			if (!HashIn.empty()) {
				cout<<"PIPE - STREAM"<<endl;
				HashIn.read(buff);
				Network.write(buff);

				if(buff.last){

					DetectState = IDLE;

				}
			}
			break;
		}

	}
}

#endif

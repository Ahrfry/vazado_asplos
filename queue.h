#ifndef QUEUE_H_
#define QUEUE_H_


using namespace std;

template <typename T, int queue_size>
class Queue{

	public:
		T arr[queue_size];
		int front;
		int rear;
		int item_count;

		Queue();
		T peek();
		bool empty();
		bool is_full();
		T size();
		void insert(T data);
		T pop();


};

template <typename T, int queue_size> Queue<T, queue_size>::Queue(){
	this->rear = -1;
	this->item_count = 0;
	this->front = -1;
}

template <typename T, int queue_size> T Queue<T, queue_size>::peek() {
   return arr[this->front];
}

template <typename T, int queue_size> bool Queue<T, queue_size>::empty() {
   return this->item_count == 0;
}

template <typename T, int queue_size> bool Queue<T, queue_size>::is_full() {
   return this->item_count == queue_size;
}

template <typename T, int queue_size> T Queue<T, queue_size>::size() {
   return this->item_count;
}

template <typename T, int queue_size> void Queue<T, queue_size>::insert(T data) {

	if(!this->is_full()) {

		if(this->front == -1){
			this->rear = this->front = 0;
			this->arr[0] = data;
		}else if(this->rear == (queue_size-1) && this->front !=0) {
			this->rear = 0;
			//cout<<"Inserting data to queue "<<this->front<<" "<<this->rear<<" "<< this->item_count<<endl;

			this->arr[0] = data;
		}else{
			//cout<<"Inserting data to queue "<<this->front<<" "<<this->rear+1<<" "<< this->item_count<<endl;

			this->arr[this->rear+1] = data;
			this->rear++;
		}



		this->item_count+=1;

	}

}

template <typename T, int queue_size> T Queue<T, queue_size>::pop() {
   T data = this->arr[this->front];

   if(this->front == this->rear){
	   this->front = this->rear = -1;
   }else if(this->front == (queue_size-1)) {
      this->front = 0;
   }else{
	   this->front++;
   }

   this->item_count--;
   return data;
}
#endif


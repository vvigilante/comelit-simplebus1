/**
 * @brief 	Circular queue implementation
 */

#ifndef CIRCULARQUEUE_H_
#define CIRCULARQUEUE_H_
#include <algorithm>
#include <iterator>


/**
 * @brief	A circular queue
 */
template<class TInfo>
class CircularQueue{
	private:
		TInfo* 				array;
		TInfo* 				array_limit;
		TInfo* 				front;
		TInfo* 				back;
	public:
		/**
		 * @brief 	Allocate the circular queue structure
		 * @param 	capacity	The capacity of the queue
		 */
		CircularQueue(int capacity);
		/**
		 * @brief 	Deallocate the queue
		 */
		~CircularQueue();

		CircularQueue(const CircularQueue<TInfo>& other){ // copy constructor
			std::copy(std::begin(this->array), std::end(this->array), std::begin(other.array));
			this->array_limit = other.array_limit;
			this->front = other.front;
			this->back = other.back;
		}
		CircularQueue(const CircularQueue<TInfo>&& other){ // move constructor
			this->array = other.array;
			this->array_limit = other.array_limit;
			this->front = other.front;
			this->back = other.back;
			other.array = nullptr;
			other.array_limit = nullptr;
			other.front = nullptr;
			other.back = nullptr;
		}
	    CircularQueue& operator=(const CircularQueue& other){ // copy assignment
			std::copy(std::begin(this->array), std::end(this->array), std::begin(other.array));
			other.array_limit = other.array_limit;
			other.front = other.front;
			other.back = other.back;
		}
	    CircularQueue& operator=(const CircularQueue&& other){ // move assignment
			other.array = other.array;
			other.array_limit = other.array_limit;
			other.front = other.front;
			other.back = other.back;
			other.array = nullptr;
			other.array_limit = nullptr;
			other.front = nullptr;
			other.back = nullptr;
		}

		/**
		 * @brief 	Check if the queue is full
		 * @return 	true	if the queue is full
		 * @return 	false	otherwise
		 */
		bool isFull() const;
		/**
		 * @brief 	Check if the queue is empty
		 * @return 	true	if the queue is empty
		 * @return 	false	otherwise
		 */
		bool isEmpty() const;
		/**
		 * @brief 	Returns the length of the queue
		 * @param	[in] q		The queue
		 * @return 	int	The length
		 */
		int getLength() const;
		/**
		 * @brief 	Add an element to the queue
		 * @return	TInfo*		If the queue is full the element that was removed
		 * @return	NULL		If the queue is not full
		 */
		TInfo* push(const TInfo*);
		/**
		 * @brief 	Remove the oldest element from the queue
		 * @return	TInfo*		The element that was removed
		 */
		TInfo* pop();

		/**
		 * @brief 	Get the ith element starting from the last inserted one (1 is the last)
		 * @param	[in] 	q	The queue
		 * @param	[in] 	i	The index starting from the last inserted element (1)
		 * @return	TInfo*		The element
		 */
		TInfo* getLast(int i);

		/**
		 * @brief	Function callback to visit the queue
		 * @param	element		The current visited element
		 * @param	context		The context passed to visit function
		 */
		typedef void(*CircularQueueVisitor_t)(TInfo* element, void* context);

		/**
		 * @brief 	Run a function on every element in the queue
		 * @param	[in] f	The visitor function
		 * @param	[in] c	The context to be passed to f
		 */
		void visit(CircularQueueVisitor_t f, void* c);

		/**
		 * @brief 	Delete every element from the queue
		 */
		void clear();
};


// -------------------- prototypes ----------------------------------





/* Array     Front==Back               Limit
 * |_________|_________________________|
 * |_________ _________________________|
 */
/* Array     Front      Back           Limit
 * |_________|__________|______________|
 * |_________abcdefghilm ______________|
 */

/* Array     Back                Front Limit
 * |_________|___________________|_____|
 * ghilmnopqr ___________________abcdef|
 */

/* Array BackFront                     Limit
 * |________||_________________________|
 * fghilmnop abcdefghilmnopqrstuvzabcde|
 */
/* Array==Front                    BackLimit
 * |__________________________________||
 * abcdefghilmnopqrstuvzabcdefghilmnop |
 */





// -------------------- implementation ------------------------------
template <class TInfo>
CircularQueue<TInfo>::CircularQueue(int capacity){
	this->array = new TInfo[capacity+1];
	this->array_limit = this->array + (capacity+1);
	this->front = this->array;
	this->back = this->array;
}

template <class TInfo>
CircularQueue<TInfo>::~CircularQueue(){
	if(this->array)
		delete[] this->array;
}

template <class TInfo>
bool CircularQueue<TInfo>::isFull() const{
	if (this->front != this->array)
		return this->back==this->front-1;
	else
		return this->back==this->array_limit-1;
	return false;
}
template <class TInfo>
void CircularQueue<TInfo>::clear(){
	this->front = this->array;
	this->back = this->array;
}


template <class TInfo>
bool CircularQueue<TInfo>::isEmpty() const{
	return this->front == this->back;
}
template <class TInfo>
int CircularQueue<TInfo>::getLength() const{
	if (this->back >= this->front){
		return (int)(this->back - this->front);
	}
	return (int)( this->back - this->array + this->array_limit - this->front );
}

template <class TInfo>
TInfo* CircularQueue<TInfo>::push(const TInfo* element){ // add to back
	TInfo* retval;
	// If full, pop oldest one
	if(this->isFull()){
		retval= this->pop();
	}else{
		retval= NULL;
	}

	*(this->back) = *element;
	this->back++;
	if(this->back >= this->array_limit){
		this->back = this->array;
	}

	return retval;
}

template <class TInfo>
TInfo* CircularQueue<TInfo>::pop(){ // take from front (oldest)
	if(this->isEmpty())
		return NULL;
	TInfo* retval = this->front;
	if(++this->front == this->array_limit)
		this->front = this->array;
	return retval;
}

template <class TInfo>
TInfo* CircularQueue<TInfo>::getLast(int i){ // inspect newest (without deleting)
	if( i > this->getLength() || i<=0 )
		return NULL;
	if(this->back-i >= this->array)
		return this->back-i;
	else
		return this->array_limit-i+(this->back-this->array);
}

template <class TInfo>
void CircularQueue<TInfo>::visit(CircularQueueVisitor_t f, void* c){
	TInfo* i;
	for(i=this->front; i!=this->back; i++, i=(i==this->array_limit)?this->array : i){
		f(i, c);
	}
}


#endif /* CIRCULARQUEUE_H_ */

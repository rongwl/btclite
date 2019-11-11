#ifndef BTCLITE_CIRCULAR_BUFFER_H
#define BTCLITE_CIRCULAR_BUFFER_H


#include <memory>


template <typename T>
class CircularBuffer {
public:
    CircularBuffer(size_t max_size)
        :  buf_(new T[max_size]), max_size_(max_size) {}
    
    //-------------------------------------------------------------------------
    void push_back(const T& item);
    T pop_front();
    
    //-------------------------------------------------------------------------
    void clear()
    {
        head_ = tail_;
        full_ = false;
    }
    
    bool empty() const
    {
        return (!full_ && head_ == tail_);
    }
    
    size_t size() const;
    
    size_t capacity() const
    {
        return max_size_;
    }
    
private:
    std::unique_ptr<T*> buf_;
    size_t head_ = 0;
	size_t tail_ = 0;
	const size_t max_size_;
	bool full_ = false;
};

#endif // BTCLITE_CIRCULAR_BUFFER_H

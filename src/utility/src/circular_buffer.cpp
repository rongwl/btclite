#include "circular_buffer.h"


template <typename T>
void CircularBuffer<T>::push_back(const T& item)
{
    buf_[tail_] = item;
    if (full_)
        head_ = (head_ + 1) % max_size_;
    tail_ = (tail_ + 1) % max_size_;
    full_ = (head_ == tail_);
}

template <typename T>
T CircularBuffer<T>::pop_front()
{
    if (empty())
        return T();
    
    T result = buf_[head_];
    head_ = (head_ + 1) % max_size_;
    full_ = false;
    
    return result;
}

template <typename T>
size_t CircularBuffer<T>::size() const
{
    size_t size = 0;
    
    if (full_)
        return max_size_;
    
    if (head_ <= tail_)
        size = tail_ - head_;
    else
        size = tail_ + max_size_ - head_;
    
    return size;
}

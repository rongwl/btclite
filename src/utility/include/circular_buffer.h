#ifndef BTCLITE_CIRCULAR_BUFFER_H
#define BTCLITE_CIRCULAR_BUFFER_H


#include <stdexcept>
#include <memory>


namespace btclite {
namespace util {

template <typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t max_size)
        :  buf_(new T[max_size]), max_size_(max_size) {}
    
   //-------------------------------------------------------------------------
    void push_back(const T& item);
    T pop_front();
    
    void clear()
    {
        head_ = tail_;
        full_ = false;
    }
    
    //-------------------------------------------------------------------------
    T& operator[](size_t n);
    const T& operator[](size_t n) const;
    
    //-------------------------------------------------------------------------
    bool exist(const T& val) const
    {
        return (std::find(buf_.get(), buf_.get() + size(), val) != buf_.get() + size());
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
    std::unique_ptr<T[]> buf_;
    size_t head_ = 0;
	size_t tail_ = 0;
	const size_t max_size_;
	bool full_ = false;
};

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
T& CircularBuffer<T>::operator[](size_t n)
{
    if (n >= size())
        throw std::out_of_range("CircularBuffer");
    
    return buf_[(head_ + n) % max_size_];
}

template <typename T>
const T& CircularBuffer<T>::operator[](size_t n) const
{
    if (n >= size())
        throw std::out_of_range("CircularBuffer");
    
    return buf_[(head_ + n) % max_size_];
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

} // namespace util
} // namespace btclit

#endif // BTCLITE_CIRCULAR_BUFFER_H

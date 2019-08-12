// Exception thrown when an attempt is made to
// use an Iter that exceeds the range of the
// underlying object.
//
class OutOfRangeExc
{
    // Add functionality if needed by your application.
};

// An iterator-like class for cycling through arrays
// that are pointed to by GCPtrs. Iter pointers
// ** do not ** participate in or affect garbage
// collection. Thus, an Iter pointing to
// some object does not prevent that object
// from being recycled.

template <class T>
class Iter
{
   private: 
        T *ptr_;
        T *end_;

        T *begin_;           // points to start of allocated array
        size_t length_;      
  
    public:   
        Iter()
        {
            ptr_ = nullptr;
            end_ = nullptr;
            begin_ = NULL;
            length_ = 0;
        }

        Iter(const T *p, const T *first, const T *last)
        {
            ptr_ = p;
            end_ = last;
            begin_ = first;
            length_ = last - first;
        }

        // Return length of sequence to which this
        // Iter points.
        size_t size() 
        {    
            return length_;
        }
    
        // Return value pointed to by ptr.
        // Do not allow out-of-bounds access.
        T &operator*()
        {
            if ((ptr_ >= end_) || (ptr_ < begin_))
            {
                throw std::out_of_range("Invalid access");
            }
            return *ptr_;
        }

        // Return address contained in ptr.
        // Do not allow out-of-bounds access.
        T* operator->()
        {
            if ((ptr_ >= end_) || (ptr_ < begin_))
            {
                throw std::out_of_range("Invalid access");
            }       
            return ptr_;
        }

        // Prefix ++.
        Iter operator++()
        {
            ptr_++;
            return *this;
        }

        // Prefix --.
        Iter operator--()
        {
            ptr_--;
            return *this;
        }

        // Return a reference to the object at the
        // specified index. Do not allow out-of-bounds
        // access.
        T& operator[](size_t i)
        {
            if ((i < 0) || (i >= (end_ - begin_)))
            {
                std::out_of_range("Invalid access");
            }         
            return ptr_[i];
        }

        // Define the relational operators.
        bool operator==(const Iter& op2)
        {
            return ptr_ == op2.ptr_;
        }

        bool operator!=(const Iter<T>& op2)
        {
            return ptr_ != op2.ptr_;
        }

        bool operator<(const Iter<T>& op2)
        {
            return ptr_ < op2.ptr_;
        }

        bool operator<=(const Iter<T>& op2)
        {
            return ptr_ <= op2.ptr_;
        }

        bool operator>(const Iter<T>& op2)
        {
            return ptr_ > op2.ptr_;
        }

        bool operator>=(const Iter<T>& op2)
        {   
            return ptr_ >= op2.ptr_;
        }

        // Subtract an integer from an Iter.
        Iter operator-(int n)
        {
            ptr_ -= n;
            return *this;
        }   

        // Add an integer to an Iter.
        Iter operator+(int n)
        {
            ptr_ += n;
            return *this;
        }

        // Return number of elements between two Iters.
        int operator-(const Iter<T>& itr2)
        {
            return ptr_ - itr2.ptr_;
        }
};

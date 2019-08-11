// This class defines an element that is stored
// in the garbage collection information list.
//
template <class T>
class PtrDetails
{
    public:
        size_t ref_count_ = 0;
        T *mem_ptr_ = nullptr;
        bool is_Array_ = false; 
        size_t arraySize = 0;
    
        explicit PtrDetails(T* obj_ptr, size_t arr_size = 0) : mem_ptr_(obj_ptr), arr_size_(arr_size),
        is_arr_(arr_size > 0) noexcept
        {
            ref_count++;
        }

    private:
        PtrDetails(const PtrDetails&) = delete;
        PtrDetails& operator=(const PtrDetails&) = delete;
    
};


// This is needed by the STL list class.
template <class T>
bool operator==(const PtrDetails<T> &ob1, const PtrDetails<T> &ob2)
{
    return (ob1.mem_ptr_ == ob2.mem_ptr_); 
}
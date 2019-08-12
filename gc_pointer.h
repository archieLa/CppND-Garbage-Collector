#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include <stdexcept>
#include <cassert>
#include "gc_details.h"
#include "gc_iterator.h"

/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
    A Pointer must only be used to point to memory
    that was dynamically allocated using new.
    When used to refer to an allocated array,
    specify the array size.
*/

template <class T, int size = 0>
class Pointer
{

private:
    // refContainer maintains the garbage collection list.
    static std::list<PtrDetails<T> > sRefContainer;
    static bool sFirst;         // true when first Pointer is created
    
    // addr points to the allocated memory to which
    // this Pointer pointer currently points.
    T* addr_ = nullptr;
    bool is_array_ = false;
    size_t array_size_ = size;     // size of the array
        
    // Return an iterator to pointer details in refContainer.
    typename std::list<PtrDetails<T> >::iterator find_ptr_info(const T* ptr);
    void increment_or_add_to_ptr_list();
    void increment_ptr_list();

    // Helper iterator variables
    typename std::list<PtrDetails<T> >::iterator it_end_of_ptr_list_;
    typename std::list<PtrDetails<T> >::iterator it_mem_;

public:
    // Define an iterator type for Pointer<T>.
    using GCiterator = Iter<T>;

    // A utility function that displays refContainer.
    static void show_list();
    // Clear refContainer when program exits.
    static void shutdown();
    
    // NOTE: templates aren't able to have prototypes with default arguments
    // this is why constructor is designed like this:
    Pointer()
    {
        Pointer(nullptr);
    }

    Pointer(T* mem);  
    Pointer(const Pointer& rhs);   
    ~Pointer(); 

    // Collect garbage. Returns true if at least
    // one object was freed.
    static bool collect();
    // Overload assignment of pointer to Pointer.
    T* operator=(T* memory);
    // Overload assignment of Pointer to Pointer.
    Pointer& operator=(const Pointer &rhs);
    
    // Return a reference to the object pointed
    // to by this Pointer.
    T& operator*()
    {
        return *addr_;
    }

    // Return the address being pointed to.
    T* operator->() 
    { 
        return addr_;
    }

    // Return a reference to the object at the
    // index specified by i.
    T& operator[](size_t index)
    { 
        if (is_array_)
        {
            if (index >= array_size_)
            {
                throw std::out_of_range("Invalid index");
            }
            return addr_[index];
        }
        else
        {
            return *addr_;
        }

    }

    // Conversion function to T *.
    operator T*() 
    {
        return addr_; 
    }
    
    // Return an Iter to the start of the allocated memory.
    GCiterator begin()
    {
        size_t lsize;
        if (is_array_)
        {
            lsize = array_size_;
        }
        else
        {
            lsize = 1;
        }
        return GCiterator(addr_, addr_, addr_ + lsize);
    }

    // Return an Iter to one past the end of an allocated array.
    GCiterator end()
    {
        size_t lsize;
        if (is_array_)
        {
            lsize = array_size_;
        }           
        else
        {
            lsize = 1;
        }            
        return GCiterator(addr_ + lsize, addr_, addr_ + lsize);
    }

    static int ref_container_size() 
    { 
        return sRefContainer.size();
    }

};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T>> Pointer<T, size>::sRefContainer;

template <class T, int size>
bool Pointer<T, size>::sFirst = true;

template<class T,int size>
Pointer<T,size>::Pointer(T* mem)
{
    // Register shutdown() as an exit function.
    if (sFirst)
    {
        atexit(shutdown);
        sFirst = false;
    }

    if (size)
    {
        is_array_ = true;
        array_size_ = size;
    }

    addr_ = mem;
    // If class is being set to nullptr don't do any of the checks below
    // it must be assigned a valid dynamically allocated memory before it can proceed
    if (addr_ == nullptr)
    {
        return;
    }
    increment_or_add_to_ptr_list();
}

// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &rhs)
{
    addr_ = rhs.addr_;
    is_array_ = rhs.is_array_;
    array_size_ = rhs.array_size_;
    
    // If the rhs shared pointer was pointing to null don't do anything else
    if(addr_ == nullptr)
    {
        return;
    }

    it_end_of_ptr_list_ = sRefContainer.end(); 
    it_mem_ = find_ptr_info(addr_); 
    
    // There is def an issue if we can't find a memory, that is already handled by another shared ptr
    // Possible issue is that the other pointer has been allocated on the stack
    assert(it_mem_ != it_end_of_ptr_list_); 
    it_mem_->ref_count_++;

}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer()
{  
    typename std::list<PtrDetails<T>>::iterator end_of_list = sRefContainer.end();
    typename std::list<PtrDetails<T>>::iterator p;
    
    if (addr_ != nullptr)
    {
        p = find_ptr_info(addr_);
    }

    if((p != end_of_list)  && p->ref_count_)
    {
        p->ref_count_--;
    }        
    
    collect();

}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect()
{
    bool memfreed = false;
    typename std::list<PtrDetails<T> >::iterator p;
    
    do
    {
        // Scan refContainer looking for unreferenced pointers.
        for (p = sRefContainer.begin(); p != sRefContainer.end(); p++)
        {
            // If in-use, skip.
            if (p->ref_count_ != 0)
            {
                continue;
            }
         
            if(p->mem_ptr_ != nullptr)
            {
                if(p->is_array_)
                {
                    delete[] p->mem_ptr_;
                }
                else
                {
                    delete p->mem_ptr_;
                }
                memfreed = true;
            }
                       
            sRefContainer.remove(*p);
            break;
        }
    } while (p != sRefContainer.end());
    
    return memfreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T* Pointer<T, size>::operator=(T* mem)
{
    if (addr_ != nullptr)
    {
        it_end_of_ptr_list_ = sRefContainer.end(); 
        it_mem_ = find_ptr_info(addr_); 
        assert(it_mem_ != it_end_of_ptr_list_);
        it_mem_->ref_count_--;
    }

    addr_ = mem;
    if (size)
    {
        is_array_ = true;
        array_size_ = size;
    }
    increment_or_add_to_ptr_list();
    return addr_;
}

template<class T, int size>
void Pointer<T, size>::increment_or_add_to_ptr_list()
{

    if (addr_ == nullptr)
    {
        return;
    }    

    it_end_of_ptr_list_ = sRefContainer.end(); 
    it_mem_ = find_ptr_info(addr_);    
    // Find if we are just another user of a memory already allocated
    if (it_mem_ != it_end_of_ptr_list_)
    {
        // This memory is already being used and looked after
        // Make sure that both ptr details and this pointer properly indicate
        // if the memory is an array, if not something is def wrong assert and exit
        assert((it_mem_->is_array_ == is_array_) && (it_mem_->array_size_ == array_size_));
        // Everything looks good increment 
        it_mem_->ref_count_++;
    }
    else
    {
        // This is newly created memory
        sRefContainer.emplace_back(addr_, array_size_);
    }
}

// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size>& Pointer<T, size>::operator=(const Pointer& rhs)
{
    if (addr_ != nullptr)
    {
        it_end_of_ptr_list_ = sRefContainer.end(); 
        it_mem_ = find_ptr_info(addr_); 
        // There def is a problem, we should be able to find
        // a memory address guided by this object if it wasn't null
        assert(it_mem_ != it_end_of_ptr_list_);
        it_mem_->ref_count_--;
    }

    addr_ = rhs.addr_;
    is_array_ = rhs.is_array_;
    array_size_ = rhs.array_size_;
    
    increment_ptr_list();
    return *this;

}

template<class T, int size>
void Pointer<T, size>::increment_ptr_list()
{
    if (addr_ == nullptr)
    {
        return;
    }   

    it_end_of_ptr_list_ = sRefContainer.end(); 
    it_mem_ = find_ptr_info(addr_); 

    // There is def an issue if we can't find a memory, that is already handled by another shared ptr
    // Possible issue is that the other pointer has been allocated on the stack
    assert(it_mem_ != it_end_of_ptr_list_); 
    it_mem_->ref_count_++;
}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::show_list()
{
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (sRefContainer.begin() == sRefContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    
    for (p = sRefContainer.begin(); p != sRefContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Find a pointer in refContainer
// Returns iterator pointing to end element of container if not found
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator Pointer<T, size>::find_ptr_info(const T* ptr)
{
    typename std::list<PtrDetails<T> >::iterator p;
    
    // Find ptr in refContainer.
    for (p = sRefContainer.begin(); p != sRefContainer.end(); p++)
    {
        if (p->mem_ptr_ == ptr)
        {
            return p;
        }
    }
    // Return end of the container indicating pointer was not found
    return p;
}

// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown()
{
    if (ref_container_size() == 0)
    {
        return;
    }
        
    typename std::list<PtrDetails<T> >::iterator p;
    
    for (p = sRefContainer.begin(); p != sRefContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->ref_count_ = 0;
    }   
    collect();
}
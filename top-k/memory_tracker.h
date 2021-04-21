/**
 * @file memory_tracker.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <utility>
#include <memory>
#include <iostream>

template<class T> class allocator_mt: public std::allocator<T>
{
private:
    size_t curr_mem_buf = 0; // All allocators constructed with the first allocator will share the same buf

public:
    size_t *curr_mem = &curr_mem_buf;

public:
    allocator_mt() throw() { }

    allocator_mt(const allocator_mt& other) throw(): curr_mem(other.curr_mem) { }
 
    template <typename U>
    allocator_mt(const allocator_mt<U> &other) throw(): curr_mem(other.curr_mem) { }

    ~allocator_mt() throw() { }

    template<typename U> struct rebind
    {
        typedef allocator_mt<U> other;
    };

public:
    inline T* allocate(size_t count)
    {
        T *ptr = new T[count];
        *curr_mem += count * sizeof(T);
        return ptr;
    }

    inline void deallocate(T* ptr, size_t count)
    {
        delete[] ptr;
        *curr_mem -= count * sizeof(T);
    }

    inline const size_t get_allocated_mem() const
    {
        return *curr_mem;
    }
};
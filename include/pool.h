#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <new>

namespace pool {

class Pool;

Pool * create_pool(std::size_t size, std::initializer_list<std::size_t> sizes);

void destroy_pool(Pool * pool);

std::size_t pool_obj_size(const Pool & pool);

void * allocate(Pool & pool, std::size_t n);

void deallocate(Pool & pool, const void * ptr);

} // namespace pool

class PoolAllocator
{
private:
    std::reference_wrapper<pool::Pool> m_pool;

public:
    PoolAllocator(const std::reference_wrapper<pool::Pool> & pool)
        : m_pool(pool)
    {
    }

    PoolAllocator(const PoolAllocator & other)
        : m_pool(other.m_pool)
    {
    }

    PoolAllocator(const std::size_t count, std::initializer_list<std::size_t> sizes)
        : m_pool(*create_pool(count, sizes))
    {
    }

    inline static pool::Pool * create_pool(const std::size_t size, std::initializer_list<std::size_t> sizes)
    {
        return pool::create_pool(size, sizes);
    }

    inline static void destroy_pool(pool::Pool * pool)
    {
        pool::destroy_pool(pool);
    }

    void * allocate(const std::size_t n)
    {
        return pool::allocate(m_pool, n);
    }

    template <class T>
    void deallocate(T * ptr)
    {
        return pool::deallocate(m_pool, ptr);
    }
};

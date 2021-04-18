#pragma once

#include "pool.h"

#include <cstdint>
#include <iostream>
#include <memory>

class AllocatorWithPool
{
    using Alloc = PoolAllocator;

private:
    std::unique_ptr<pool::Pool, decltype(&Alloc::destroy_pool)> m_pool;
    Alloc m_alloc;

public:
    AllocatorWithPool(const std::size_t size, std::initializer_list<std::size_t> sizes)
        : m_pool(Alloc::create_pool(size, sizes), Alloc::destroy_pool)
        , m_alloc(*m_pool)
    {
    }

    template <class T, class... Args>
    T * create(Args &&... args)
    {
        auto * ptr = m_alloc.allocate(sizeof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    template <class T>
    void destroy(T * ptr)
    {
        m_alloc.deallocate(ptr);
        ptr->~T();
    }
};

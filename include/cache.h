#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iostream>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_top_size(cache_size)
        , m_max_low_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
    {
    }

    std::size_t size() const
    {
        return priv_queue.size() + not_priv_queue.size();
    }

    bool empty() const
    {
        return priv_queue.empty() && not_priv_queue.empty();
    }

    template <class T>
    T & get(const Key & key);

    std::ostream & print(std::ostream & strm) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }

private:
    const std::size_t m_max_top_size; //priv
    const std::size_t m_max_low_size; //not_priv
    Allocator m_alloc;
    std::deque<KeyProvider *> priv_queue;
    std::deque<KeyProvider *> not_priv_queue;
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    auto it = std::find_if(priv_queue.begin(), priv_queue.end(), [&key](const KeyProvider * elem) {
        return *elem == key;
    });
    if (it != priv_queue.end()) {
        while (it != priv_queue.begin()) {
            const auto old = it--;
            std::iter_swap(it, old);
        }
        return static_cast<T &>(*priv_queue.front());
    }
    else {
        auto it2 = std::find_if(not_priv_queue.begin(), not_priv_queue.end(), [&key](const KeyProvider * elem) {
            return *elem == key;
        });
        if (it2 != not_priv_queue.end()) {
            while (it2 != not_priv_queue.begin()) {
                const auto old = it2--;
                std::iter_swap(it2, old);
            }
            auto temp = not_priv_queue.front();
            not_priv_queue.pop_front();
            if (priv_queue.size() == m_max_top_size) {
                not_priv_queue.push_back(priv_queue.back());
                priv_queue.pop_back();
            }
            priv_queue.push_front(temp);
            return static_cast<T &>(*priv_queue.front());
        }
        else {
            if (m_max_low_size == not_priv_queue.size()) {
                m_alloc.destroy(not_priv_queue.front());
                not_priv_queue.pop_front();
            }
            T * elem = m_alloc.template create<T>(key);
            not_priv_queue.push_back(elem);
            return static_cast<T &>(*not_priv_queue.back());
        }
    }
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    return strm << "Priority: <empty>"
                << "\nRegular: <empty>"
                << "\n";
}
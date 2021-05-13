#include "pool.h"

#include <assert.h>
#include <cstddef>
#include <memory>
#include <new>
#include <set>
#include <vector>

using std::size_t;

namespace pool {

class Pool
{
public:
    Pool(const std::size_t count, std::initializer_list<std::size_t> sizes)
    {
        for (auto & element : sizes) {
            obj_sizes.insert(element);
        }
        m_used_map.resize(obj_sizes.size());
        m_storage.resize(obj_sizes.size());
        for (size_t i = 0; i < obj_sizes.size(); i++) {
            m_used_map[i].resize(count);
            m_storage[i].resize(count);
        }
    }

    size_t get_obj_size() const
    {
        size_t res = 0;
        for (const size_t element : obj_sizes) {
            res += element;
        }
        return res;
    }

    void * allocate(size_t n);

    std::pair<size_t, size_t> getInd(const void * b_ptr);

    void remove(const std::byte * b_ptr, size_t ind, size_t len);

    void deallocate(const void * ptr);

private:
    static constexpr size_t npos = static_cast<size_t>(-1);

    size_t find_empty_place(size_t n, int ind) const;
    std::set<size_t> obj_sizes;
    std::vector<std::vector<std::byte>> m_storage;
    std::vector<std::vector<bool>> m_used_map;
};

size_t Pool::find_empty_place(const size_t n, int ind) const
{
    if (static_cast<size_t>(ind) >= m_used_map.size() || n > m_used_map[ind].size()) {
        return npos;
    }
    for (size_t i = 0; i < m_used_map[ind].size(); ++i) {
        if (m_used_map[ind][i]) {
            continue;
        }
        size_t j = i;
        while (j < std::min(m_used_map[ind].size(), i + n)) {
            if (m_used_map[ind][j]) {
                break;
            }
            j++;
        }
        if (n == j - i) {
            return i;
        }
        i = j;
    }
    return npos;
}

void * Pool::allocate(const size_t n)
{
    int ind = 0;
    for (const auto element : obj_sizes) {
        if (element == n) {
            break;
        }
        ind++;
    }
    const size_t pos = find_empty_place(n, ind);
    if (pos != npos) {
        for (size_t i = pos, end = pos + n; i < end; ++i) {
            m_used_map[ind][i] = true;
        }
        return &m_storage[ind][pos];
    }
    throw std::bad_alloc{};
}

std::pair<size_t, size_t> Pool::getInd(const void * b_ptr)
{
    size_t ind = 0;
    auto len = *obj_sizes.begin();
    for (const auto indS : obj_sizes) {
        auto begin = &m_storage[ind][0];
        auto end = &m_storage[ind][m_storage[ind].size() - 1];
        if (b_ptr >= begin && b_ptr <= end) {
            len = indS;
            break;
        }
        ind++;
    }
    return std::pair<size_t, size_t>(ind, len);
}

void Pool::remove(const std::byte * b_ptr, size_t ind, size_t len)
{
    if (ind < m_storage.size()) {
        auto begin = &m_storage[ind][0];
        const size_t offset = b_ptr - begin;
        assert(((b_ptr - begin) % len) == 0);
        if (offset < m_used_map[ind].size()) {
            const size_t end_delete = offset + std::min(len, m_used_map[ind].size() - offset);
            for (size_t i = offset; i < end_delete; ++i) {
                m_used_map[ind][i] = false;
            }
        }
    }
}

void Pool::deallocate(const void * ptr)
{
    auto b_ptr = static_cast<const std::byte *>(ptr);
    std::pair<size_t, size_t> pair = getInd(b_ptr);
    remove(b_ptr, pair.first, pair.second);
}

Pool * create_pool(const std::size_t size, std::initializer_list<std::size_t> sizes)
{
    return new Pool(size, sizes);
}

void destroy_pool(Pool * pool)
{
    delete pool;
}

size_t pool_obj_size(const Pool & pool)
{
    return pool.get_obj_size();
}

void * allocate(Pool & pool, const size_t n)
{
    return pool.allocate(n);
}

void deallocate(Pool & pool, const void * ptr)
{
    pool.deallocate(ptr);
}

} // namespace pool
#include "allocator.h"
#include "cache.h"

#include <iostream>
#include <stdlib.h>
#include <string>

namespace {

struct String
{
    std::string data;
    bool marked = false;

    String(const std::string & key)
        : data(key)
    {
    }

    bool operator==(const std::string & other) const
    {
        return data == other;
    }
};

using TestCache = Cache<std::string, String, AllocatorWithPool>;

} // anonymous namespace

int main()
{
    TestCache cache(4, 280, std::initializer_list<std::size_t>{sizeof(String)});
    std::string line;
    for (int c = 0; c < 10; c++) {
        std::getline(std::cin, line);
        //while (std::getline(std::cin, line)) {
        auto & s = cache.get<String>(line);
        if (s.marked) {
            std::cout << "known" << std::endl;
        }
        else {
            std::cout << "unknown" << std::endl;
        }
        s.marked = true;
    }
    std::cout << "\n"
              << cache << std::endl;
}

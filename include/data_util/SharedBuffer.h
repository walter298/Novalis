#pragma once

#include <cstddef>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

namespace nv {
    class SharedBuffer {
    private:
        boost::local_shared_ptr<std::byte[]> m_buff;
    public:
        SharedBuffer(size_t size) : m_buff{ boost::make_local_shared<std::byte[]>(size)}
        {
        }
        template<typename Allocator>
        SharedBuffer(Allocator& allocator, size_t size) 
            : m_buff{ boost::allocate_local_shared<std::byte>(allocator, size) } 
        {
        }
        
        std::byte* data() noexcept {
            return m_buff.get();
        }

        template<std::integral Index>
        std::byte* operator[](Index i) noexcept {
            return m_buff + i;
        }
    };
}
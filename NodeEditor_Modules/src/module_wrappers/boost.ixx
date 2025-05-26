export module boost;

#include <boost/unordered_flat_map.hpp>

export namespace nv {
    template<typename T, typename U>
    using hash_map = boost::unordered_flat_map<T, U>;
}
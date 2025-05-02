//#include <boost/graph/adjacency_list.hpp>
//#include <boost/unordered/unordered_flat_set.hpp>
//#include <boost/graph/tiernan_all_cycles.hpp>
//#include <novalis/BasicConcepts.h>
//#include <novalis/detail/reflection/DataMemberUtil.h>

#include <utility>

namespace nv {
    namespace editor {
        template<typename Func>
        struct CycleVisitor {
            Func func;

            template<typename U>
            CycleVisitor(U&& func) : func{ std::forward<U>(func) } {}

            template<typename Path, typename Graph>
            void cycle(const Path& p, const Graph& g) {
                func(p);
            }
        };

		template<typename U>
		CycleVisitor(U&& u) -> CycleVisitor<std::remove_cvref_t<U>>;
    }
}
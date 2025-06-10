#include <novalis/BufferedNode.h>

namespace nv {
	namespace editor {
		template<bool IsBase = true, typename T>
		static constexpr void calculateSizeBytes(const T& t, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			if constexpr (concepts::Primitive<T>) {
				objectRegionLengths.get<T>() += sizeof(T);
			} else if constexpr (std::ranges::viewable_range<T>) {
				using ValueType = typename T::value_type;
				for (const auto& elem : t) {
					calculateSizeBytes(elem, objectRegionLengths);
				}
			} else {
				if constexpr (IsBase) {
					objectRegionLengths.get<T>() += sizeof(T);
				}
				nv::detail::forEachDataMember([&]<typename Field>(const Field & field) {
					if constexpr (!concepts::Primitive<Field>) {
						calculateSizeBytes<false>(field, objectRegionLengths);
					}
					return nv::detail::STAY_IN_LOOP;
				}, t);
			}
		}
	}
}
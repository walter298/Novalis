#pragma once

#include <variant>
#include <boost/functional/hash.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include "detail/NodeBase.h"
#include "detail/memory/AlignedBuffer.h"

namespace nv {
	namespace detail {
		struct BufferedString {
			static constexpr const char* LEN_KEY = "Offset";

			char* buff = nullptr;
			size_t len = 0;

			inline bool operator==(const BufferedString& other) const noexcept {
				if (len != other.len) {
					return false;
				}
				return strncmp(buff, other.buff, len) == 0;
			}

			template<concepts::String String>
			inline bool operator==(const String& other) const noexcept {
				if (len != std::ranges::size(other)) {
					return false;
				}
				return strncmp(buff, other.data(), len) == 0;
			}

			template<size_t N>
			inline bool operator==(const char(&other)[N]) const noexcept {
				if (len != N) {
					return false;
				}
				return strncmp(buff, other, len) == 0;
			}
		};
	}
}

namespace std {
	template<>
	struct hash<nv::detail::BufferedString> {
		size_t operator()(const nv::detail::BufferedString& str) const noexcept {
			return std::hash<std::string_view>()(std::string_view{ str.buff, str.len });
		}
	};

	template<size_t N>
	struct hash<char[N]> {
		size_t operator()(const char (&str)[N]) const noexcept {
			return std::hash<std::string_view>()(std::string_view{ str, N });
		}
	};
}

namespace nv {
	class BufferedNode; //IMPORTANT: forward declare so that traits can see it

	namespace detail {
		template<std::equality_comparable Key, typename T>
		class BufferedObjectMap {
		public:
			struct Entry {
				Key key;
				T object;
				bool isTombstone = true;
			};
		private:
			std::span<Entry> m_arr;

			static void deepCopyMapEntry(const std::byte* srcArena, std::byte* destArena,
				const Entry& srcEntry, Entry& destEntry)
			{
				matchOffset(srcArena, srcEntry.key.buff, destArena, destEntry.key.buff);
				matchOffset(srcArena, srcEntry.object, destArena, destEntry.object);
			}
		public:
			static void copy(const std::byte* srcArena, std::byte* destArena,
				const BufferedObjectMap& srcMap, BufferedObjectMap& destMap)
			{
				for (auto&& [srcEntry, destEntry] : std::views::zip(srcMap.m_arr, destMap.m_arr)) {
					//deepCopyMapEntry(srcArena, destArena, srcEntry, destEntry);
					matchOffset(srcArena, srcEntry.key.buff, destArena, destEntry.key.buff);
					matchOffset(srcArena, srcEntry.object, destArena, destEntry.object);
				}
			}

			static constexpr size_t ENTRY_SIZE = sizeof(Entry);

			BufferedObjectMap() = default;
			BufferedObjectMap(std::span<Entry> entries) : m_arr{ entries } 
			{
			}

			template<typename U>
			decltype(auto) at(this auto&& self, const U& otherKey) noexcept {
				decltype(auto) hashIdx = std::hash<U>{}(otherKey) % self.m_arr.size();

				for (size_t i = 0; i < self.m_arr.size(); i++) {
					auto& [key, objPtr, isTombstone] = self.m_arr[hashIdx];
					if (isTombstone) {
						continue;
					}
					if (key.operator==(otherKey)) { //.operator== inhibits argument dependent lookup
						return std::forward_like<decltype(self)>(unptrwrap(objPtr));
					}
					hashIdx = (hashIdx + 1) % self.m_arr.size();
				}
				if constexpr (concepts::Printable<U>) {
					std::println("Error: could not find {}", otherKey);
				}
				else {
					std::println("Error: could not find object");
				}
				std::abort();
				std::unreachable();
				return std::forward_like<decltype(self)>(unptrwrap(self.m_arr[0].object)); //only here so that return type can be deduced
			}

			void insert(const Key& newKey, T object) noexcept {
				assert(m_arr.data());

				size_t hashIdx = std::hash<Key>{}(newKey) % m_arr.size();
				for (size_t i = 0; i < m_arr.size(); i++) {
					auto& [key, objPtr, isTombstone] = m_arr[hashIdx];
					if (isTombstone) {
						key = newKey;
						objPtr = std::move(object);
						isTombstone = false;
						return;
					}
					hashIdx = (hashIdx + 1) % m_arr.size();
				}
				std::println("Error: no space left in hashmap!");
				std::abort();
			}

			auto begin(this auto&& self) {
				return self.m_arr.begin();
			}
			auto end(this auto&& self) {
				return self.m_arr.end();
			}
			using mapped_type = T*;
			using key_type = BufferedString;
		};
	}

	namespace detail {
		struct BufferedNodeTraits {
			using String = BufferedString;

			using Node    = BufferedNode*;
			using Polygon = BufferedPolygon;

			template<typename Key, typename Value>
			using ObjectLookupMap = BufferedObjectMap<Key, Value>;

			template<typename T>
			using ObjectStorage = std::span<T>;

			using ObjectGroup = std::tuple<
				std::span<Texture*>, 
				std::span<BufferedPolygon*>, 
				std::span<BufferedNode*>
			>;
			using ObjectGroupMap = ObjectLookupMap<BufferedString, ObjectGroup>;

			using ObjectLookups = std::tuple<
				ObjectLookupMap<BufferedString, Texture*>,
				ObjectLookupMap<BufferedString, BufferedPolygon*>,
				ObjectLookupMap<BufferedString, BufferedNode*>
			>;

			using Layer = std::tuple<
				std::span<Texture>, std::span<BufferedPolygon>, std::span<Node>
			>;
			using Layers   = nv::detail::ObjectLayers<std::span<Layer>>;
			using LayerMap = ObjectLookupMap<BufferedString, Layer*>;
		};

		static_assert(NodeTraits<BufferedNodeTraits>);
	}

	class BufferedNode : public detail::NodeBase<detail::BufferedNodeTraits> {
	private:
		/*unique_ptr means that the buffered node is the root, otherwise having a raw pointer
		means that we are a child whose memory points to a subsection of data in the root*/
		using Arena = std::variant<detail::AlignedBuffer<std::byte>, std::byte*>;

		Arena m_arena = nullptr;
		size_t m_byteC = 0;

		static void copyNodeSpan(const std::byte* srcArena, std::byte* destArena, const std::span<BufferedNode*>& srcSpan,
			std::span<BufferedNode*>& destSpan);
		static void deepCopyLayer(const std::byte* srcArena, std::byte* destArena,
			const Layer& srcLayer, Layer& destLayer);
		static void copyLayers(const std::byte* srcArena, std::byte* destArena,
			const Layers& srcLayers, Layers& destLayers);

		template<typename T>
		static void deepCopySpan(const std::byte* srcArena, std::byte* destArena, const std::span<T>& srcSpan,
			std::span<T>& destSpan)
		{
			//make dest span point to the same relative address to its arena
			T* destSpanPtr = nullptr;
			nv::detail::matchOffset(srcArena, srcSpan.data(), destArena, destSpanPtr);
			destSpan = { destSpanPtr, srcSpan.size() };

			if constexpr (std::same_as<T, nv::BufferedNode*>) {
				copyNodeSpan(srcArena, destArena, srcSpan, destSpan);
			} else if constexpr (std::same_as<T, nv::BufferedPolygon>) {
				for (auto&& [srcPoly, destPoly] : std::views::zip(srcSpan, destSpan)) {
					nv::detail::PolygonConverter::deepCopyBufferedPolygons(srcArena, destArena, srcPoly, destPoly);
				}
			} else {
				std::ranges::copy_n(srcSpan.begin(), srcSpan.size(), destSpan.begin());
			}
		}

		static void deepCopyChild(const nv::BufferedNode& src, const std::byte* srcArena,
			nv::BufferedNode& dest, std::byte* destArena);

		BufferedNode() = default;
	public:
		template<typename T>
		using Map = ObjectLookupMap<detail::BufferedString, T>;

		template<typename T>
		using ObjectMapEntry = typename Map<T>::Entry;

		template<typename T>
		using TypeMap = detail::TypeMap<
			T,
			char,
			Point,
			Texture,
			Texture*,
			BufferedPolygon,
			BufferedPolygon*,
			BufferedNode,
			BufferedNode*,
			std::byte*, //child node arena region
			detail::BufferedNodeTraits::Layer,
			ObjectMapEntry<nv::Texture*>,
			ObjectMapEntry<BufferedPolygon*>,
			ObjectMapEntry<BufferedNode*>,
			ObjectGroupMap::Entry,
			LayerMap::Entry
		>;
		using RegionMap = TypeMap<detail::MemoryRegion>;
		
		BufferedNode(const BufferedNode& other);
		BufferedNode(BufferedNode&&) noexcept = default;

		size_t getSizeBytes() const noexcept {
			return m_byteC;
		}

		friend struct nlohmann::adl_serializer<BufferedNode>;
	};
}
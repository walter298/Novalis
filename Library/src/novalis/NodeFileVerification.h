#include <expected>

#include "detail/serialization/DynamicNodeSerialization.h"
#include "detail/serialization/BufferedNodeSerialization.h"

#define CHECK_EXPECTED(expected) \
if (!expected) { \
	std::println(stderr, "{}", expected.error()); \
	return false; \
}

namespace nv {
	using ObjectRegions = detail::TypeMap<size_t, Point, Texture, DynamicPolygon>;

	namespace detail {
		template<typename T>
		std::expected<T, std::string> find(json& j, std::string_view key) {
			auto keyIt = j.find(key);
			if (keyIt == j.end()) {
				return std::unexpected{ std::format("Error: {} key not found", key) };
			}
			return *keyIt;
		}

		void writeAllocation(const DynamicPolygon& poly, ObjectRegions& objectRegions) {
			auto bufferedPoly = PolygonConverter::makeBufferedPolygon(poly);
			objectRegions.get<DynamicPolygon>() += sizeof(BufferedPolygon);
			objectRegions.get<Point>() += (sizeof(Point) * (poly.getSize() * 2));
		}
		void writeAllocation(const Texture& tex, ObjectRegions& objectRegions) {
			objectRegions.get<Texture>() += sizeof(Texture);
		}
	}

	bool isNodeFileCorrect(const std::string& filename) noexcept {
		using namespace detail;
		using namespace json_constants;
		using NodeParser = nlohmann::adl_serializer<BufferedNode>;

		bool correct = true;

		std::ifstream file{ filename };
		auto nodeJson = json::parse(file);

		//auto totalBytes = find<size_t>(nodeJson, BYTES_KEY);
		//CHECK_EXPECTED(totalBytes);

		ObjectRegions parsedObjectRegionLengths{ 0 };
		parsedObjectRegionLengths.forEach([&]<typename Object>(auto& bytes) {
			using BufferedObject = std::conditional_t<
				std::same_as<Object, DynamicPolygon>,
				BufferedPolygon, Object>;

			const auto& key = NodeParser::typeSizeKey<BufferedObject>();
			auto keyValue = find<size_t>(nodeJson, key);
			if (!keyValue) {
				correct = false;
				std::println("Error: {}", keyValue.error());
				return;
			}
			bytes = keyValue.value();
		});

		ObjectRegions dynamicObjectRegionLengths{ 0 };

		auto dynamicNode = nodeJson.get<DynamicNode>();
		dynamicNode.forEach([&]<typename Object>(auto layer, const Object& object) {
			if constexpr (std::same_as<Object, Texture> || std::same_as<Object, Point> || std::same_as<Object, DynamicPolygon>) {
				writeAllocation(object, dynamicObjectRegionLengths);
			}
			return nv::detail::STAY_IN_LOOP;
		});

		ObjectRegions::forEachZip(
			parsedObjectRegionLengths, dynamicObjectRegionLengths, [&]<typename Object>(auto dynamicByteC, auto parsedByteC) 
		{
			if (parsedByteC != dynamicByteC) {
				correct = false;
				std::println("Error: {} Size Parameters {} (Dynamically Loaded) != {} (Parsed)",
					getTypeName<Object>(), parsedByteC, dynamicByteC
				);
			}
		});
		
		return correct;
	}
}
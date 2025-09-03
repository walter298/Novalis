#pragma once

#include <vector>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <nlohmann/json.hpp>

namespace nlohmann {
	template<typename K, typename V>
	struct adl_serializer<boost::unordered_flat_map<K, V>> {
		static void from_json(const json& j, boost::unordered_flat_map<K, V>& map) {
			map.reserve(j.size());
			for (const auto& kvpJson : j) {
				auto [k, v] = kvpJson.get<std::pair<K, V>>();
				map.emplace(std::move(k), std::move(v));
			}
		}
		static void to_json(json& j, const boost::unordered_flat_map<K, V>& map) {
			std::vector v{ std::from_range, map | std::ranges::views::transform([](const auto& kvp) {
				return std::pair{ kvp.first, kvp.second };
			})};
			j = std::move(v);
		}
	};
	template<typename K>
	struct adl_serializer<boost::unordered_flat_set<K>> {
		static void from_json(const json& j, boost::unordered_flat_set<K>& set) {
			set.reserve(j.size());
			for (const auto& kJson : j) {
				set.insert(kJson.get<K>());
			}
		}
		static void to_json(json& j, const boost::unordered_flat_set<K>& set) {
			std::vector v{ std::from_range, set };
			j = std::move(v);
		}
	};
}
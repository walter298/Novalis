#pragma once

#include <nlohmann/json.hpp>
#include <plf_hive.h>

namespace plf {
	template<typename T>
	void to_json(nlohmann::json& j, const hive<T>& hive) {
		j = nlohmann::json::array();
		j.insert(hive.begin(), hive.end());
	}
	template<typename T>
	void from_json(const nlohmann::json& j, hive<T>& hive) {
		hive.insert(j.begin(), j.end());
	}
}
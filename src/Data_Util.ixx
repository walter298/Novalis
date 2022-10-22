#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <iostream>
#include <type_traits>

export module Data_Util;

namespace nv {
	template<typename T>
	concept Integral = std::is_integral<T>::value;

	export template<Integral... Nums>
	void parseSpacedNums(const std::string& line, Nums&... nums)
	{
		size_t index = 0;

		auto numify = [&](auto& x) {
			size_t iIndex = index; //initial index

			while (true) {
				index++;
				if (line[index] == '_' || index == line.size()) {
					index++;
					break;
				}
			}

			x = std::stoi(line.substr(iIndex, index));
		};

		(numify(nums), ...);
	}

	export template<Integral... Nums> 
	std::string writeNums(Nums&... nums) {
		std::string ret;

		int argCount = sizeof...(nums);

		auto add = [&](auto num) {
			argCount--;
			if (argCount == 0) {
				ret.append(std::to_string(num));
			} else {
				ret.append(std::to_string(num) + "_");
			}
		};

		((add(nums)), ...);

		return ret;
	}

	export std::tuple<std::string, int, int> staticObjectData(std::string& line) {
		std::string path;
		int x, y;

		size_t numIndex; //index of numbers in line 

		//parse path
		for (numIndex = 0; numIndex < line.size(); numIndex++) {
			if (line[numIndex] == '@') {
				numIndex++;
				break;
			}
			path.push_back(line[numIndex]);
		}

		//parse numbers
		parseSpacedNums(line.substr(numIndex, line.size() - 1), x, y);

		return std::make_tuple(path, x, y);
	}
}
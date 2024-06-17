#include "Benchmarks.h"

#include <print>

#include "../DataUtil.h"

template<nv::Aggregate Aggr>
Aggr operator/(Aggr aggr1, Aggr aggr2) {
	Aggr ret;
	nv::iterateStructs([](const auto& aggr1Mem, const auto& aggr2Mem, auto& retMem) {
		retMem = std::remove_cvref_t<decltype(retMem)>{ aggr1Mem / aggr2Mem };
		return nv::STAY_IN_LOOP;
	}, aggr1, aggr2, ret);
	return ret;
}

namespace chrono = std::chrono;
using time_point = chrono::system_clock::time_point;

struct ArenaBenchmark {
	chrono::milliseconds allocTime{ 0 };
	chrono::milliseconds deallocTime{ 0 };
};

template<typename Vec>
static ArenaBenchmark createVec() {
	ArenaBenchmark ret;
	time_point destructionStart;
	{
		auto start = std::chrono::system_clock::now();
		Vec vec;
		typename Vec::value_type str = "abcdefghijklmnopqrstuvwxyz12345678910";
		for (int i = 0; i < 500000; i++) {
			vec.push_back(str);
		}
		ret.allocTime = std::chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start);
		destructionStart = chrono::system_clock::now();
	}
	ret.deallocTime = std::chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - destructionStart);
	return ret;
}

void nv::benchmarks::compArenaVsNormalAllocation() {
	Arena::initialize(500000000);

	auto [arenaAllocTime, arenaDeallocTime] = createVec<ArenaVector<ArenaString>>();
	auto [allocTime, deallocTime] = createVec<std::vector<std::string>>();

	std::println("Allocation Time: {} versus {}", arenaAllocTime, allocTime);
	std::println("Deallocation Time: {} versus {}", arenaDeallocTime, deallocTime);

	Arena::reset();
}

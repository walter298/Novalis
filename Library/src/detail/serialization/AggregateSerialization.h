#pragma once

#include <nlohmann/json.hpp>

#include "../reflection/ClassIteration.h"

namespace nlohmann {
	template<typename Aggr>
	void from_json(const json& j, Aggr& aggr) 
		requires(std::is_aggregate_v<Aggr> && !nv::detail::MemberIterable<Aggr>) 
	{
		using Tuple = decltype(boost::pfr::structure_to_tuple(aggr));
		auto parsedTuple = j.get<Tuple>();
		
		nv::detail::forEachDataMember([](auto& aggrMem, auto& tupleMem) {
			aggrMem = std::move(tupleMem);
			return nv::detail::STAY_IN_LOOP;
		}, aggr, parsedTuple);
	}

	template<typename Aggr>
	void to_json(json& j, const Aggr& aggr) 
		requires(std::is_aggregate_v<Aggr> && !nv::detail::MemberIterable<Aggr>)
	{
		j = boost::pfr::structure_to_tuple(aggr);
	}

	template<nv::detail::MemberIterable Iterable>
	void from_json(const json& j, Iterable& iterable) {
		using Tuple = typename Iterable::__Tuple;
		Tuple dummy = j;
		nv::detail::forEachDataMember([](const auto& tupleElem, auto& iterableElem) {
			iterableElem = tupleElem;
			return nv::detail::STAY_IN_LOOP;
		}, dummy, iterable);
	}

	template<nv::detail::MemberIterable Iterable>
	void to_json(json& j, const Iterable& iterable) {
		using Tuple = typename Iterable::__Tuple;
		Tuple dummy;
		nv::detail::forEachDataMember([](auto& tupleElem, const auto& iterableElem) {
			tupleElem = iterableElem;
			return nv::detail::STAY_IN_LOOP;
		}, dummy, iterable);
		j = dummy;
	}
}
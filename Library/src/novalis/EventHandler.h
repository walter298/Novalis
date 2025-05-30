#pragma once

#include <chrono>
#include <functional>
#include <boost/unordered/unordered_flat_map.hpp>
#include <SDL3/SDL_keyboard.h>

#include "detail/reflection/FunctionTraits.h"
#include "detail/reflection/ClassIteration.h"
#include "Event.h"
#include "ID.h"

namespace nv {
	class EventHandler {
	private:
		template<typename Ret>
		using Events = boost::unordered_flat_map<ID<Event<Ret>>, Event<Ret>>;
	
		std::tuple<Events<void>, Events<bool>> m_events;

		template<typename T>
		struct IDParameter {};

		template<typename P>
		struct IDParameter<ID<P>> { using Param = P; };

		template<typename ID>
		constexpr decltype(auto) getEvents() {
			using Param = typename IDParameter<ID>::Param;
			return std::get<boost::unordered_flat_map<ID, Event<Param>>>(m_events);
		}
	public:
		using EventID            = ID<Event<void>>;
		using CancellableEventID = ID<Event<bool>>;
	private:
		std::tuple<std::vector<EventID>, std::vector<CancellableEventID>> m_cancelledEventQueues;
	public:
		template<typename Event>
		void cancel(ID<Event> id) {
			//std::get<std::vector<ID<Event>>(m_cancelledEventQueues).push_back(id);
		}

		template<typename Func>
		auto add(Func&& func) {
			// using FuncInfo = FunctionTraits<std::decay_t<Func>>;
			// using FuncRet  = typename FuncInfo::Ret;
			
			// using IDSpecialization = ID<typename GetParameterizedTypeFromTuple<Event, FuncRet>::type>;
			// IDSpecialization id;
		
			// //getEvents<IDSpecialization>().emplace(id, std::forward<Func>(func));
			
			// return id;
			return 0;
		}
				
		template<typename Func, typename Rep, typename Period>
		auto add(Func&& func, std::chrono::duration<Rep, Period> period) {
			// using FuncInfo = detail::FunctionTraits<std::decay_t<Func>>;
			// using FuncRet  = typename FuncInfo::Ret;
			
			// using Duration = std::chrono::duration<Rep, Period>;
		
			// auto periodicEvtWrapper = [func = std::forward<Func>(func), 
			// 							period = period,
			// 							lastExecuted = std::chrono::system_clock::now()](auto... args) mutable 
			// {
			// 	auto now = std::chrono::system_clock::now();
						
			// 	auto timeElapsed = duration_cast<Duration>(now - lastExecuted);
						
			// 	if (timeElapsed < period) {
			// 		if constexpr (std::same_as<FuncRet, bool>) {
			// 			return false;
			// 		} else {
			// 			return;
			// 		}
			// 	}
			// 	lastExecuted = now;
			// 	auto timesToExecute = timeElapsed / period;
		
			// 	for (auto i : views::iota(0, timesToExecute)) {
			// 		if constexpr (std::same_as<FuncRet, bool>) {
			// 			if (func(args...)) {
			// 				return true;
			// 			}
			// 		} else {
			// 			func(args...);
			// 		}
			// 	}
			// };

			// return add(std::move(periodicEvtWrapper));
		}

		void operator()() {
			// //cancelled all events queued to be cancelled
			// detail::forEachDataMember([this]<typename ID>(std::vector<ID>& ids) {
			// 	auto& events = getEvents<ID>();
			// 	for (const auto& id : ids) {
			// 		events.erase(id);
			// 	}
			// 	ids.clear();
			// 	return detail::STAY_IN_LOOP;
			// }, m_cancelledEventQueues);

			// auto& events = std::get<Events<void>>(m_events);
			// for (auto& [id, event] : events) {
			// 	event();
			// }
			// auto& cancellableEvents = std::get<Events<bool>>(m_events);
			// for (auto& [id, event] : cancellableEvents) {
			// 	if (event()) {
			// 		std::get<std::vector<CancellableEventID>>(m_cancelledEventQueues).push_back(id);
			// 	}
			// }
		}
	};
}
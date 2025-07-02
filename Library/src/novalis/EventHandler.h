#pragma once

#include <chrono>
#include <ranges>
#include <tuple>
#include <vector>
#include <boost/unordered/unordered_flat_map.hpp>

#include "detail/reflection/FunctionTraits.h"
#include "Event.h"
#include "ID.h"
#include "KeyState.h"

namespace nv {
	class EventHandler {
	private:
		template<typename Ret>
		using Events = boost::unordered_flat_map<ID<Event<Ret>>, Event<Ret>>;
	
		std::tuple<Events<void>, Events<bool>> m_events;

		template<typename T>
		struct IDParameter {};

		template<typename R>
		struct IDParameter<ID<Event<R>>> { using Ret = R; };

		template<typename ID>
		constexpr decltype(auto) getEvents() {
			using Param = typename IDParameter<ID>::Ret;
			return std::get<Events<Param>>(m_events);
		}
	public:
		using EventID            = ID<Event<void>>;
		using CancellableEventID = ID<Event<bool>>;
	private:
		std::tuple<std::vector<EventID>, std::vector<CancellableEventID>> m_cancelledEventQueues;
	public:
		static constexpr bool CONTINUE_EVENT = false;
		static constexpr bool END_EVENT      = true;

		static bool isKeyPressed(keys::Key key) {
			static auto keys = SDL_GetKeyboardState(nullptr);
			return keys[static_cast<SDL_Scancode>(key)];
		}

		template<typename T>
		void cancel(ID<T> id) {
			//std::get<std::vector<ID<T>>(m_cancelledEventQueues).push_back(id);
		}

		template<typename Func>
		auto add(Func&& func) {
			using FuncInfo = detail::FunctionTraits<std::decay_t<Func>>;
			using FuncRet  = typename FuncInfo::Ret;
			
			using IDSpecialization = ID<Event<FuncRet>>;
			IDSpecialization id;
		
			getEvents<IDSpecialization>().emplace(id, std::forward<Func>(func));
			
			return id;
		}

		template<typename FirstEvent, typename SecondEvent, typename... Events>
		void chain(FirstEvent firstEvent, SecondEvent secondEvent, Events... chainedEvents) {
			add([this, first = firstEvent, next = std::move(secondEvent), 
						... chainedEvents = std::move(chainedEvents)]() mutable
			{
				if (first()) {
					if constexpr (sizeof...(Events) > 0) {
						chain(std::move(next), std::move(chainedEvents)...);
					} else {
						add(std::move(next));
					}
					return END_EVENT;
				}
				return CONTINUE_EVENT;
			});
		}

		template<typename Rep, typename Period, typename FirstEvent, typename SecondEvent, typename... Events>
		void chain(std::chrono::duration<Rep, Period> period, FirstEvent firstEvent, SecondEvent secondEvent, Events... chainedEvents) {
			add([this, period, first = firstEvent, next = std::move(secondEvent),
				... chainedEvents = std::move(chainedEvents)]() mutable
			{
				if (first()) {
					if constexpr (sizeof...(Events) > 0) {
						chain(std::move(next), std::move(chainedEvents)...);
					} else {
						add(std::move(next), period);
					}
					return END_EVENT;
				}
				return CONTINUE_EVENT;
			}, period);
		}
				
		template<typename Func, typename Rep, typename Period>
		auto add(Func&& func, std::chrono::duration<Rep, Period> period) {
			using FuncInfo = detail::FunctionTraits<std::decay_t<Func>>;
			using FuncRet  = typename FuncInfo::Ret;
			
			using Duration = std::chrono::duration<Rep, Period>;
		
			auto periodicEvtWrapper = [func = std::forward<Func>(func), 
			 							period = period,
			 							lastExecuted = std::chrono::system_clock::now()]() mutable 
			{
			 	auto now = std::chrono::system_clock::now();
						
			 	auto timeElapsed = std::chrono::duration_cast<Duration>(now - lastExecuted);
						
			 	if (timeElapsed < period) {
			 		if constexpr (std::same_as<FuncRet, bool>) {
			 			return false;
			 		} else {
			 			return;
			 		}
			 	}
			 	lastExecuted = now;
			 	auto timesToExecute = timeElapsed / period;
		
			 	for (auto i : std::views::iota(0, timesToExecute)) {
			 		if constexpr (std::same_as<FuncRet, bool>) {
			 			if (func()) {
			 				return true;
			 			}
			 		} else {
			 			func();
			 		}
			 	}

				if constexpr (std::same_as<FuncRet, bool>) {
					return false;
				} else {
					return;
				}
			};

			return add(std::move(periodicEvtWrapper));
		}

		void operator()();
	};
}
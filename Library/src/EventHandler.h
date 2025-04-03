#pragma once

#include <chrono>
#include <functional>

#include <SDL3/SDL_keyboard.h>

#include "Event.h"
#include "ID.h"

namespace nv {
	class EventHandler {
	private:
		template<typename Object>
		using IDObjects = std::vector<std::pair<Object, ID<Object>>>;

		template<typename Ret, typename... EventParams>
		using Events = IDObjects<Event<Ret, EventParams...>>;

		template<typename... EventArgs>
		struct EventData {
			Events<void, EventArgs...> events;
			Events<bool, EventArgs...> cancellableEvents;

			std::vector<typename Events<bool, EventArgs...>::iterator> cancelledEventIterators;
		};

		const bool* m_keystate = SDL_GetKeyboardState(nullptr);

		std::tuple<EventData<>, EventData<MouseData>, EventData<const uint8_t*>> m_eventData;
	public:
		template<typename Func>
		auto addEvent(Func&& func) {
			using FuncInfo = FunctionTraits<std::decay_t<Func>>;
			using FuncArgs = typename FuncInfo::Args;
			using FuncRet  = typename FuncInfo::Ret;
			using FuncSig  = typename FuncInfo::Sig;
		
			using IDSpecialization = ID<typename GetParameterizedTypeFromTuple<Event, FuncSig>::type>;
			IDSpecialization id;
		
			using EventDataSpecialization = typename GetParameterizedTypeFromTuple<EventData, FuncArgs>::type;
			auto& eventData = std::get<EventDataSpecialization>(m_eventData);
					
			if constexpr (std::same_as<FuncRet, bool>) {
				eventData.cancellableEvents.emplace_back(std::forward<Func>(func), id);
			} else {
				eventData.events.emplace_back(std::forward<Func>(func), id);
			}
			return id;
		}
				
		template<typename Func, typename Rep, typename Period>
		auto addEvent(Func&& func, std::chrono::duration<Rep, Period> period) {
			using FuncInfo = FunctionTraits<std::decay_t<Func>>;
			using FuncArgs = typename FuncInfo::Args;
			using FuncRet = typename FuncInfo::Ret;
			using FuncSig = typename FuncInfo::Sig;
		
			using IDSpecialization = ID<typename GetParameterizedTypeFromTuple<Event, FuncSig>::type>;
			IDSpecialization id;
		
			using namespace std::chrono;
			using Duration = duration<Rep, Period>;
		
			auto periodicEvtWrapper = [func = std::forward<Func>(func), 
										period = period,
										lastExecuted = system_clock::now()](auto... args) mutable 
			{
				auto now = system_clock::now();
						
				auto timeElapsed = duration_cast<Duration>(now - lastExecuted);
						
				if (timeElapsed < period) {
					if constexpr (std::same_as<FuncRet, bool>) {
						return false;
					} else {
						return;
					}
				}
				lastExecuted = now;
				auto timesToExecute = timeElapsed / period;
		
				for (auto i : views::iota(0, timesToExecute)) {
					if constexpr (std::same_as<FuncRet, bool>) {
						if (func(args...)) {
							return true;
						}
					} else {
						func(args...);
					}
				}
			};
		
			using EventDataSpecialization = typename GetParameterizedTypeFromTuple<EventData, FuncArgs>::type;
			auto& eventData = std::get<EventDataSpecialization>(m_eventData);
		
			if constexpr (std::same_as<FuncRet, bool>) {
				eventData.cancellableEvents.emplace_back(std::move(periodicEvtWrapper), id);
			} else {
				eventData.events.emplace_back(std::move(periodicEvtWrapper), id);
			}
			return id;
		}
	};
}
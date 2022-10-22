#include <vector>
#include <map>
#include <functional>
#include <iostream>

#include "SDL.h"

export module EventHandler;

namespace nv {
	export class EventHandler {
	private:
		typedef std::function<bool()> Condition;
		typedef std::function<void()> Reaction;
		
		typedef std::map<Uint32, std::vector<std::pair<Reaction, size_t>>> SDLEventMap;
		typedef std::map<size_t, std::pair<Condition, Reaction>> CustomMap;

		static inline SDLEventMap m_eventMap, m_keyEventMap;

		static inline CustomMap m_customEventMap;

		static inline SDL_Event m_evt;

		static inline const Uint8* m_keyState = SDL_GetKeyboardState(NULL);

		static inline size_t IDCount = 1;

		static void runKeyboardEvents() {
			for (auto& [key, functions] : m_keyEventMap) {
				if (m_keyState[key]) {
					for (auto& [func, ID] : functions) {
						func();
					}
				}
			}
		}
	public:
		static void reset(Reaction quit) { 
			//clear all the events
			m_eventMap.clear(); 
			m_customEventMap.clear();
			m_keyEventMap.clear();

			//repush permanent events
			m_eventMap[SDL_KEYDOWN].push_back(std::make_pair(runKeyboardEvents, 0)); 
			m_eventMap[SDL_QUIT].push_back(std::make_pair(quit, 1));

			IDCount = 1; //reset taken IDs
		}

		static void runEvents() {
			while (SDL_PollEvent(&m_evt)) {
				//if event is not mapped to any reactions, ignore it
				if (!m_eventMap.count(m_evt.type)) { 
					continue;
				}
				
				for (auto& [func, ID] : m_eventMap.at(m_evt.type)) {
					func();
				}
			}
		}

		static inline const SDL_Event getEvent() {
			return m_evt;
		}

		struct Event {
		protected:
			const size_t m_ID;

			static size_t reserveID() {
				IDCount++;
				return IDCount;
			}

			Event() : m_ID(reserveID()) {}

			bool isKey(Uint32 key) {
				return (4 <= key && key <= 39);
			}
		public:
			virtual void push() = 0;
			virtual void cancel() = 0;
		};

		class InputEvent : public Event {
		protected:
			Uint16 m_eventType;

			Reaction m_event;

			bool m_isKeyEvent = false;
		public:
			virtual void push() override {
				m_eventMap[m_eventType].push_back(std::make_pair(m_event, m_ID));
			}

			virtual void cancel() override {
				for (size_t i = 0; i < m_eventMap[m_eventType].size(); i++) {
					if (m_eventMap[m_eventType][i].second == m_ID) {
						m_eventMap[m_eventType].erase(m_eventMap[m_eventType].begin() + i);
						return;
					}
				}
			}

			InputEvent(Uint16 eventType, Reaction func)
				: m_eventType(eventType), m_event(func)
			{}
		};

		class KeyboardEvent : public InputEvent {
		public:
			virtual void push() override {
				m_keyEventMap[m_eventType].push_back(std::make_pair(m_event, m_ID));
			}

			KeyboardEvent(Uint16 key, Reaction reaction)
				: InputEvent(key, reaction) 
			{}
		};

		class CustomEvent : public Event {
		private:
			Condition m_condition;
			Reaction m_reaction;
		public:
			CustomEvent(Condition condition, Reaction reaction)
				: m_condition(condition), m_reaction(reaction) 
			{}

			virtual void push() override {
				m_customEventMap[m_ID] = std::make_pair(m_condition, m_reaction);
			}

			virtual void cancel() override {
				m_customEventMap.erase(m_ID);
			}
		};

		EventHandler() = delete;
	};

	export typedef EventHandler::InputEvent InputEvent; 

	export typedef EventHandler::KeyboardEvent KeyboardEvent;

	export typedef EventHandler::CustomEvent CustomEvent;

	export template<typename T>
	concept Event_C = std::is_base_of<EventHandler::Event, T>::value;

	export template<typename T>
	concept InputEvent_C = std::is_same<T, EventHandler::InputEvent>::value;

	export template<typename T>
	concept KeyboardEvent_C = std::is_same<T, EventHandler::KeyboardEvent>::value;

	export template<typename T>
	concept CustomEvent_C = std::is_same<T, EventHandler::CustomEvent>::value;
}
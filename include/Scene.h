#ifndef SCENE_H
#define SCENE_H

#include <print>
#include <tuple>

#include <nlohmann/json.hpp>

#include "data_util/Algorithms.h"
#include "data_util/Reflection.h"

#include "Button.h"
#include "ID.h"
#include "Event.h"
#include "Sprite.h"
#include "Text.h"
#include "Texture.h"

namespace nv {
	class Scene {
	private:
		void render();

		ObjectLayers<Sprite, Texture, Text, Rect, SpriteRef, TextureRef, TextRef, RectRef> m_objectLayers;
		
		SDL_Event m_SDLEvt{};

		template<typename Object>
		using IDObjects = std::vector<std::pair<Object, ID<Object>>>;

		template<typename Ret, typename... EventParams>
		using Events = IDObjects<Event<Ret, EventParams...>>;

		template<typename... EventParams>
		struct EventData {
			Events<void, EventParams...> events;
			Events<bool, EventParams...> cancellableEvents;

			std::vector<typename Events<bool, EventParams...>::iterator> cancelledEventIterators;
		};

		Keymap m_keyMap;
		const Uint8* m_keystate = SDL_GetKeyboardState(nullptr);

		std::tuple<EventData<>, EventData<MouseData>, EventData<const Keymap&>> m_eventData;

		IDObjects<TextInput> m_textInputs;
		TextInput* m_currEditedTextInput = nullptr;
		std::string m_textInputBuff;
		void selectTextInput();
		
		MouseData m_mouseData;

		void executeEvents();
	public:
		SDL_Renderer* renderer;
		FontMap& fontMap;
		TextureMap& texMap;

		bool running = false;

		Scene(std::string_view path, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap);

		template<typename Object>
		auto& find(this auto&& self, int layer, std::string_view name) 
			requires(RenderObject<std::unwrap_reference_t<Object>&>) 
		{
			decltype(auto) objs = std::get<std::vector<Object>>(self.m_objectLayers.at(layer));
			auto objIt = ranges::find_if(objs, [&](const auto& obj) { 
				return unrefwrap(obj).getName() == name;
			});
			/*if (objIt == objs.end()) {
				std::println("Could not find: {}", name);
				for (const auto& [currLayer, objs] : self.m_objectLayers) {
					std::print("{} ", currLayer);
					forEachDataMember([](const auto& objs) {
						std::print("\n\n{}: ", typeid(ValueType<decltype(objs)>).name());
						for (const auto& obj : objs) {
							std::print("{} ", unrefwrap(obj).getName());
						}
						std::println("");
						return STAY_IN_LOOP;
					}, objs);
				}
			}*/
			
			assert(objIt != objs.end());
		
			return *objIt;
		}

		template<typename Object>
		decltype(auto) addObject(Object&& object, int layer) 
			requires(RenderObject<std::unwrap_reference_t<Object&>>) 
		{
			decltype(auto) objects = std::get<std::vector<std::remove_cvref_t<Object>>>(m_objectLayers[layer]);
			objects.push_back(std::forward<Object>(object));
			return unrefwrap(objects.back());
		}

	private:
		template<typename Object, typename Objects, typename Transform>
		void eraseImpl(ID<Object> id, Objects& objects, Transform transform) {
			auto objIt = binaryFind(objects, id, transform);
			assert(objIt != objects.end());
			objects.erase(objIt);
		}
	public:
		template<typename EventType>
		void removeEvent(ID<EventType> id) {
			auto& eventData = std::get<EventData<EventType>>(m_eventData);
			
			eraseImpl(id, std::get<EventData<EventType>>(m_eventData).events, &std::pair<EventType, ID<EventType>>::second);
		}
		template<typename Object>
		void removeObject(ID<Object> id, int layer) {
			eraseImpl(id, std::get<std::vector<Object>>(m_objectLayers.at(layer)), &ObjectBase::getID);
		}
	
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
		ID<TextInput> addTextInput(nv::TextInput&& textInput);

		void operator()();

		void overlay(Scene& scene);
		void deoverlay();
	};
}

#endif
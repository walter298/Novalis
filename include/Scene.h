//#ifndef SCENE_H
//#define SCENE_H
//
//#include <print>
//#include <tuple>
//
//#include <plf_hive.h>
//
//#include <nlohmann/json.hpp>
//
//#include "data_util/Algorithms.h"
//#include "data_util/DataStructures.h"
//#include "data_util/Reflection.h"
//
//#include "Button.h"
//#include "ID.h"
//#include "Event.h"
//#include "Sprite.h"
//#include "Text.h"
//#include "Texture.h"
//
//namespace nv {
//	class Scene {
//	private:
//		void render();
//
//		//ONLY USED BY CUSTOM OBJECTS, SPRITES AND TEXTURES HAVE ZERO INDIRECTION!!!
//		struct TypeErasedBase {
//			virtual void render(SDL_Renderer* renderer) const noexcept = 0;
//			virtual const std::string& getName() const noexcept = 0;
//			virtual ~TypeErasedBase() noexcept {}
//		};
//		using TypeErasedBasePtr = std::unique_ptr<TypeErasedBase>;
//
//		struct TypeErasedMoveableBase : public virtual TypeErasedBase {
//			virtual void move(int, int) noexcept = 0;
//			virtual void move(SDL_Point) noexcept = 0;
//		};
//		using TypeErasedMoveableBasePtr = std::unique_ptr<TypeErasedMoveableBase>;
//
//		template<typename T, typename Base = TypeErasedBase>
//		struct TypeErasedDerived : public Base {
//			T obj;
//
//			template<typename U>
//			TypeErasedDerived(U&& u) : obj{ std::forward<U>(u) }
//			{
//			}
//
//			const std::string& getName() const noexcept override {
//				return unrefwrap(obj).name;
//			}
//
//			void render(SDL_Renderer* renderer) const noexcept override {
//				unrefwrap(obj).render(renderer);
//			}
//
//			~TypeErasedDerived() noexcept override {}
//		};
//
//		template<typename T>
//		struct TypeErasedMoveableDerived : public TypeErasedDerived<T, TypeErasedMoveableBase> {
//			using TypeErasedDerived<T, TypeErasedMoveableBase>::TypeErasedDerived;
//
//			void move(int dx, int dy) noexcept override {
//				unrefwrap(this->obj).move(dx, dy);
//			}
//			void move(SDL_Point p) noexcept override {
//				unrefwrap(this->obj).move(p);
//			}
//
//			~TypeErasedMoveableDerived() noexcept override = default;
//		};
//		using TypeErasedMoveableBasePtr = std::unique_ptr<TypeErasedMoveableBase>;
//
//		ObjectLayers<Sprite, Texture, Text, Rect, SpriteRef, TextureRef, TextRef, RectRef, 
//					 TypeErasedBasePtr, TypeErasedMoveableBasePtr, TypeErasedBase*, TypeErasedMoveableBase*> 
//			m_objectLayers;
//		
//		SDL_Event m_SDLEvt{};
//
//		template<typename Object>
//		using IDObjects = std::vector<std::pair<Object, ID<Object>>>;
//
//		template<typename Ret, typename... EventParams>
//		using Events = IDObjects<Event<Ret, EventParams...>>;
//
//		template<typename... EventArgs>
//		struct EventData {
//			Events<void, EventArgs...> events;
//			Events<bool, EventArgs...> cancellableEvents;
//
//			std::vector<typename Events<bool, EventArgs...>::iterator> cancelledEventIterators;
//		};
//
//		const Uint8* m_keystate = SDL_GetKeyboardState(nullptr);
//
//		std::tuple<EventData<>, EventData<MouseData>, EventData<const Uint8*>> m_eventData;
//
//		IDObjects<TextInput> m_textInputs;
//		TextInput* m_currEditedTextInput = nullptr;
//		std::string m_textInputBuff;
//		void selectTextInput();
//		
//		MouseData m_mouseData;
//
//		std::unordered_map<std::string, SDL_Point> m_specialPoints;
//
//		void executeEvents();
//	public:
//		SDL_Renderer* renderer;
//		FontMap& fontMap;
//		TextureMap& texMap;
//
//		bool running = false;
//		int FPS = 60;
//
//		Scene(std::string_view path, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap);
//
//		template<typename Object>
//		auto find(this auto&& self, int layer, std::string_view name)
//			requires(concepts::RenderableObject<std::unwrap_reference_t<Object>&>) 
//		{
//			decltype(auto) objs = std::get<plf::hive<Object>>(self.m_objectLayers.layers.at(layer));
//			auto objIt = ranges::find_if(objs, [&](const auto& obj) { 
//				return unrefwrap(obj).name == name;
//			});
//			if (objIt == objs.end()) {
//				std::println("Error: could not find {} at layer {}", name, layer);
//				self.printElements();
//				exit(-5555);
//			}
//			
//			return StableRef{ objs, objIt };
//		}
//
//		SDL_Point getSpecialPoint(std::string_view name) const noexcept;
//
//		/*template<typename Object>
//		auto addObject(Object&& object, int layer) {
//			decltype(auto) objects = std::get<plf::hive<std::remove_cvref_t<Object>>>(m_objectLayers.layers[layer]);
//			return StableRef{ objects, objects.insert(std::forward<Object>(object)) };
//		}
//		template<typename Object>
//		auto addCustomObject(Object&& object, int layer) {
//			if constexpr (MoveableObject<std::remove_cvref_t<Object>>) {
//				auto& objs = std::get<plf::hive<TypeErasedMoveableBasePtr>>(m_objectLayers.layers[layer]);
//				auto it = objs.insert(std::make_unique<TypeErasedMoveableDerived<Object>>(std::forward<Object>(object)));
//				std::println("{} custom moveable objects", objs.size());
//				return StableRef{ objs, it };
//			} else {
//				auto& objs = std::get<plf::hive<TypeErasedBasePtr>>(m_objectLayers.layers[layer]);
//				auto it = objs.insert(std::make_unique<TypeErasedDerived<Object>>(std::forward<Object>(object)));
//				std::println("{} custom non-moveable objects", objs.size());
//				return StableRef{ objs, it };
//			}
//		}*/
//	private:
//		template<typename Object, typename Objects, typename Transform>
//		void eraseImpl(ID<Object> id, Objects& objects, Transform transform) {
//			auto objIt = binaryFind(objects, id, transform);
//			assert(objIt != objects.end());
//			objects.erase(objIt);
//		}
//	public:
//		template<typename EventType>
//		void removeEvent(ID<EventType> id) {
//			using EventTypeTPs     = typename GetTemplateTypes<EventType>::Types; //will be of "one" type like void(int, int)
//			using EventInfo        = FunctionTraits<std::tuple_element_t<0, EventTypeTPs>>;
//			using EventArgs        = typename EventInfo::Args;
//			using EventRet         = typename EventInfo::Ret;
//			using MatchedEventData = typename GetParameterizedTypeFromTuple<EventData, EventArgs>::type;
//
//			auto& eventData = std::get<MatchedEventData>(m_eventData);
//			auto getID = &std::pair<EventType, ID<EventType>>::second;
//			if constexpr (std::same_as<EventRet, bool>) {
//				eraseImpl(id, eventData.cancellableEvents, getID);
//			} else {
//				eraseImpl(id, eventData.events, getID);
//			}
//		}
//
//		template<typename Func, typename Ret, typename... Args>
//		void addEventImpl(Func&& func) {
//
//		}
//
//		template<typename Func>
//		auto addEvent(Func&& func) {
//			using FuncInfo = FunctionTraits<std::decay_t<Func>>;
//			using FuncArgs = typename FuncInfo::Args;
//			using FuncRet  = typename FuncInfo::Ret;
//			using FuncSig  = typename FuncInfo::Sig;
//
//			using IDSpecialization = ID<typename GetParameterizedTypeFromTuple<Event, FuncSig>::type>;
//			IDSpecialization id;
//
//			using EventDataSpecialization = typename GetParameterizedTypeFromTuple<EventData, FuncArgs>::type;
//			auto& eventData = std::get<EventDataSpecialization>(m_eventData);
//			
//			if constexpr (std::same_as<FuncRet, bool>) {
//				eventData.cancellableEvents.emplace_back(std::forward<Func>(func), id);
//			} else {
//				eventData.events.emplace_back(std::forward<Func>(func), id);
//			}
//			return id;
//		}
//		
//		template<typename Func, typename Rep, typename Period>
//		auto addEvent(Func&& func, std::chrono::duration<Rep, Period> period) {
//			using FuncInfo = FunctionTraits<std::decay_t<Func>>;
//			using FuncArgs = typename FuncInfo::Args;
//			using FuncRet = typename FuncInfo::Ret;
//			using FuncSig = typename FuncInfo::Sig;
//
//			using IDSpecialization = ID<typename GetParameterizedTypeFromTuple<Event, FuncSig>::type>;
//			IDSpecialization id;
//
//			using namespace std::chrono;
//			using Duration = duration<Rep, Period>;
//
//			auto periodicEvtWrapper = [func = std::forward<Func>(func), 
//									   period = period,
//									   lastExecuted = system_clock::now()](auto... args) mutable 
//			{
//				auto now = system_clock::now();
//				
//				auto timeElapsed = duration_cast<Duration>(now - lastExecuted);
//				
//				if (timeElapsed < period) {
//					if constexpr (std::same_as<FuncRet, bool>) {
//						return false;
//					} else {
//						return;
//					}
//				}
//				lastExecuted = now;
//				auto timesToExecute = timeElapsed / period;
//
//				for (auto i : views::iota(0, timesToExecute)) {
//					if constexpr (std::same_as<FuncRet, bool>) {
//						if (func(args...)) {
//							return true;
//						}
//					} else {
//						func(args...);
//					}
//				}
//			};
//
//			using EventDataSpecialization = typename GetParameterizedTypeFromTuple<EventData, FuncArgs>::type;
//			auto& eventData = std::get<EventDataSpecialization>(m_eventData);
//
//			if constexpr (std::same_as<FuncRet, bool>) {
//				eventData.cancellableEvents.emplace_back(std::move(periodicEvtWrapper), id);
//			} else {
//				eventData.events.emplace_back(std::move(periodicEvtWrapper), id);
//			}
//			return id;
//		}
//		
//		ID<TextInput> addTextInput(nv::TextInput&& textInput);
//
//		void operator()();
//
//		void overlay(Scene& scene);
//		void deoverlay();
//
//		void printElements() const;
//	};
//}
//
//#endif
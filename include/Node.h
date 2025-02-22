//#pragma once
//
//#include <boost/smart_ptr/local_shared_ptr.hpp>
//#include <boost/unordered/unordered_flat_map.hpp>
//
//#include "data_util/Arena.h"
//#include "data_util/BufferViewAllocator.h"
//#include "data_util/DataStructures.h"
//#include "data_util/Reflection.h"
//#include "data_util/TypeErasure.h"
//#include "Sprite.h"
//#include "Text.h"
//
//namespace nv {
//	/*class BufferedNode;
//	class DynamicNode;
//	class Instance;*/
//
//	namespace detail {
//		template<template<typename> typename Allocator, template<typename> typename ObjectLayerStorage, typename... Objects>
//		class NodeBase {
//		private:
//			template<typename K, typename V>
//			using MapAllocator = Allocator<std::pair<const K, V>>;
//
//			using StringAllocator = Allocator<char>;
//
//			template<typename K, typename V>
//			using HashMap = boost::unordered_flat_map<
//				K, V, boost::hash<K>, std::equal_to<K>, MapAllocator<K, V>
//			>;
//
//			using String = std::basic_string<char, std::char_traits<char>, StringAllocator>;
//
//			template<typename... Ts>
//			using ObjectLookupMaps = std::tuple<HashMap<String, Ts*>...>;
//
//			using Layers = ObjectLayers<Allocator, ObjectLayerStorage,
//				Texture, 
//				Text, 
//				Rect
//				//type_erased::RenderableObject
//				//type_erased::MoveableObject
//				//BufferedNode, 
//				//DynamicNode
//			>;
//
//			/*void parse(json layer, Instance& instance) {
//				auto loadObjectLayers = [&](const json& layer, int layer) {
//					forEachDataMember([](const auto& objects) {
//						using ObjectType = typename std::remove_cvref_t<decltype(objects)>::value_type;
//						if constexpr (std::same_as<BufferedNode, ObjectType>) {
//							
//						}
//					}, m_objectLayers[layer]);
//				};
//
//				auto layers = json[LAYERS_KEY].array();
//				for (const auto& layer : layers) {
//
//				}
//			}*/
//		protected:
//			ObjectLookupMaps<Texture, Text, Rect> m_objectLookups;
//			HashMap<String, SDL_FPoint> m_specialPoints;
//			Layers m_objectLayers;
//			
//			template<typename... AllocatorConstructorArgs>
//			NodeBase(AllocatorConstructorArgs... args) 
//				: m_objectLookups{  
//					MapAllocator<String, Texture*> { args... },
//					MapAllocator<String, Text*> { args... }, 
//					MapAllocator<String, Rect*> { args... } 
//				},
//				m_specialPoints{ 
//					MapAllocator<String, SDL_Point>{ args... } 
//				},
//				m_objectLayers{ 
//					typename Layers::SpecializedAllocator{ args... } 
//				}
//			{
//			}
//		public:
//			void render(SDL_Renderer* renderer) {
//				m_objectLayers.forEach([&, this](int layer, const auto& obj) {
//					using Type = std::remove_cvref_t<decltype(obj)>;
//					if constexpr (IsClassTemplate<boost::local_shared_ptr, Type>::value || std::is_pointer_v<Type>) {
//						obj->render(renderer);
//					} else {
//						unrefwrap(obj).render(renderer);
//					}
//					return STAY_IN_LOOP;
//				});
//			}
//		};
//
//		using ArenaNodeBase = NodeBase<ArenaAllocator, std::span>;
//		using DynamicNodeBase = NodeBase<std::allocator, plf::hive>;
//
//		/*class StaticSubnode : public ArenaNodeBase {
//		protected:
//			StaticSubnode(Arena& arena) : ArenaNodeBase{ arena }
//			{
//			}
//		};*/
//	}
//
//	/*class BufferedNode : public detail::ArenaNodeBase {
//	private:
//		Arena m_arena;
//
//		BufferedNode(size_t size) : detail::ArenaNodeBase(m_arena), m_arena{ size }
//		{
//		}
//	public:
//		static BufferedNode load(const char* path);
//
//		friend class nlohmann::adl_serializer<BufferedNode>;
//	};*/
//
//	//using BufferedNodeMap = boost::unordered_flat_map<std::string, BufferedNode>;
//
//	/*class DynamicNode : public detail::DynamicNodeBase {
//
//	};*/
//
//	//using DynamicNodeMap = boost::unordered_flat_map<std::string, DynamicNode>;
//
//
//	/*class Node {
//	private:
//		boost::unordered_flat_map<std::string, SDL_FPoint> m_specialPoints;
//		ObjectLayers<Texture, Text, Rect, TextureRef, TextRef, RectRef,
//					 type_erased::RenderableObject, type_erased::MoveableObject, NodePtr, Node*
//		> m_objectLayers;
//
//		template<typename... Ts>
//		using NameMap = std::tuple<boost::unordered_flat_map<std::string, Ts*>...>;
//
//		NameMap<Texture, Text, Rect, Node> m_nameMap;
//	public:
//		Node(std::string_view path, Instance& instance);
//
//		void render(SDL_Renderer* renderer) const noexcept;
//
//		template<typename Object>
//		decltype(auto) addObject(Object&& object, int layer) {
//			decltype(auto) objects = std::get<plf::hive<std::remove_cvref_t<Object>>>(m_objectLayers.layers[layer]);
//			return *objects.insert(std::forward<Object>(object));
//		}
//
//		template<typename Object>
//		decltype(auto) addCustomObject(Object&& object, int layer) {
//			if constexpr (concepts::MoveableObject<std::remove_cvref_t<Object>>) {
//				auto& objs = std::get<plf::hive<type_erased::MoveableObject>>(m_objectLayers.layers[layer]);
//				return *objs.insert(std::forward<Object>(object));
//			} else {
//				auto& objs = std::get<plf::hive<type_erased::RenderableObject>>(m_objectLayers.layers[layer]);
//				return *objs.insert(std::forward<Object>(object));
//			}
//		}
//
//		template<typename Object>
//		decltype(auto) find(this auto&& self, std::string_view name) requires(concepts::RenderableObject<Object>) {
//			return std::get<boost::unordered_flat_map<std::string, Object>>(self.nameMap).at(name).get();
//		}
//
//		inline void move(SDL_FPoint change) noexcept {}
//		inline void screenMove(SDL_FPoint change) noexcept {}
//		inline void worldMove(SDL_FPoint change) noexcept {}
//
//		inline void setScreenPos(SDL_FPoint size) noexcept {}
//		inline void setWorldPos(SDL_FPoint size) noexcept {}
//		inline SDL_FPoint getScreenPos() const noexcept { return {}; }
//		inline SDL_FPoint getWorldPos() const noexcept { return {}; }
//
//		void setScreenSize(SDL_FPoint size) noexcept {};
//		void setWorldSize(SDL_FPoint size) noexcept {};
//		SDL_FPoint getScreenSize() const noexcept { return {}; };
//		SDL_FPoint getWorldSize() const noexcept { return {}; };
//
//		void setOpacity(uint8_t opacity) {}
//
//		bool containsCoord(SDL_FPoint p) const noexcept { return false; }
//	};*/
//}
//
////namespace nlohmann {
////	template<>
////	struct adl_serializer<nv::BufferedNode> {
////		SDL_Renderer* renderer = nullptr;
////		nv::Arena* currentArena = nullptr;
////		nv::Instance* globalInstance = nullptr;
////
////		static void to_json(json& j, const nv::BufferedNode& node) {
////			
////		}
////
////		static nv::BufferedNode from_json(const json& j) {
////			
////		}
////	};
////}
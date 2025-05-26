#pragma once

#include "../BasicConcepts.h"

#include "memory/MemoryRegion.h"
#include "reflection/ClassIteration.h"
#include "reflection/TypeInfo.h"
#include "reflection/TypeMap.h"
#include "../Polygon.h"
#include "../Texture.h"
#include "../Text.h"

namespace nv {
	class BufferedNode;

	namespace detail {
		template<typename Storage>
		struct ObjectLayers {
			Storage storage;

			decltype(auto) operator[](this auto&& self, size_t n) noexcept {
				return self.storage[n];
			}
			auto begin(this auto&& self) noexcept {
				return self.storage.begin();
			}
			auto end(this auto&& self) noexcept {
				return self.storage.end();
			}
			auto data(this auto&& self) noexcept {
				return self.storage.data();
			}
			size_t size() const noexcept {
				return storage.size();
			}
			template<typename Callable>
			void forEach(this auto&& self, Callable callable) {
				for (auto& layer : self.storage) {
					forEachDataMember([layer, &callable](auto& objHive) {
						for (auto& obj : objHive) {
							if (callable(layer, obj) == BREAK_FROM_LOOP) {
								return BREAK_FROM_LOOP;
							}
						}
						return STAY_IN_LOOP;
						}, layer);
				}
			}

			template<typename Callable>
			void forEach(this auto&& self, Callable callable, size_t layer) {
				forEachDataMember([layer, &callable](auto& objHive) {
					for (auto& obj : objHive) {
						if (callable(layer, obj) == BREAK_FROM_LOOP) {
							return BREAK_FROM_LOOP;
						}
					}
					return STAY_IN_LOOP;
					}, self.storage[layer]);
			}

			template<typename Callable>
			void forEachHive(this auto&& self, Callable callable) {
				for (auto& [layerIdx, objects] : std::views::enumerate(self.storage)) {
					forEachDataMember([layerIdx, &callable](auto& objHive) {
						if (callable(layerIdx, objHive) == BREAK_FROM_LOOP) {
							return BREAK_FROM_LOOP;
						}
						}, objects);
				}
			}

			template<typename Callable>
			void forEachHive(this auto&& self, Callable callable, size_t layerIdx) {
				forEachDataMember([layerIdx, &callable](auto& objHive) {
					return callable(layerIdx, objHive) == BREAK_FROM_LOOP;
					}, self.storage[layerIdx]);
			}

			template<typename... Us>
			ObjectLayers(Us&&... us) : storage{ std::forward<Us>(us)... }
			{
			}
		};

		template<typename T>
		concept NodeTraits = requires(T t) {
			typename T::Node;
			typename T::Polygon;
			typename T::Layer;
			typename T::Layers;
			typename T::LayerMap;
			typename T::ObjectLookups;
			typename T::String;
		};

		template<NodeTraits NodeTraits>
		class NodeBase : public NodeTraits {
		protected:
			typename NodeTraits::Layers m_objectLayers;
			typename NodeTraits::LayerMap m_layerMap;
			typename NodeTraits::ObjectLookups m_objectLookups;

			size_t m_currLayer = 0;
			float m_screenScale = 1.0f;
			float m_worldScale = 1.0f;
			uint8_t m_opacity = 255;
		public:
			template<typename T, typename StringComp>
			decltype(auto) find(this auto&& self, const StringComp& name) noexcept {
				return std::get<
					typename NodeTraits::template ObjectLookupMap<typename NodeTraits::String, T>
				>(self.m_objectLookups).at(name);
			}

			void render(SDL_Renderer* renderer) const noexcept {
				m_objectLayers.forEach([&, this](auto layer, const auto& obj) {
					unptrwrap(obj).render(renderer);
					return STAY_IN_LOOP;
				});
			}

			void screenMove(Point change) noexcept {
				m_objectLayers.forEach([&, this](auto layer, auto& obj) {
					unptrwrap(obj).screenMove(change);
					return STAY_IN_LOOP;
				});
			}

			void worldMove(Point change) noexcept {
				m_objectLayers.forEach([&, this](auto layer, auto& obj) {
					unptrwrap(obj).worldMove(change);
					return STAY_IN_LOOP;
				});
			}

			void setScreenPos(Point pos) noexcept {
				/*m_objectLayers.forEach([&, this](auto layer, auto& obj) {
					unptrwrap(obj).setScreenPos(pos);
					return STAY_IN_LOOP;
				});*/
			}

			void setWorldPos(Point pos) noexcept {
				/*m_objectLayers.forEach([&, this](auto layer, auto& obj) {
					unptrwrap(obj).setWorldPos(pos);
					return STAY_IN_LOOP;
				});*/
			}

			template<typename Func>
			void forEach(Func f) {
				m_objectLayers.forEach(f);
			}

			inline Point getScreenPos() const noexcept {
				return { 0.0f, 0.0f };
			}
			inline Point getWorldPos() const noexcept {
				return { 0.0f, 0.0f };
			}

			inline void setOpacity(uint8_t opacity) noexcept {
				m_opacity = opacity;
				m_objectLayers.forEach([&, this](auto layer, auto& obj) {
					unptrwrap(obj).setOpacity(opacity);
					return STAY_IN_LOOP;
					});
			}

			inline uint8_t getOpacity() const {
				return m_opacity;
			}

			bool containsScreenCoord(Point p) const noexcept {
				bool containsCoord = false;
				m_objectLayers.forEach([&](auto layer, const auto& obj) {
					if (unptrwrap(obj).containsScreenCoord(p)) {
						containsCoord = true;
						return BREAK_FROM_LOOP;
					}
					return STAY_IN_LOOP;
					});
				return containsCoord;
			}
			bool containsWorldCoord(Point p) const noexcept {
				bool containsCoord = false;
				m_objectLayers.forEach([&](auto layer, const auto& obj) {
					if (unptrwrap(obj).containsWorldCoord(p)) {
						containsCoord = true;
						return BREAK_FROM_LOOP;
					}
					return STAY_IN_LOOP;
					});
				return containsCoord;
			}

			void screenScale(float scale) noexcept {
				/*m_screenScale = scale;
				m_objectLayers.forEach([&](auto layer, auto& obj) {
					unptrwrap(obj).screenScale(scale);
					return STAY_IN_LOOP;
				});*/
			}
			void worldScale(float scale) noexcept {
				m_worldScale = scale;
				m_objectLayers.forEach([&](auto layer, auto& obj) {
					unptrwrap(obj).worldScale(scale);
					return STAY_IN_LOOP;
					});
			}
			float getScreenScale() const noexcept {
				return m_screenScale;
			}
			float getWorldScale() const noexcept {
				return m_worldScale;
			}
		};

		struct BufferedString {
			static constexpr const char* LEN_KEY = "Offset";

			char* buff = nullptr;
			size_t len = 0;

			inline bool operator==(const BufferedString& other) const noexcept {
				if (len != other.len) {
					return false;
				}
				return strncmp(buff, other.buff, len) == 0;
			}

			template<concepts::String String>
			inline bool operator==(const String& other) const noexcept {
				if (len != std::ranges::size(other)) {
					return false;
				}
				return strncmp(buff, other.data(), len) == 0;
			}

			template<size_t N>
			inline bool operator==(const char(&other)[N]) const noexcept {
				if (len != N) {
					return false;
				}
				return strncmp(buff, other, len) == 0;
			}
		};
	}
}
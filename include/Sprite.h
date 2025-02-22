//#pragma once
//
//#include <fstream>
//#include <string_view>
//#include <optional>
//
//#include <boost/unordered/unordered_flat_map.hpp>
//
//#include <SDL3_image/SDL_image.h>
//
//#include "data_util/Arena.h"
//#include "data_util/BasicConcepts.h"
//#include "data_util/SharedBuffer.h"
//#include "Collision.h"
//#include "Texture.h"
//
//namespace nv {
//	class Sprite {
//	private:
//		Arena m_arena;
//
//		struct TextureLayer {
//			Polygon hitbox;
//			std::vector<SDL_Texture*, ArenaAllocator<SDL_Texture*>> textures;
//		};
//
//		std::vector<TextureLayer, ArenaAllocator<TextureLayer>> m_layers;
//		size_t m_layerIdx = 0;
//
//		template<typename Func>
//		void forEachTexture(this auto&& self, Func f) {
//			for (auto& texLayer : self.m_layers) {
//				for (auto& tex : texLayer.textures) {
//					f(tex);
//				}
//			}
//		}
//	public:
//		inline void render(SDL_Renderer* renderer) const noexcept {
//			for (const auto& tex : m_layers[m_layerIdx].textures) {
//				tex.render(renderer);
//			}
//		}
//		inline void screenMove(SDL_FPoint p) noexcept {
//			forEachTexture([=](auto& tex) {
//				tex.screenMove(p);
//			});
//		}
//		inline void worldMove(SDL_FPoint p) {
//			for (auto& layer : m_layers) {
//				layer.hitbox.move(p);
//			}
//		}
//		inline void screenScale(float factor) noexcept {
//			forEachTexture([=](auto& tex) {
//				tex.scale(factor);
//			});
//		}
//		inline void worldScale(float factor) noexcept {
//			for (auto& layer : m_layers) {
//				layer.hitbox.scale(factor);
//			}
//		}
//	};
//
//	/*class Sprite {
//	private:
//		boost_con::flat_map<int, std::vector<Texture>> m_texObjLayers;
//
//		template<typename Animation>
//		using Animations = boost::unordered_flat_map<ID<Animation>, Animation>;
//
//		int m_currLayer = 0;
//	public:
//		Sprite(SDL_Renderer* renderer, const json& json, TextureMap& texMap);
//
//		using JsonFormat = boost_con::flat_map<int, std::vector<std::pair<std::string, TextureData>>>;
//
//		TextureData& getTexData(size_t texIdx);
//
//		inline auto& getHitbox(this auto&& self, size_t index = 0) {
//			return self.m_texObjLayers.at(m_currLayer)[index].getHitbox();
//		}
//
//		void setTextureLayer(int layer) noexcept;
//
//		const std::vector<nv::Texture>& getTextures() const noexcept;
//
//		inline SDL_FPoint getScreenPos() const {
//			return m_texObjLayers.at(m_currLayer)[0].getPos();
//		}
//
//		inline void screenMove(SDL_FPoint p) noexcept {
//			forEachTexture([=](auto& tex) {
//				tex.screenMove(p);
//			});
//		}
//
//		inline void screenScale(float factor) noexcept {
//			forEachTexture([=](auto& tex) {
//				tex.scale(factor);
//			});
//		}
//
//		void scale(SDL_FPoint p) noexcept;
//
//		void setRotationCenter() noexcept;
//
//		bool containsCoord(SDL_FPoint p) const noexcept;
//
//		void rotate(double angle, SDL_FPoint p);
//
//		template<typename Animation>
//		auto makeAnimation(Animation&& animation) {
//			using PlainAnimation = std::remove_cvref_t<Animation>;
//
//			ID<PlainAnimation> key;
//			auto& animations = std::get<Animations<PlainAnimation>>(m_animations);
//
//			animations.insert(std::pair{ key, std::forward<Animation>(animation) });
//			
//			return key;
//		}
//
//		template<typename Animation>
//		size_t animate(ID<Animation> id) {
//			auto& animations = std::get<Animations<Animation>>(m_animations);
//			auto frame = animations.at(id)();
//			setTextureLayer(frame);
//			return frame;
//		}
//
//		template<typename Animation>
//		void cancelAnimation(ID<Animation> id) {
//			auto& animations = std::get<Animations<Animation>>(m_animations);
//			animations.at(id).cancel();
//		}
//
//		void setOpacity(Uint8 opacity) noexcept;
//		void flip(SDL_RendererFlip flip) noexcept;
//
//		void render(SDL_Renderer* renderer) const noexcept;
//
//		void save(json& json) const;
//		static void saveJson(const Sprite& sprite, json& json);
//		
//		friend class editor::SceneEditor;
//		friend class editor::SpriteEditor;
//	};*/
//
//	//using SpriteRef = std::reference_wrapper<Sprite>;
//}
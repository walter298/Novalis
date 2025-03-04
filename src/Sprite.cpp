//#include "Sprite.h"
//
//#include <print>
//
//#include "data_util/BasicJsonSerialization.h"
//#include "data_util/File.h"
//
//nv::Sprite::Sprite(SDL_Renderer* renderer, const json& json, TextureMap& texMap) {
//	auto uniqueTexIndices = json["texture_object_layers"].get<JsonFormat>();
//	for (const auto& [layer, texObjs] : uniqueTexIndices) {
//		for (const auto& [texPath, texData] : texObjs) {
//			auto fullTexPath = relativePath(texPath);
//			auto texIt = texMap.find(fullTexPath);
//			if (texIt == texMap.end()) {
//				auto tex = IMG_LoadTexture(renderer, fullTexPath.c_str());
//				m_texObjLayers[layer].emplace_back(renderer, fullTexPath, tex, texData);
//				texMap.emplace(std::piecewise_construct, std::forward_as_tuple(fullTexPath), std::forward_as_tuple(tex, SDL_DestroyTexture));
//			} else {
//				m_texObjLayers[layer].emplace_back(renderer, fullTexPath, texIt->second.get(), texData);
//			}
//		}
//	}
//}
//
//nv::TextureData& nv::Sprite::getTexData(size_t texIdx) {
//	return m_texObjLayers[m_currLayer][texIdx].texData;
//}
//
//void nv::Sprite::setTextureLayer(int layer) noexcept {
//	m_currLayer = layer;
//}
//
//const std::vector<nv::Texture>& nv::Sprite::getTextures() const noexcept {
//	return m_texObjLayers.at(m_currLayer);
//}
//
//void nv::Sprite::setPos(SDL_FPoint p) noexcept {
//	for (auto& [layer, texObjs] : m_texObjLayers) {
//		auto [x, y] = texObjs[0].getPos();
//		SDL_FPoint change{ p.x - x, p.y - y };
//		for (auto& tex : texObjs) {
//			tex.move(change);
//		}
//	}
//}
//
//void nv::Sprite::move(SDL_FPoint change) noexcept {
//	for (const auto& [layer, textures] : m_texObjLayers) {
//		for (auto& texData : m_texObjLayers.at(layer)) {
//			texData.move(change);
//		}
//	}
//}
//
//void nv::Sprite::scale(SDL_FPoint p) noexcept {
//	for (auto& [layer, texData] : m_texObjLayers) {
//		for (auto& texDatum : texData) {
//			texDatum.scale(p);
//		}
//	}
//}
//
////don't know how the hell this will work
//void nv::Sprite::rotate(double angle, SDL_FPoint p) {}
//
//void nv::Sprite::setRotationCenter() noexcept {}
//
//bool nv::Sprite::containsCoord(SDL_FPoint p) const noexcept {
//	for (const auto& [idx, tex] : views::enumerate(m_texObjLayers.at(m_currLayer))) {
//		if (tex.containsCoord(p)) {
//			return true;
//		}
//	}
//	return false;
//}
//
//void nv::Sprite::setOpacity(Uint8 opacity) noexcept {
//	for (auto& [layer, texObjs] : m_texObjLayers) {
//		for (auto& texObj : texObjs) {
//			texObj.setOpacity(opacity);
//		}
//	}
//}
//
//void nv::Sprite::flip(SDL_RendererFlip flip) noexcept {
//	for (auto& [layer, texObjs] : m_texObjLayers) {
//		for (auto& texObj : texObjs) {
//			texObj.texData.flip = flip;
//		}
//	}
//}
//
//void nv::Sprite::render(SDL_Renderer* renderer) const noexcept {
//	for (const auto& texData : m_texObjLayers.at(m_currLayer)) {
//		texData.render(renderer);
//	}
//}
//
//void nv::Sprite::save(json& json) const {
//	Sprite::JsonFormat jsonFormat;
//	for (const auto& [layer, texObjs] : m_texObjLayers) {
//		for (const auto& texObj : texObjs) {
//			jsonFormat[layer].emplace_back(texObj.getTexPath(), texObj.texData);
//		}
//	}
//	json["texture_object_layers"] = std::move(jsonFormat);
//}
//
//void nv::Sprite::saveJson(const Sprite& sprite, json& json) {
//	Sprite::JsonFormat jsonFormat;
//	for (const auto& [layer, texObjs] : sprite.m_texObjLayers) {
//		for (const auto& texObj : texObjs) {
//			jsonFormat[layer].emplace_back(texObj.getTexPath(), texObj.texData);
//		}
//	}
//	json["texture_object_layers"] = std::move(jsonFormat);
//}

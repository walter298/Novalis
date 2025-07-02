#pragma once 

#include <SDL3/SDL_render.h>

#include "../Rect.h"

namespace nv {
	namespace detail {
		struct TextureRotationData {
			float angle = 0.0f;
			Point rotationPoint;
		};

		struct TextureRenderData {
			Rect ren;
			Rect world;
			TextureRotationData screenRotationData;
			TextureRotationData worldRotationData;
			SDL_FlipMode flip = SDL_FLIP_NONE;

			void screenScale(float newScale, SDL_FPoint refPoint = { 0, 0 }) noexcept {
				ren.scale(newScale);
			}
			void worldScale(float newScale, SDL_FPoint refPoint = { 0, 0 }) noexcept {
				world.scale(newScale);
			}
			float getScreenScale() const noexcept {
				return ren.currScale;
			}
			float getWorldScale() const noexcept {
				return world.currScale;
			}
			void screenMove(Point change) noexcept {
				ren.move(change);
			}
			void worldMove(Point change) noexcept {
				world.move(change);
			}
			void move(Point change) {
				screenMove(change);
				worldMove(change);
			}
			Point getScreenPos() const noexcept {
				return ren.getPos();
			}
			Point setScreenPos(Point p) noexcept {
				return ren.setPos(p);
			}
			Point getWorldPos() const noexcept {
				return world.getPos();
			}
			Point setWorldPos(Point p) noexcept {
				return world.setPos(p);
			}
			Point getWorldSize() const noexcept {
				return world.getSize();
			}
			Point setWorldSize(Point p) noexcept {
				return world.setSize(p);
			}
			Point getScreenSize() const noexcept {
				return ren.getSize();
			}
			Point setScreenSize(Point p) noexcept {
				return ren.setSize(p);
			}
			void setScreenRotation(float angle, Point rotationPoint) noexcept {
				screenRotationData = { angle, rotationPoint };
			}
			void setWorldRotation(float angle, Point rotationPoint) noexcept {
				worldRotationData = { angle, rotationPoint };
			}
			void setRotation(float angle, Point rotationPoint) noexcept {
				setScreenRotation(angle, rotationPoint);
				setWorldRotation(angle, rotationPoint);
			}
			bool containsScreenCoord(Point p) const noexcept {
				return ren.containsCoord(p);
			}
			bool containsWorldCoord(Point p) const noexcept {
				return world.containsCoord(p);
			}
			void resetWorld() noexcept {
				world = ren;
			}
		};
	}
}
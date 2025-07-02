#pragma once

#include <algorithm>
#include <concepts>
#include <cmath>

#include "Instance.h"
#include "Rect.h"

namespace nv {
	static bool approximatelyEqual(float a, float b, float error) noexcept {
		return std::fabs(a - b) < error;
	}

	struct CameraLock {
		Rect frame;
		Point objectCenter;
		bool screenLocked = false;

		CameraLock(Rect frame, Point objectCenter) noexcept : frame{ frame }, objectCenter{ objectCenter }
		{
		}

		template<typename Node, typename Object>
		void move(Node& node, Object& object, Point delta) noexcept {
			auto screenWidth = getGlobalInstance()->getScreenWidth();
			auto screenWidthMid = screenWidth / 2.0f;

			constexpr auto ERROR = 5.0f;

			if (approximatelyEqual(objectCenter.x, screenWidthMid, std::abs(delta.x))) {
				if (delta.x > 0.0f) {
					auto frameRightX = frame.pos.x + frame.size.x;
					auto distFromScreenUntilEndOfFrame = frameRightX - screenWidth;

					//only move the player if the screen would be over-scrolled
					if (distFromScreenUntilEndOfFrame < delta.x) {
						object.screenMove({ delta.x, 0.0f });
						objectCenter.x += delta.x;
					} else { //scroll with the screen
						node.screenMove({ -delta.x, 0.0f });
						object.screenMove({ delta.x, 0.0f });
						frame.pos.x -= delta.x;
					}
				} else {
					auto remainingLeftFrameSpace = frame.pos.x;
					if (remainingLeftFrameSpace > delta.x) {
						object.screenMove({ delta.x, 0.0f });
						objectCenter += delta.x;
					} else { //scroll with the screen
						node.screenMove({ -delta.x, 0.0f });
						object.screenMove({ delta.x, 0.0f });
						frame.pos.x -= delta.x;
					}
				}
			} else {
				object.screenMove({ delta.x, 0.0f });
				objectCenter.x += delta.x;
			}

			auto screenHeight = nv::getGlobalInstance()->getScreenHeight();
			auto screenHeightMid = screenHeight / 2.0f;
			if (approximatelyEqual(objectCenter.y, screenHeightMid, std::abs(delta.y))) {
				if (delta.y > 0.0f) {
					auto frameBottom = frame.pos.y + frame.size.y;
					auto distFromScreenUntilEndOfFrame = frameBottom - screenHeight;

					//only move the player if the screen would be over-scrolled
					if (distFromScreenUntilEndOfFrame < delta.x) {
						object.screenMove({ 0.0f, delta.y });
						objectCenter.y += delta.y;
					} else { //scroll with the screen
						node.screenMove({ 0.0f, -delta.y });
						object.screenMove({ 0.0f, delta.y });
						frame.pos.y -= delta.y;
					}
				} else {
					auto frameTop = frame.pos.y;
					if (frameTop > delta.x) {
						object.screenMove({ 0.0f, delta.y });
						objectCenter.y += delta.y;
					} else { //scroll with the screen
						node.screenMove({ 0.0f, -delta.y });
						object.screenMove({ 0.0f, delta.y });
						frame.pos.y -= delta.y;
					}
				}
			} else {
				object.screenMove({ 0.0f, delta.y });
				objectCenter.y += delta.y;
			}

			object.worldMove(delta);
		}
	};
}
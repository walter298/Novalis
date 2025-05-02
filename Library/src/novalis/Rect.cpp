#include "Rect.h"

#include "detail/serialization/AggregateSerialization.h"

bool nv::Rect::isInRegion(float mx, float my, float x, float y, float w, float h) noexcept {
	return mx > x && mx < x + w &&
		my > y && my < y + h;
}

bool nv::Rect::isInRegion(Point coord, float x, float y, float w, float h) noexcept {
	return isInRegion(coord.x, coord.y, x, y, w, h);
}

bool nv::Rect::containsCoord(Point p) const noexcept {
	return isInRegion(p, pos.x, pos.y, size.x, size.y);
}

nv::Point nv::Rect::move(float dx, float dy) noexcept {
	pos.x += dx;
	pos.y += dy;
	return getPos();
}
nv::Point nv::Rect::move(Point p) noexcept {
	move(p.x, p.y);
	return getPos();
}
nv::Point nv::Rect::setPos(float x, float y) noexcept {
	pos.x = x;
	pos.y = y;
	return getPos();
}
nv::Point nv::Rect::setPos(Point p) noexcept {
	return setPos(p.x, p.y);
}
nv::Point nv::Rect::getPos() const noexcept {
	return pos;
}
nv::Point nv::Rect::setSize(float w, float h) noexcept {
	size.x = w;
	size.y = h;
	return getPos();
}
nv::Point nv::Rect::setSize(Point p) noexcept {
	return setSize(p.x, p.y);
}

nv::Point nv::Rect::getSize() const noexcept {
	return size;
}
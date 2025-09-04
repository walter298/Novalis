#pragma once

#include <novalis/BufferedNode.h>
#include <novalis/EventHandler.h>

namespace rm {
	namespace player {
		extern void setPlayerMovement(nv::EventHandler& evtHandler, nv::BufferedNode& root);
	}
}
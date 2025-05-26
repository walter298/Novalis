#pragma once

#include <atomic>
#include <boost/asio.hpp>

#include <novalis/Rect.h>

#include "WindowLayout.h"

namespace nv {
	namespace editor {
		namespace {
			boost::asio::io_context context;
			std::atomic_bool isWorkRunning = false;
		}

		void startAsyncWork() {
			auto workGuard = boost::asio::make_work_guard(context);
			context.run();
		}

		template<std::invocable Func>
		static void schedule(Func&& func) {
			boost::asio::post(context, [func = std::forward<Func>(func)]() mutable {
				isWorkRunning.store(true);
				func();
				isWorkRunning.store(false);
			});
		}

		bool isAsyncWorkRunning() {
			return isWorkRunning.load();
		}

		void stopAsyncWork() {
			context.stop();
		}
	}
}
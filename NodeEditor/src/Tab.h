#pragma once

#include "File.h"

namespace nv {
	namespace editor {
		struct Tab {
			FileID id;
			File::Type type{};
		};
	}
}

namespace boost {
	template<>
	struct hash<nv::editor::Tab> {
		size_t operator()(const nv::editor::Tab& t) const noexcept {
			return boost::hash<nv::editor::FileID>{}(t.id);
		}
	};
}
namespace std {
	template<>
	struct hash<nv::editor::Tab> {
		size_t operator()(const nv::editor::Tab& t) const noexcept {
			return std::hash<nv::editor::FileID>{}(t.id);
		}
	};
}
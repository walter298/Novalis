#pragma once

#include <string>
#include <novalis/ID.h>

namespace nv {
	namespace editor {
		namespace detail { 
			struct FileTag {}; 
			struct DirectoryTag {}; 
		}
		using FileID      = ID<detail::FileTag>;
		using DirectoryID = ID<detail::DirectoryTag>;
	}
}
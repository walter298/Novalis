#pragma once

#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <novalis/BufferedNode.h>

#include "imgui/imgui.h"
#include "FileID.h"
#include "ImGuiID.h"
#include "NameManager.h"

namespace nv {
	namespace editor {
		struct File;

		using FileMap = std::unordered_map<FileID, File>;
		using FileSet = std::unordered_set<FileID>;

		struct File {
			std::string name;
			FileSet dependencies;
			FileSet dependants;
			DirectoryID parent;
			int imguiID = getPermanentImGuiID();
			ImTextureID icon = 0;
			enum Type { //make type values powers of two so that we can have correct bitwise ops
				Image = 1,
				Font = 2,
				Node = 4
			} type{};
			std::filesystem::path realPath;
		};

		inline constexpr ImVec2 FILE_ICON_SIZE{ 35.0f, 40.0f };

		struct Directory;

		using DirectoryMap = std::unordered_map<DirectoryID, Directory>;
		using DirectorySet = std::unordered_set<DirectoryID>;

		struct Directory {
			NameManager nameManager{ "file" };
			std::string name;
			FileSet files;
			DirectorySet children;
			DirectoryID parent;
			bool open = false;
			int imguiID = getPermanentImGuiID();
		};

		inline constexpr ImVec2 DIRECTORY_ICON_SIZE{ 40.0f, 40.0f };
	}
}
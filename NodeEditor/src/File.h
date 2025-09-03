#pragma once

#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <novalis/BufferedNode.h>
#include <novalis/detail/reflection/ClassIteration.h>

#include "imgui/imgui.h"
#include "FileID.h"
#include "ImGuiID.h"
#include "NameManager.h"

namespace nv {
	namespace editor {
		void loadFileIcons(SDL_Renderer* renderer);
		void destroyFileIcons(SDL_Renderer* renderer);
		
		struct File;

		using FileMap = std::unordered_map<FileID, File>;
		using FileSet = std::unordered_set<FileID>;

		class File {
		public:
			enum Type { //make type values powers of two so that we can have correct bitwise ops
				None = 0,
				Font = 2,
				Node = 4,
				JPG = 8,
				PNG = 16,
				AVIF = 32,
				BMP = 64,
				Image = JPG | PNG | AVIF | BMP
			};
		private:
			ImTextureID m_icon{};
			int m_imguiID = getPermanentImGuiID();
			Type m_type{};
			std::string m_name;
		public:
			File() = default;
			File(NameManager& parentNameManager, std::string name, File::Type type);
			File(NameManager& parentNameManager, File::Type type);
			
			FileSet dependencies;
			FileSet dependants;
			DirectoryID parent;

			void show() noexcept;
			void show(NameManager& dirNameManager, bool& finishedRenaming) noexcept;
			void makeNameUnique(NameManager& parentNameManager);
			const std::string& getName() const noexcept;
			std::string parseExtension() const;
			Type getType() const noexcept;
			ImTextureID getIcon() const noexcept;

			MAKE_INTROSPECTION(m_type, m_name, dependencies, dependants, parent)

			friend void to_json(nlohmann::json& j, const File& file);
			friend void from_json(const nlohmann::json& j, File& file);
		};

		void to_json(nlohmann::json& j, const File& file);
		void from_json(const nlohmann::json& j, File& file);

		inline constexpr ImVec2 FILE_ICON_SIZE{ 35.0f, 40.0f };

		struct Directory;

		using DirectoryMap = std::unordered_map<DirectoryID, Directory>;
		using DirectorySet = boost::unordered_flat_set<DirectoryID>;

		class Directory {
		private:
			std::string m_name;
			int m_imguiID = getPermanentImGuiID();
		public:
			static Directory makeRoot();

			Directory() = default;
			Directory(NameManager& parentNameManager, std::string name);
			Directory(NameManager& parentNameManager);

			NameManager nameManager{ "file" };
			FileSet files;
			DirectorySet children;
			DirectoryID parent;
			bool open = false;

			void showIcon() const noexcept;
			void inputName(NameManager& parentNameManager, bool& finishedRenaming);
			void makeNameUnique(NameManager& parentNameManager);
			const std::string& getName() const noexcept;
			int getImGuiID();

			MAKE_INTROSPECTION(m_name, m_imguiID, nameManager, files, children, parent, open)
		};

		ImTextureID getFolderIcon() noexcept;

		inline constexpr ImVec2 DIRECTORY_ICON_SIZE{ 40.0f, 40.0f };
	}
}
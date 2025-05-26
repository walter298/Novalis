#pragma once

#include <novalis/detail/ScopeExit.h>
#include "NodeTabList.h"

namespace nv {
	namespace editor {
		class TaskBar {
		private:
			static std::optional<EditedObjectData<BufferedNode>> uploadNode() noexcept {
				auto filePath = openFile({ { "node", "nv_node" } });
				if (!filePath) {
					return std::nullopt;
				}
				std::ifstream file{ filePath->c_str() };
				if (!file.is_open()) {
					std::println(stderr, "Error: could not open {}", *filePath);
					return std::nullopt;
				}
				try {
					auto nodeJson = json::parse(file);
					EditedObjectData<BufferedNode> ret{ nodeJson.get<BufferedNode>() };
					ret.name = fileName(*filePath);
					ret.filePath = *filePath;
					return ret;
				} catch (std::exception e) {
					std::println(stderr, "{}", e.what());
					return std::nullopt;
				}
			}

			bool m_showingWindow = false;

			class ObjectDropdown {
			private:
				enum class ObjectLayout {
					Grid,
					Stack,
					Row,
					None
				};
				ObjectLayout m_currentlyChosenLayout = ObjectLayout::None;
				std::string m_layoutName = "No layout chosen";
				bool m_showingLayoutPopup = false;
				std::variant<
					std::vector<EditedObjectData<Texture>>//,
					//std::vector<EditedObjectData<Node>>
				> m_currentlyLoadedObjects;

				using Textures = std::vector<EditedObjectData<Texture>>;

				std::optional<Textures> uploadTextures(SDL_Renderer* renderer) {
					auto texPaths = openMultipleFiles({ { "images", "png" } });
					if (!texPaths) {
						return std::nullopt;
					}

					Textures textures;

					for (const auto& texPath : *texPaths) {
						TexturePtr tex{ renderer, texPath.c_str() };
						if (tex.tex == nullptr) {
							std::println(stderr, "{}", SDL_GetError());
							continue;
						}
						auto& editedTex = textures.emplace_back(std::move(tex));
						editedTex.name = fileName(texPath);
						editedTex.texPath = texPath;
					}

					return textures;
				}

				//returns true if textures were loaded
				bool createTexturesFromImages(SDL_Renderer* renderer, NodeEditor& currTab) {
					auto texturesRet = uploadTextures(renderer);
					if (!texturesRet) {
						return false;
					}

					auto& textures = *texturesRet;

					if (textures.size() > 1) {
						std::get<Textures>(m_currentlyLoadedObjects) = std::move(textures);
						m_showingLayoutPopup = true;
					} else {
						currTab.transfer(std::move(textures.front()));
					}
					return true;
				}

				void uploadSpriteSheet(SDL_Renderer* renderer, NodeEditor& currTab) {
					auto texturesRes = uploadTextures(renderer);
					if (!texturesRes) {
						return;
					}
					currTab.createSpritesheet(renderer, std::move(*texturesRes));
				}

				void insertNodeFromFile(NodeEditor& currTab) {
					auto node = uploadNode();
					if (node) {
						currTab.transfer(std::move(*node));
					}
				}

				void setGridLayout() {
					std::visit([](auto& objects) {
						auto maxHeight = std::ranges::max(objects, [](const auto& a, const auto& b) {
							return a.obj.getWorldSize().y > b.obj.getWorldSize().y;
						}).obj.getWorldSize().x;

						auto columnCount = static_cast<int>(objects.size());
						auto x = 0.0f;
						auto y = 0.0f;
						for (auto& obj : objects) {
							obj.obj.setScreenPos({ x, y });
							if (x + 1 == columnCount) {
								x = 0;
								y += maxHeight;
							} else {
								x += obj.obj.getScreenSize().x;
							}
						}
					}, m_currentlyLoadedObjects);
				}

				void showLayoutOption() {
					std::visit([this](auto&& objects) {
						ImGui::Begin("Choose Layout");
						nv::detail::ScopeExit imGuiEnd{ [] { ImGui::End(); } };

						if (objects.size() % 2 == 0 && ImGui::Button("Grid")) {
							m_currentlyChosenLayout = ObjectLayout::Grid;
							m_layoutName = "Current layout: grid";
							//setGridLayout(objects);
						}
						if (ImGui::Button("Stack")) {
							m_currentlyChosenLayout = ObjectLayout::Stack;
							m_layoutName = "Current layout: stack";
						}
						if (ImGui::Button("Row")) {
							m_currentlyChosenLayout = ObjectLayout::Row;
							m_layoutName = "Current layout: row";
						}

						ImGui::Text(m_layoutName.c_str());

						ImGui::SetNextItemWidth(getInputWidth());
						ImGui::BeginDisabled(m_currentlyChosenLayout == ObjectLayout::None);
						nv::detail::ScopeExit endDisabled{ [] { ImGui::EndDisabled(); } };
						if (ImGui::Button("Set")) {
							m_currentlyChosenLayout = ObjectLayout::None;
							m_layoutName = "No layout chosen";
							m_showingLayoutPopup = false;
						}
					}, m_currentlyLoadedObjects);
				}
			public:
				void show(SDL_Renderer* renderer, NodeTabList& tabs) {
					if (!tabs.currentTab() || tabs.currentTab()->hasNoLayers()) {
						showDisabledMenu("Object");
						return;
					}

					if (m_showingLayoutPopup) {
						showLayoutOption();
						return;
					}

					if (ImGui::BeginMenu("Object")) {
						if (ImGui::MenuItem("Create Textures From Images")) {
							createTexturesFromImages(renderer, *tabs.currentTab());
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Upload Sprite Sheet")) {
							uploadSpriteSheet(renderer, *tabs.currentTab());
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Create Text")) {
							//todo
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Create Polygon")) {
							//todo
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Upload Node(s)")) {
							insertNodeFromFile(*tabs.currentTab());
						}
						ImGui::EndMenu();
					}
				}
			};

			class LayerDropdown {
			private:
				std::string m_layerName;
				bool m_showingAddNewLayerPopup = false;

				void showLayerCreationWindow(NodeEditor& currTab) {
					m_showingAddNewLayerPopup = true;

					auto center = ImGui::GetMainViewport()->GetCenter();
					ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });

					ImGui::OpenPopup(LAYER_CREATION_POPUP_NAME);
					ImGui::BeginPopupContextWindow(LAYER_CREATION_POPUP_NAME);

					centerNextText("Create Layer");
					ImGui::Text("Create Layer");

					ImGui::InputText("Layer Name", &m_layerName);

					centerNextText("Create");
					if (ImGui::Button("Create")) {
						currTab.addLayer(m_layerName);
						m_showingAddNewLayerPopup = false;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			public:
				void show(NodeTabList& tabs) {
					if (tabs.empty()) {
						showDisabledMenu("Layer");
						return;
					}

					if (m_showingAddNewLayerPopup) {
						showLayerCreationWindow(*tabs.currentTab());
					}

					if (ImGui::BeginMenu("Layer")) {
						if (ImGui::MenuItem("New Layer")) {
							m_showingAddNewLayerPopup = true;
						}
						if (ImGui::MenuItem("Delete Layer")) {

						}
						if (ImGui::MenuItem("Duplicate Layer")) {

						}
						ImGui::EndMenu();
					}
				}
			};

			class FileDropdown {
			private:
				std::string m_nodeNameInput;
				bool m_showingNodeCreationPopupWindow = false;

				void showNodeCreationPopupWindow(NodeTabList& tabs, bool& showingNodeCreationPopupWindow) {
					auto center = ImGui::GetMainViewport()->GetCenter();
					ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					ImGui::OpenPopup(NODE_CREATION_POPUP_NAME);
					ImGui::BeginPopupContextWindow(NODE_CREATION_POPUP_NAME);

					centerNextText("Create Node");
					ImGui::Text("Create Node");

					ImGui::InputText("Node Name", &m_nodeNameInput);

					centerNextText("Create");
					if (ImGui::Button("Create")) {
						showingNodeCreationPopupWindow = false;
						tabs.add(m_nodeNameInput);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			public:
				void show(NodeTabList& tabs) {
					if (m_showingNodeCreationPopupWindow) {
						showNodeCreationPopupWindow(tabs, m_showingNodeCreationPopupWindow);
					}

					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("New Node")) {
							m_showingNodeCreationPopupWindow = true;
						}
						if (ImGui::MenuItem("Open Node")) {
							tabs.upload();
						}
						ImGui::Separator();
						ImGui::BeginDisabled(!tabs.currentTab().has_value());
						if (ImGui::MenuItem("Save")) {
							tabs.currentTab()->save();
						}
						if (ImGui::MenuItem("Save as")) {
							tabs.currentTab()->saveAs();
						}
						ImGui::EndDisabled();
						ImGui::EndMenu();
					}
				}
			};

			ObjectDropdown m_objectLoader;
			LayerDropdown m_layerCreator;
			FileDropdown m_fileDropdown;
		public:
			static constexpr const char* NODE_CREATION_POPUP_NAME = "Create Node";
			static constexpr const char* LAYER_CREATION_POPUP_NAME = "Create Layer";

			void show(SDL_Renderer* renderer, NodeTabList& tabs) {
				ImGui::BeginMainMenuBar();
				m_fileDropdown.show(tabs);
				m_layerCreator.show(tabs);
				m_objectLoader.show(renderer, tabs);
				ImGui::EndMainMenuBar();
			};
		};
	}
}
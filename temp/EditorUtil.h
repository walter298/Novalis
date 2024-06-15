#ifndef EDITOR_UTIL_H
#define EDITOR_UTIL_H

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <thread> //sleep

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <Windows.h>
#include <ShlObj.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <hash_table7.hpp>

#include "Instance.h"
#include "ID.h"

namespace nv {
	namespace editor {
		enum class EditorDest {
			None,
			Quit,
			Scene,
			Sprite,
			Text,
			Home
		};

		template<typename Func>
		struct ScopeExit {
		private:
			Func m_f;
		public:
			ScopeExit(const Func& f) noexcept(std::is_nothrow_copy_constructible_v<Func>) : m_f{ f } {}
			ScopeExit(Func&& f) noexcept(std::is_nothrow_move_constructible_v<Func>) : m_f{ std::move(f) } {}
			~ScopeExit() noexcept(std::is_nothrow_invocable_v<Func>) {
				m_f();
			}
		};

		template<typename RenderMethod, typename... Events>
		EditorDest runEditor(ImGuiIO& io, Renderer& renderer, RenderMethod& showGui)
			requires std::invocable<RenderMethod, Renderer&> &&
		std::same_as<std::invoke_result_t<RenderMethod, Renderer&>, EditorDest>
		{
			while (true) {
				constexpr auto waitTime = 1000ms / 180;
				const auto endTime = chrono::system_clock::now() + waitTime;

				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					ImGui_ImplSDL2_ProcessEvent(&evt);
					if (evt.type == SDL_QUIT) {
						return EditorDest::Quit;
					}
					else if (evt.type == SDL_KEYDOWN) {
						if (evt.key.keysym.scancode == SDL_SCANCODE_MINUS) {
							renderer.clear();
							return EditorDest::Home;
						}
					}
				}

				ImGui_ImplSDLRenderer2_NewFrame();
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();

				auto dest = showGui(renderer);

				const auto now = chrono::system_clock::now();

				//checks frames, render
				if (now < endTime) {
					std::this_thread::sleep_for(endTime - now);
				}

				renderer.renderWithImGui(io);

				if (dest != EditorDest::None) {
					return dest;
				}
			}
		}

		void runEditors();

		std::optional<std::string> openFilePath();
		std::optional<std::vector<std::string>> openFilePaths();
		std::optional<std::string> saveFile(std::wstring openMessage);

		void loadImages(std::vector<std::string>& imagePaths, plf::hive<Texture>& textures, Renderer& renderer);

		template<typename T>
		constexpr auto centerPos(T l1, T l2) {
			return (l1 - l2) / 2;
		}

		constexpr ImVec2 adjacentPos(const ImVec2& pos, const ImVec2& size, float spacing = 0.0f) {
			return ImVec2{ pos.x + size.x + spacing,  pos.y };
		}
		constexpr ImVec2 buttonList(const ImVec2& btnSize, int btnC) noexcept {
			return {
				btnSize.x + 15,
				btnSize.y * static_cast<float>(btnC) + 40.0f
			};
		}

		//used to convert std::pair<int, int> <----> ImVec2
		template<typename Ret, typename Pair>
		Ret convertPair(const Pair& pair) noexcept {
			using Converted = std::remove_cvref_t<decltype(Ret::x)>;
			return Ret{ static_cast<Converted>(pair.x), static_cast<Converted>(pair.y) };
		}

		template<RenderObject... Objects>
		class ObjectEditor {
		private:
			Renderer& m_renderer;
			
			ImVec2 m_objOptionsPos;

			template<RenderObject Object>
			struct ObjectHiveData {
				using HiveType = plf::hive<Object>;
				HiveType* objHive = nullptr;
				HiveType::iterator selectedIt;
				int layer = 0;
				bool isSelected = false;
			};
			
			using ObjectHives = std::tuple<ObjectHiveData<Objects>...>;
			ObjectHives m_objHiveData;

			int m_scale = 0;
			float m_angle = 0.0; //should be double, but ImGui frustratingly only supports InputFloat
			SDL_Point m_rotationPoint{ 0, 0 };

			bool m_editingObj = false;

			template<RenderObject Obj>
			void edit(ObjectHiveData<Obj>& objHiveData, SDL_Point mousePos) {
				auto& [objs, selectedObjIt, layer, isSelected] = objHiveData;
				auto& obj = *selectedObjIt;

				if (obj.containsCoord(mousePos)) {
					auto mouseChange = convertPair<SDL_Point>(ImGui::GetMouseDragDelta());
					obj.move(mouseChange);
					ImGui::ResetMouseDragDelta();
				}

				ImGui::SetNextWindowPos(m_objOptionsPos);
				ImGui::SetNextWindowSize({ 300, 200 });
				ImGui::Begin("Object");

				//scaling texture
				int oldScale = m_scale;
				if (ImGui::SliderInt("Scale", &m_scale, 0, 1500)) {
					int deltaScale = m_scale - oldScale;
					obj.scale(deltaScale, deltaScale);
				}
				
				ImGui::Text("Rotation");
				if (ImGui::SliderFloat("Angle", &m_angle, 0.0f, 360.0f)) {
					obj.rotate(static_cast<double>(m_angle), m_rotationPoint);
				}
				if (ImGui::InputInt("x", &m_rotationPoint.x)) {
					obj.rotate(static_cast<double>(m_angle), m_rotationPoint);
				}
				if (ImGui::InputInt("y", &m_rotationPoint.y)) {
					obj.rotate(static_cast<double>(m_angle), m_rotationPoint);
				}
				
				if (ImGui::Button("Delete")) {
					objs->erase(selectedObjIt);
					m_renderer.erase(&obj, layer);
					selectedObjIt = objs->end();
					isSelected = false;
				}

				if (ImGui::Button("Duplicate")) {
					objs->insert(obj);
					auto insertedObj = &getBack(*objs);
					m_renderer.add(insertedObj, layer);
					insertedObj->setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
				}

				ImGui::End();
			}

			/*search through a hive of objects, and if one is hovered over, update the
			iterator corresponding to the hive*/
			template<RenderObject Object>
			bool selectObj(ObjectHiveData<Object>& objHiveData, SDL_Point mousePos) {
				auto& [objs, it, layer, isSelected] = objHiveData;
				if (objs == nullptr) {
					return STAY_IN_LOOP;
				}
				auto selectedObjIt = ranges::find_if(*objs, [&](const auto& obj) {
					return obj.containsCoord(mousePos);
				});
				if (selectedObjIt != objs->end()) {
					it = selectedObjIt;
					isSelected = true;
					return BREAK_FROM_LOOP;
				}
				return STAY_IN_LOOP;
			}
		public:
			ObjectEditor(Renderer& renderer, ImVec2 optionsPos) 
				: m_objOptionsPos{ optionsPos }, m_renderer { renderer }
			{
				iterateStructs([this](auto& objHiveData) {
					objHiveData = { nullptr, {}, 0, false };
					return STAY_IN_LOOP;
				}, m_objHiveData);
			}

			~ObjectEditor() {
				m_renderer.clear();
			}

			void operator()() {
				auto mousePos = convertPair<SDL_Point>(ImGui::GetMousePos());
				iterateStructs([&, this](auto& objHiveData) {
					auto& [objs, selectedObjIt, layer, isSelected] = objHiveData;
					if (objs == nullptr) {
						return STAY_IN_LOOP;
					}
					if (!isSelected) {
						return STAY_IN_LOOP;
					}
					edit(objHiveData, mousePos);
					return BREAK_FROM_LOOP;
				}, m_objHiveData);
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) { //if we have a new clicked 
					iterateStructs([&](auto& objHiveData) {
						return selectObj(objHiveData, mousePos);
					}, m_objHiveData);
				}
			}
			
			template<RenderObject Object>
			void reseat(plf::hive<Object>* objs, int layer) {
				std::get<ObjectHiveData<Object>>(m_objHiveData) = { objs, objs->end(), layer, false };
			}
		};

		struct TextureDataAndPath : public TextureObject {
			TextureDataAndPath(std::string_view path, SDL_Texture* tex, TextureData texData);
			TextureDataAndPath(std::string_view path, TexturePtr texPtr, TextureData texData);
			std::string path;
		};

		template<RenderObject Object>
		void makeOneLayerMoreVisible(Layers<Object>& objLayers, int visibleLayer, Uint8 reducedOpacity) {
			auto reduceOpacity = [&](auto range) {
				for (auto& [layer, objs] : range) {
					for (auto& obj : objs) {
						obj.setOpacity(reducedOpacity);
					}
				}
			};
			auto visibleLayerIt = objLayers.find(visibleLayer);
			auto beforeVisibleLayer = ranges::subrange(objLayers.begin(), visibleLayerIt);
			auto afterVisibleLayer  = ranges::subrange(ranges::next(visibleLayerIt), objLayers.end());
			reduceOpacity(beforeVisibleLayer); 
			reduceOpacity(afterVisibleLayer);

			//set to full opacity in case it was already reduced
			for (auto& obj : objLayers.at(visibleLayer)) {
				obj.setOpacity(255);
			}
		}
	}
}

#endif
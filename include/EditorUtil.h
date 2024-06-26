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
#include "Renderer.h"
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

		template<typename T>
		concept Editor = requires(T t) { 
			{ t.imguiRender() } -> std::same_as<EditorDest>;
			t.sdlRender(); 
		};

		template<Editor RenderMethod>
		EditorDest runEditor(ImGuiIO& io, SDL_Renderer* renderer, RenderMethod& editor) {
			while (true) {
				constexpr auto waitTime = 1000ms / 180;
				const auto endTime = chrono::system_clock::now() + waitTime;

				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					ImGui_ImplSDL2_ProcessEvent(&evt);
					if (evt.type == SDL_QUIT) {
						return EditorDest::Quit;
					} else if (evt.type == SDL_KEYDOWN) {
						if (evt.key.keysym.scancode == SDL_SCANCODE_MINUS) {
							return EditorDest::Home;
						}
					}
				}

				ImGui_ImplSDLRenderer2_NewFrame();
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();

				auto dest = editor.imguiRender();
				
				static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };
				ImGui::Render();
				SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
				SDL_SetRenderDrawColor(renderer,
					//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
					static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
					static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
				
				SDL_RenderClear(renderer);
				ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
				editor.sdlRender();
				SDL_RenderPresent(renderer);

				if (dest != EditorDest::None) {
					return dest;
				}
			}
		}

		void runEditors();

		std::optional<std::string> openFilePath();
		std::optional<std::vector<std::string>> openFilePaths();
		std::optional<std::string> saveFile(std::wstring openMessage);

		/*void loadImages(std::vector<std::string>& imagePaths, plf::hive<Texture>& textures, Renderer& renderer);*/

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
			ImVec2 m_objOptionsPos;

			template<RenderObject Object>
			struct ObjectLayersData {
				std::vector<Object>* objHive;
				std::vector<Object>::iterator selectedObjIt;
				size_t layer = 0;
				bool isSelected = false;
			};
			
			using ObjectLayers = std::tuple<ObjectLayersData<Objects>...>;
			ObjectLayers m_objLayers;

			int m_w = 0;
			int m_h = 0;
			int m_scale = 0;
			float m_angle = 0.0; //should be double, but ImGui frustratingly only supports InputFloat
			SDL_Point m_rotationPoint{ 0, 0 };

			bool m_editingObj = false;

			template<RenderObject Object>
			void edit(ObjectLayersData<Object>& objHiveData, SDL_Point mousePos) {
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

				ImGui::Text("Size");

				//setting size
				if constexpr (SizeableObject<Object>) {
					if (ImGui::InputInt("width", &m_w)) {
						obj.setSize(m_w, m_h);
					}
					if (ImGui::InputInt("height", &m_h)) {
						obj.setSize(m_w, m_h);
					}
				}

				//scaling texture
				int oldScale = m_scale;
				if (ImGui::SliderInt("Scale", &m_scale, 0, 1500)) {
					int deltaScale = m_scale - oldScale;
					obj.scale(deltaScale, deltaScale);
				}
				
				if constexpr (RotatableObject<Object>) {
					ImGui::Text("Rotation");
					if (ImGui::SliderFloat("Angle", &m_angle, 0.0f, 360.0f)) {
						obj.rotate(static_cast<double>(m_angle), m_rotationPoint);
					}
					if (ImGui::InputInt("Rotation x", &m_rotationPoint.x)) {
						obj.rotate(static_cast<double>(m_angle), m_rotationPoint);
					}
					if (ImGui::InputInt("Rotation y", &m_rotationPoint.y)) {
						obj.rotate(static_cast<double>(m_angle), m_rotationPoint);
					}
				}
				
				if (ImGui::Button("Delete")) {
					objs->erase(selectedObjIt);
					selectedObjIt = objs->end();
					isSelected = false;
				}

				if (ImGui::Button("Duplicate")) {
					objs->push_back(obj);
					objs->back().setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
					selectedObjIt = objs->end() - 1;
				}

				ImGui::End();
			}

			/*search through a hive of objects, and if one is hovered over, update the
			iterator corresponding to the hive*/
			template<RenderObject Object>
			bool selectObj(ObjectLayersData<Object>& objHiveData, SDL_Point mousePos) {
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
			ObjectEditor(ImVec2 optionsPos) 
				: m_objOptionsPos{ optionsPos }
			{
				iterateStructs([this](auto& objHiveData) {
					objHiveData = { nullptr, {}, 0, false };
					return STAY_IN_LOOP;
				}, m_objLayers);
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
				}, m_objLayers);
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) { //if we have a new clicked 
					iterateStructs([&](auto& objHiveData) {
						return selectObj(objHiveData, mousePos);
					}, m_objLayers);
				}
			}
			
			template<RenderObject Object>
			void reseat(std::vector<Object>* objLayer, size_t layer) {
				assert(objLayer != nullptr);
				std::get<ObjectLayersData<Object>>(m_objLayers) = { objLayer, objLayer->end(), layer, false };
			}
		};

		struct TextureObjectAndPath : public TextureObject {
			TextureObjectAndPath(std::string_view path, TexturePtr tex, TextureData texData);
			std::string path;
		};

		template<RenderObject Object>
		void makeOneLayerMoreVisible(Layers<Object>& objLayers, size_t visibleLayer, Uint8 reducedOpacity) {
			auto reduceOpacity = [&](auto range) {
				for (auto& layer : range) {
					for (auto& obj : layer) {
						obj.setOpacity(reducedOpacity);
					}
				}
			};
			auto beforeVisibleLayer = ranges::subrange(objLayers.begin(), objLayers.begin() + visibleLayer);
			auto afterVisibleLayer  = ranges::subrange(objLayers.begin() + visibleLayer + 1, objLayers.end());
			reduceOpacity(beforeVisibleLayer); 
			reduceOpacity(afterVisibleLayer);

			//set to full opacity in case it was already reduced before we called this function
			for (auto& obj : objLayers[visibleLayer]) {
				obj.setOpacity(255);
			}
		}
	}
}

#endif
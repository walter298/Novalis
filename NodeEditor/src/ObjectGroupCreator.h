#include "ObjectSearch.h"

namespace nv {
	namespace editor {
		class ObjectGroupCreator {
		private:
			ObjectSearch m_searchedObjects;
		public:
			void setNewObjects(std::vector<Layer>& layers) {
				m_searchedObjects.setNewObjects(layers);
			}
			bool show() {
				bool showing = true;

				centerNextWindow();
				ImGui::SetNextWindowPos({ 1000.0f, 1000.0f });
				ImGui::Begin(OBJECT_GROUP_CREATION_WINDOW_NAME, &showing, WINDOW_FLAGS);

				ImGui::BeginChild("Object Search", { 500.0f, 500.0f });
				m_searchedObjects.show();
				ImGui::EndChild();
				
				ImGui::BeginChild("Objects", { 0.0f, 0.0f });
				ImGui::EndChild();

				ImGui::End();

				return showing;
			}
		};
	}
}
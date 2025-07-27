#pragma once

#include "ObjectSearch.h"
#include "ObjectGroupManager.h"

namespace nv {
	namespace editor {
		class ObjectGroupCreator {
		private:
			std::string m_objectGroupName;
			ObjectSearch m_searchedObjects;
			UniformObjectVector m_currentlyStoredObjects;
	
			void showObjectSearchChild(NameManager& objectGroupNameManager);
			bool showCurrentStoredObjectsChild(ObjectGroupManager& objectGroupManager);
		public:
			bool show(ObjectGroupManager& objectGroupManager, NameManager& objectGroupNameManager);

			void setNewObjects(std::vector<Layer>& layers) {
				m_searchedObjects.setNewObjects(layers);
			}
		};
	}
}
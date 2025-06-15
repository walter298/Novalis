#pragma once

#include <boost/unordered/unordered_node_set.hpp>
#include <boost/unordered/unordered_set.hpp>

#include "EditedObjectData.h"

namespace nv {
	namespace editor {
		template<typename Object>
		using ObjectLookup = boost::unordered_flat_set<EditedObjectData<Object>*>;

		struct EditedObjectGroup {
			std::string name;

			enum SyncOption : bool { 
				Unsynced = false,
				Synced = true
			};

			SyncOption positionSynced = SyncOption::Synced;
			SyncOption rotationSynced = SyncOption::Synced;
			SyncOption scaleSynced    = SyncOption::Synced;
			SyncOption opacitySynced  = SyncOption::Synced;

			using Objects = std::tuple<
				ObjectLookup<DynamicPolygon>,
				ObjectLookup<Texture>,
				ObjectLookup<BufferedNode>
			>;

			Objects objects;

			EditedObjectGroup()                             = default;
			EditedObjectGroup(const EditedObjectGroup&)     = delete;
			EditedObjectGroup(EditedObjectGroup&&) noexcept = default;

			template<typename Object>
			void addObject(EditedObjectData<Object>* object) {
				assert(object);
				std::get<boost::unordered_flat_set<EditedObjectData<Object>*>>(objects).insert(object);
			}
			template<typename Object>
			void removeObject(EditedObjectData<Object>* object) {
				assert(object);
				std::get<boost::unordered_flat_set<EditedObjectData<Object>*>>(objects).erase(object);
			}
		};
	}
}
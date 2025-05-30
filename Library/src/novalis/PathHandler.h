#pragma once

#include "detail/reflection/DataMemberUtil.h"
#include "BufferedNode.h"
#include "DynamicNode.h"
#include "KeyState.h"
#include "ObjectPath.h"

namespace nv {
    template<typename PolygonStorage>
    class PathHandler {
    private:
        std::reference_wrapper<const PolygonStorage> m_polygonStorage;

        template<typename Node>
        struct PathMapping {
            std::reference_wrapper<Node> node;
            ObjectPath path;
        };

        template<typename Node>
        using MovementMap = boost::unordered_flat_map<KeyState, std::vector<PathMapping<Node>>, detail::HashDataMembers>;

        std::tuple<MovementMap<BufferedNode>, MovementMap<DynamicNode>> m_objects;
    public:
        PathHandler(const PolygonStorage& polygonStorage) : m_polygonStorage{ std::ref(polygonStorage) }
        {
        }
        
        template<typename Node, typename Path>
        void map(Node& node, KeyState input, Path&& path) {
            auto& map = std::get<MovementMap<Node>>(m_objects);
            map[input].emplace_back(std::ref(node), std::forward<Path>(path));
        }
    };
}
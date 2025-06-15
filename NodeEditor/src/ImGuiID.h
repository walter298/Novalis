#pragma once

namespace nv {
    namespace editor {
        static int id = 0;

        inline int getUniqueImGuiID() {
            id++;
            return id;
        }
        inline void resetImGuiIDs() {
            id = 0;
        }
    }
}
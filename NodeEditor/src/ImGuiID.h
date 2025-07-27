#pragma once

namespace nv {
    namespace editor {
        int getPermanentImGuiID();
        int getTemporaryImGuiID() noexcept;
        void resetTemporaryImGuiIDs() noexcept;
    }
}
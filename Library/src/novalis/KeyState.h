#include <bitset>
#include <SDL3/SDL_Scancode.h>

#include "detail/reflection/ClassIteration.h"

//boost does not have a hash specialization for std::bitset, so manually specialize it, delegating to std::hash<std::bitset<N>>
namespace boost {
    template<size_t N>
    struct hash<std::bitset<N>> {
        size_t operator()(const std::bitset<N>& bitset) const noexcept {
            return std::hash<std::bitset<N>>{}(bitset);
        }
    };
}

namespace nv {
    struct KeyState {
		std::bitset<4> rep = 0;

		constexpr KeyState(std::bitset<4> rep) noexcept : rep{ rep } 
		{
		}

		constexpr KeyState(SDL_Scancode code) noexcept
			: rep{ rep | std::bitset<4>{ static_cast<uint64_t>(code) } } 
		{
		}

		constexpr explicit operator bool() const noexcept {
			return rep.any();
		}

        constexpr bool operator==(const KeyState& other) {
            return rep == other.rep;
        }

		constexpr KeyState operator&(const KeyState& other) noexcept {
			return KeyState{ rep & other.rep };
		}

		constexpr void operator|=(KeyState other) noexcept {
			rep |= other.rep;
		}

		constexpr void operator|=(SDL_Scancode code) noexcept {
			rep |= static_cast<uint64_t>(code);
		}

		constexpr void operator|=(uint64_t code) noexcept {
			rep |= code;
		}

		constexpr KeyState operator|(KeyState other) const noexcept {
			return rep | other.rep;
		}

		bool operator==(KeyState other) const noexcept {
			return rep == other.rep;
		}

        MAKE_INTROSPECTION(rep)
	};

	namespace keys {
		inline const KeyState A{ SDL_SCANCODE_A };
		inline const KeyState B{ SDL_SCANCODE_B };
		inline const KeyState C{ SDL_SCANCODE_C };
		inline const KeyState D{ SDL_SCANCODE_D };
		inline const KeyState E{ SDL_SCANCODE_E };
		inline const KeyState F{ SDL_SCANCODE_F };
		inline const KeyState G{ SDL_SCANCODE_G };
		inline const KeyState H{ SDL_SCANCODE_H };
		inline const KeyState I{ SDL_SCANCODE_I };
		inline const KeyState J{ SDL_SCANCODE_J };
		inline const KeyState K{ SDL_SCANCODE_K };
		inline const KeyState L{ SDL_SCANCODE_L };
		inline const KeyState M{ SDL_SCANCODE_M };
		inline const KeyState N{ SDL_SCANCODE_N };
		inline const KeyState O{ SDL_SCANCODE_O };
		inline const KeyState P{ SDL_SCANCODE_P };
		inline const KeyState Q{ SDL_SCANCODE_Q };
		inline const KeyState R{ SDL_SCANCODE_R };
		inline const KeyState S{ SDL_SCANCODE_S };
		inline const KeyState T{ SDL_SCANCODE_T };
		inline const KeyState U{ SDL_SCANCODE_U };
		inline const KeyState V{ SDL_SCANCODE_V };
		inline const KeyState W{ SDL_SCANCODE_W };
		inline const KeyState X{ SDL_SCANCODE_X };
		inline const KeyState Y{ SDL_SCANCODE_Y };
		inline const KeyState Z{ SDL_SCANCODE_Z };

        inline const KeyState SPACE{ SDL_SCANCODE_SPACE };

        inline const KeyState ZERO{ SDL_SCANCODE_0 };
		inline const KeyState ONE{ SDL_SCANCODE_1 };
		inline const KeyState TWO{ SDL_SCANCODE_2 };
		inline const KeyState THREE{ SDL_SCANCODE_3 };
		inline const KeyState FOUR{ SDL_SCANCODE_4 };
		inline const KeyState FIVE{ SDL_SCANCODE_5 };
		inline const KeyState SIX{ SDL_SCANCODE_6 };
		inline const KeyState SEVEN{ SDL_SCANCODE_7 };
		inline const KeyState EIGHT{ SDL_SCANCODE_8 };
		inline const KeyState NINE{ SDL_SCANCODE_9 };

		inline const KeyState LEFT_SHIFT{ SDL_SCANCODE_LSHIFT };
		inline const KeyState RIGHT_SHIFT{ SDL_SCANCODE_RSHIFT };

		inline const KeyState BACKSPACE{ SDL_SCANCODE_BACKSPACE };
	}
}
#pragma once

#include <algorithm>
#include <boost/container/static_vector.hpp>
#include <SDL3/SDL_mouse.h>

#include "EventHandler.h"
#include "Instance.h"
#include "KeyState.h"

namespace nv {
    static bool running = false;
    static constexpr size_t MAX_TEXT_INPUT_LEN = 256;
    static boost::container::static_vector<char, MAX_TEXT_INPUT_LEN> textInputBuff;
    static bool isTypingImpl = false;
    static MouseData mouseState;

    inline bool isTyping() {
        return isTypingImpl;
    }

    inline std::string_view getKeyboardInput() {
        return { textInputBuff.data(), textInputBuff.size() };
    }

    inline void cancelScene() {
        running = false;
    }

    inline MouseData getMouseState() {
        return mouseState;
    }

    static void pumpSDLEvents() {
        auto deltaX = 0.0f;
        auto deltaY = 0.0f;

        auto setMouseState = [](uint8_t btn, MouseButtonState newState) {
            switch (btn) {
            case SDL_BUTTON_LEFT:
                mouseState.left = newState;
                break;
            case SDL_BUTTON_MIDDLE:
                mouseState.mid = newState;
                break;
            case SDL_BUTTON_RIGHT:
                mouseState.right = newState;
                break;
            }
        };

        bool textEditing = false;

        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                setMouseState(evt.button.button, MouseButtonState::Down);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                setMouseState(evt.button.button, MouseButtonState::Released);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouseState.x = static_cast<float>(evt.button.x);
                mouseState.y = static_cast<float>(evt.button.y);
                deltaX = static_cast<float>(evt.motion.x);
                deltaY = static_cast<float>(evt.motion.y);
                break;
            case SDL_EVENT_TEXT_EDITING:
                auto newText = evt.edit.text;
                assert(evt.edit.length != -1);
                auto textLen = std::min(MAX_TEXT_INPUT_LEN, static_cast<size_t>(evt.edit.length));
                textInputBuff.resize(textLen);
                std::ranges::copy(newText, newText + textLen, textInputBuff.begin());
                textEditing = true;
                break;
            }
        }
	
        mouseState.deltaX = static_cast<float>(deltaX);
        mouseState.deltaY = static_cast<float>(deltaY);

        /*keystate = getPressedKeys();
        if (keystate & keys::BACKSPACE) {
            textInputBuff.pop_back();
        }*/
    }

    template<typename Node, typename Rep, typename Period>
    void showScene(Node& root, SDL_Renderer* renderer, EventHandler& handler, std::chrono::duration<Rep, Period> frameRate) {
        running = true;
        
        while (running) {
            auto endTime = std::chrono::system_clock::now() + frameRate;

            pumpSDLEvents();
            handler();

            SDL_RenderClear(renderer);
            root.render(renderer);

            const auto now = std::chrono::system_clock::now();
            if (now < endTime) {
                std::this_thread::sleep_for(endTime - now);
            }
            
            SDL_RenderPresent(renderer);
        }
    }
}
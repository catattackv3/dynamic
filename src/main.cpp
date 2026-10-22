// Predictive Input Buffer - community recreation for Geode / Geometry Dash 2.2081.
// This is a recreation, not the original author's source.
// 1 simulation tick = 1/240 s. At 60 FPS, 4 ticks are approximately 1 rendered frame.

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <deque>
#include <string>

using namespace geode::prelude;

namespace pib {
    struct Stage { int all; int wave; };

    // Cycle through 1-10 ticks, then OFF. Normal and wave inputs use the
    // selected amount so the HUD can show one unambiguous setting.
    constexpr Stage STAGES[10] = {
        {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5},
        {6, 6}, {7, 7}, {8, 8}, {9, 9}, {10, 10}
    };
    constexpr int OFF = 10; // stage index 10 means raw input / disabled

    // Preserve the recreation's original default of 2 ticks.
    inline int g_stage = 1;

    inline bool enabled() { return g_stage != OFF; }

    inline bool verbose() {
        return Mod::get()->getSettingValue<bool>("verbose-log");
    }

    inline std::string indicatorText() {
        if (!enabled()) return "INPUT BUFFER: OFF";
        auto const stage = STAGES[g_stage];
        return fmt::format("INPUT BUFFER: ON  |  {} TICKS", stage.all);
    }

    inline void logStage() {
        if (!enabled()) {
            log::info("[InputBuffer] F6 -> OFF (raw input)");
            return;
        }
        auto const stage = STAGES[g_stage];
        log::info("[InputBuffer] F6 -> ON: {} ticks (normal), {} ticks (wave)",
            stage.all, stage.wave);
    }
}

struct PendingInput {
    bool down;
    int button;
    bool player1;
    int remaining;
};

class $modify(PIBBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<PendingInput> queue;
        bool applying = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();

        // Forward immediately while disabled or while replaying a queued input.
        if (f->applying || !pib::enabled()) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto* player = isPlayer1 ? m_player1 : m_player2;
        bool const isWave = player && player->m_isDart;
        auto const stage = pib::STAGES[pib::g_stage];
        int const delay = isWave ? stage.wave : stage.all;

        f->queue.push_back({down, button, isPlayer1, delay});

        if (pib::verbose()) {
            log::info("[InputBuffer] Queued {} for {} ticks ({:.2f} frames at 60 FPS)",
                down ? "PRESS" : "RELEASE", delay, delay / 4.0f);
        }
    }

    // Advance queued inputs once per physics tick.
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto f = m_fields.self();

        for (auto it = f->queue.begin(); it != f->queue.end();) {
            it->remaining -= 1;
            if (it->remaining <= 0) {
                f->applying = true;
                GJBaseGameLayer::handleButton(it->down, it->button, it->player1);
                f->applying = false;
                it = f->queue.erase(it);
            } else {
                ++it;
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(PIBPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* indicator = nullptr;
    };

    bool init(GJGameLevel* level, bool p1, bool p2) {
        if (!PlayLayer::init(level, p1, p2)) return false;

        auto const winSize = CCDirector::sharedDirector()->getWinSize();
        auto* label = CCLabelBMFont::create(pib::indicatorText().c_str(), "bigFont.fnt");
        if (label) {
            label->setAnchorPoint({0.f, 1.f});
            label->setPosition({12.f, winSize.height - 12.f});
            label->setScale(0.42f);
            label->setZOrder(9999);
            addChild(label, 9999);
            m_fields->indicator = label;
            updateIndicator();
        }
        return true;
    }

    void updateIndicator() {
        auto* label = m_fields->indicator;
        if (!label) return;

        auto const text = pib::indicatorText();
        label->setString(text.c_str());
        if (pib::enabled()) {
            label->setColor({120, 255, 140});
        } else {
            label->setColor({255, 130, 130});
        }
    }

    void resetLevel() {
        // Prevent queued input from leaking into the next attempt.
        static_cast<PIBBaseGameLayer*>(static_cast<GJBaseGameLayer*>(this))
            ->m_fields->queue.clear();
        PlayLayer::resetLevel();
        updateIndicator();
    }
};

class $modify(PIBKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_F6) {
            pib::g_stage = (pib::g_stage + 1) % 11;
            pib::logStage();

            if (auto* layer = static_cast<PIBPlayLayer*>(PlayLayer::get())) {
                layer->updateIndicator();
            }
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};

$on_mod(Loaded) {
    log::info("[InputBuffer] Predictive Input Buffer recreation loaded.");
    log::info("[InputBuffer] Default: ON, 2 ticks. Press F6 in-level to cycle 1-10 ticks, then OFF.");
}

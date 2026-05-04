#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <cmath>
#include <vector>

using namespace geode::prelude;

namespace {
    struct DeathPoint {
        float x;
        int deathsSeen;
    };

    class LearningBot {
    public:
        void resetForNewAttempt() {}

        void resetForNewLevel() {
            m_deathPoints.clear();
        }

        void onFrame(PlayLayer* layer) {
            if (!Mod::get()->getSettingValue<bool>("enabled") || !layer || !layer->m_player1) {
                return;
            }

            // Conservative compile-safe behavior:
            // for now only tracks proximity to learned points.
            auto const playerX = layer->m_player1->getPositionX();
            auto const decisionWindow = static_cast<float>(Mod::get()->getSettingValue<int>("decision-window"));
            for (auto const& point : m_deathPoints) {
                if (std::fabs(playerX - point.x) <= decisionWindow) {
                    // Placeholder: action logic intentionally omitted until API-specific input calls are validated.
                    break;
                }
            }
        }

        void onDeath(PlayLayer* layer) {
            if (!Mod::get()->getSettingValue<bool>("enabled") || !layer || !layer->m_player1) {
                return;
            }

            auto const x = layer->m_player1->getPositionX();
            auto const mergeWindow = static_cast<float>(Mod::get()->getSettingValue<int>("merge-window"));
            auto const maxMemory = static_cast<size_t>(Mod::get()->getSettingValue<int>("max-memory"));

            bool merged = false;
            for (auto& point : m_deathPoints) {
                if (std::fabs(point.x - x) <= mergeWindow) {
                    point.x = (point.x * point.deathsSeen + x) / static_cast<float>(point.deathsSeen + 1);
                    point.deathsSeen += 1;
                    merged = true;
                    break;
                }
            }

            if (!merged) {
                if (m_deathPoints.size() >= maxMemory && !m_deathPoints.empty()) {
                    m_deathPoints.erase(m_deathPoints.begin());
                }
                m_deathPoints.push_back({x, 1});
            }

            if (Mod::get()->getSettingValue<bool>("auto-restart")) {
                layer->resetLevel();
            }
        }

    private:
        std::vector<DeathPoint> m_deathPoints;
    };

    LearningBot g_bot;
}

class $modify(AIPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        g_bot.resetForNewLevel();
        return true;
    }

    void resetLevel() {
        g_bot.resetForNewAttempt();
        PlayLayer::resetLevel();
    }

    void update(float dt) {
        PlayLayer::update(dt);
        g_bot.onFrame(this);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        g_bot.onDeath(this);
        PlayLayer::destroyPlayer(player, object);
    }
};

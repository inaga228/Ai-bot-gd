#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {
    struct DeathPoint {
        float x;
        bool requiresJump;
        int deathsSeen;
    };

    class LearningBot {
    public:
        void resetForNewAttempt() {
            m_appliedIndices.clear();
            m_jumpHoldFrames = 0;
        }

        void resetForNewLevel() {
            m_deathPoints.clear();
            resetForNewAttempt();
        }

        void onFrame(PlayLayer* layer) {
            if (!Mod::get()->getSettingValue<bool>("enabled") || !layer->m_player1) {
                return;
            }

            if (m_jumpHoldFrames > 0) {
                --m_jumpHoldFrames;
                if (m_jumpHoldFrames == 0) {
                    layer->releaseButton(PlayerButton::Jump, true);
                }
            }

            auto playerX = layer->m_player1->getPositionX();
            auto decisionWindow = static_cast<float>(Mod::get()->getSettingValue<int64_t>("decision-window"));

            for (size_t i = 0; i < m_deathPoints.size(); ++i) {
                if (m_appliedIndices.contains(i)) {
                    continue;
                }

                auto const& point = m_deathPoints[i];
                if (std::abs(playerX - point.x) <= decisionWindow) {
                    if (point.requiresJump) {
                        layer->pushButton(PlayerButton::Jump, true);
                        m_jumpHoldFrames = 2;
                    }
                    m_appliedIndices.insert(i);
                }
            }
        }

        void onDeath(PlayLayer* layer) {
            if (!Mod::get()->getSettingValue<bool>("enabled") || !layer->m_player1) {
                return;
            }

            auto x = layer->m_player1->getPositionX();
            auto mergeWindow = static_cast<float>(Mod::get()->getSettingValue<int64_t>("merge-window"));
            auto maxMemory = static_cast<size_t>(Mod::get()->getSettingValue<int64_t>("max-memory"));

            bool merged = false;
            for (auto& point : m_deathPoints) {
                if (std::abs(point.x - x) <= mergeWindow) {
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
                m_deathPoints.push_back({x, true, 1});
            }

            log::info("[AI Bot] death at x={:.2f}, memory points={}", x, m_deathPoints.size());

            resetForNewAttempt();
            if (Mod::get()->getSettingValue<bool>("auto-restart")) {
                layer->resetLevel();
            }
        }

    private:
        std::vector<DeathPoint> m_deathPoints;
        std::unordered_set<size_t> m_appliedIndices;
        int m_jumpHoldFrames = 0;
    };

    LearningBot g_bot;
}

class $modify(AIPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        g_bot.resetForNewLevel();
        log::info("[AI Bot] Initialized for level: {}", level ? level->m_levelName : "<unknown>");
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

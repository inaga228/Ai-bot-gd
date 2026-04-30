#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {
    struct DeathPoint {
        float x;
        bool requiresJump;
    };

    class LearningBot {
    public:
        void resetForNewLevel() {
            m_framesAlive = 0;
            m_appliedCheckpoints.clear();
        }

        void onFrame(PlayLayer* layer) {
            if (!Mod::get()->getSettingValue<bool>("enabled")) {
                return;
            }

            ++m_framesAlive;
            auto playerX = layer->m_player1->getPositionX();
            auto window = static_cast<float>(Mod::get()->getSettingValue<int64_t>("decision-window"));

            for (size_t i = 0; i < m_deathPoints.size(); ++i) {
                if (m_appliedCheckpoints.contains(i)) {
                    continue;
                }

                auto const& point = m_deathPoints[i];
                if (std::abs(playerX - point.x) <= window) {
                    if (point.requiresJump) {
                        layer->pushButton(PlayerButton::Jump, true);
                    }
                    m_appliedCheckpoints.insert(i);
                }
            }
        }

        void onDeath(PlayLayer* layer) {
            if (!Mod::get()->getSettingValue<bool>("enabled")) {
                return;
            }

            auto x = layer->m_player1->getPositionX();
            m_deathPoints.push_back({x, true});
            log::info("[AI Bot] Learned death point at x = {:.2f}. Total memory: {}", x, m_deathPoints.size());

            m_framesAlive = 0;
            m_appliedCheckpoints.clear();

            if (Mod::get()->getSettingValue<bool>("auto-restart")) {
                layer->resetLevel();
            }
        }

    private:
        int m_framesAlive = 0;
        std::vector<DeathPoint> m_deathPoints;
        std::unordered_set<size_t> m_appliedCheckpoints;
    };

    LearningBot g_bot;
}

class $modify(AIPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        g_bot.resetForNewLevel();
        log::info("[AI Bot] Initialized for level: {}", level->m_levelName);
        return true;
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

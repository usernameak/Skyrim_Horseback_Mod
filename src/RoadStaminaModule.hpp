#pragma once

#include "Module.hpp"

namespace hback {
    class RoadStaminaModule : public Module {
        float m_staminaConsumptionRateOnRoad;

    public:
        RoadStaminaModule();

        void loadConfig(const cpptoml::table &table) override;

        [[nodiscard]] size_t getTrampolineSize() const override;

        void dataLoaded() override;

        [[nodiscard]] float getStaminaConsumptionRateOnRoad() const { return m_staminaConsumptionRateOnRoad; }
    };
}
#pragma once

#include "Module.hpp"

namespace hback {
    class HorseInCellsModule : public Module {
    public:
        HorseInCellsModule();

        void loadConfig(const cpptoml::table &table) override;

        [[nodiscard]] size_t getTrampolineSize() const override;

        void dataLoaded() override;
    };
}
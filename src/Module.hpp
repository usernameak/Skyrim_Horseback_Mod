// Horseback -- mod for The Elder Scrolls V: Skyrim that extends
// horseback riding features.
// Copyright (C) 2023-2026  usernameak
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <cpptoml.h>

namespace hback {
    class Module {
        bool m_enabled;

    public:
        Module();

        virtual ~Module() = default;

        virtual void loadConfig(const cpptoml::table &table);

        [[nodiscard]] virtual size_t getTrampolineSize() const {
            return 0;
        }

        virtual void dataLoaded() = 0;

        [[nodiscard]] bool isEnabled() const { return m_enabled; }
        void setEnabled(bool enabled) { m_enabled = enabled; }
    };

    class ModuleManager {
        std::unordered_map<std::string, std::unique_ptr<Module>> m_modules;

    public:
        void registerModule(std::string name, std::unique_ptr<Module> module);
        [[nodiscard]] Module *getModule(const std::string &name) const {
            auto it = m_modules.find(name);
            if (it == m_modules.end()) {
                return nullptr;
            }
            return it->second.get();
        }

        void dataLoaded() const {
            for (const auto &module : m_modules | std::views::values) {
                if (module->isEnabled()) {
                    module->dataLoaded();
                }
            }
        }

        [[nodiscard]] auto &getModules() const {
            return m_modules;
        }
    };

    extern ModuleManager g_moduleManager;
}

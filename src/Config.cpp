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

#include "Config.hpp"

#include "Module.hpp"

#include <SKSE/SKSE.h>
#include <cpptoml.h>

constexpr static std::string_view configPath = "Data/SKSE/Plugins/Horseback.toml";

namespace hback {
    Config g_config;

    void Config::load() {
        SKSE::log::info("loading config");
        try {
            auto configFile = cpptoml::parse_file(std::string(configPath));

            if (auto modules = configFile->get_table("modules")) {
                for (auto &[name, moduleConfig] : *modules) {
                    Module *mod = g_moduleManager.getModule(name);
                    if (moduleConfig && moduleConfig->is_table()) {
                        mod->loadConfig(*moduleConfig->as_table());
                    }
                }
            }
        } catch (cpptoml::parse_exception &e) {
            SKSE::log::error("config load error: {}", e.what());
        }
        SKSE::log::info("config loaded!");
    }
}

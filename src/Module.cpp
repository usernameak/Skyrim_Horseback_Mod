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

#include "Module.hpp"

hback::ModuleManager hback::g_moduleManager;

hback::Module::Module() : m_enabled(false) {}

void hback::Module::loadConfig(const cpptoml::table &table) {
    m_enabled = table.get_as<bool>("enabled").value_or(m_enabled);
}

void hback::ModuleManager::registerModule(std::string name, std::unique_ptr<Module> module) {
    m_modules[std::move(name)] = std::move(module);
}

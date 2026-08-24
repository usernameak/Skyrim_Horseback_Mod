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
#include "HorseInCellsModule.hpp"
#include "Module.hpp"
#include "RoadStaminaModule.hpp"

#include <SKSE/SKSE.h>
#include <REL/Relocation.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

void InitLogger() {
    auto path = SKSE::log::log_directory();
    if (!path)
        return;

    auto plugin = SKSE::PluginDeclaration::GetSingleton();
    *path /= std::format("{}.log", plugin->GetName());

    std::shared_ptr<spdlog::sinks::sink> sink;
    if (REX::W32::IsDebuggerPresent()) {
        sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    } else {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    }

    auto log = std::make_shared<spdlog::logger>("global log", sink);
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%s(%#): [%^%l%$] %v");
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message *message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        hback::g_moduleManager.registerModule("RoadStamina", std::make_unique<hback::RoadStaminaModule>());
        hback::g_moduleManager.registerModule("HorseInCells", std::make_unique<hback::HorseInCellsModule>());
        hback::g_config.load();

        for (const auto &[name, module] : hback::g_moduleManager.getModules()) {
            if (module->isEnabled()) {
                SKSE::log::info("Module {} is enabled", name);
            }
        }

        size_t totalTrampolineSize = 0;
        for (const auto &module : hback::g_moduleManager.getModules() | std::views::values) {
            if (module->isEnabled()) {
                totalTrampolineSize += module->getTrampolineSize();
            }
        }
        if (totalTrampolineSize != 0) {
            SKSE::AllocTrampoline(totalTrampolineSize);
        }

        hback::g_moduleManager.dataLoaded();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    InitLogger();

    SKSE::Init(skse);

    auto *msgIfc = SKSE::GetMessagingInterface();
    msgIfc->RegisterListener(&SKSEMessageHandler);

    SKSE::log::info("Horseback loaded");

    return true;
}

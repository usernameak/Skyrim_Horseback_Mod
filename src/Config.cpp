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

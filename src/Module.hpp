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

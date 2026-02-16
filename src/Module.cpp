#include "Module.hpp"

hback::ModuleManager hback::g_moduleManager;

hback::Module::Module() : m_enabled(false) {}

void hback::Module::loadConfig(const cpptoml::table &table) {
    m_enabled = table.get_as<bool>("enabled").value_or(m_enabled);
}

void hback::ModuleManager::registerModule(std::string name, std::unique_ptr<Module> module) {
    m_modules[std::move(name)] = std::move(module);
}

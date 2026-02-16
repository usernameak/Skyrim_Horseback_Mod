includes("lib/commonlibsse")

set_project("Horseback")
set_version("1.0.3")
set_license("BSD-3-Clause")
set_languages("c++23")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

set_policy("package.requires_lock", true)

add_requires("vcpkg::cpptoml", "xbyak")

set_config("skse_xbyak", true)

if is_mode("debug") then
    add_defines("_DEBUG")
end

rule("msvc_settings")
    on_config(function (target)
        if is_mode("releasedbg") then
            target:add("cxflags", "cl::/Gy", {force=true})
            if target:kind() ~= "static" then
                target:add("ldflags", "link::-OPT:REF", "link::-OPT:ICF")
            end
        end
    end)

target("commonlibsse")
    add_rules("msvc_settings")

target("commonlib-shared")
    add_rules("msvc_settings")

target("Horseback")
    add_deps("commonlibsse-ng")
    add_packages("vcpkg::cpptoml")
    
    add_rules("commonlibsse-ng.plugin", {
        name = "Horseback",
        author = "usernameak",
        description = "Horseback - Witcher-style horse stamina mechanic"
    })
    
    add_files("src/**.cpp")
    add_headerfiles("src/**.hpp")

    add_rules("msvc_settings")

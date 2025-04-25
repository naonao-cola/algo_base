set_project("tvt")
set_version("1.0.0")
set_languages("c++17")
add_rules("mode.debug", "mode.release","mode.releasedbg")

-- 指定三方库目录
local thirdparty = "E:/demo/3rdparty/algo_3rdparty/tv_algo_base_3rdparty/"

add_requires("opencv 4.8.0",{system = false})
add_requires("nlohmann_json v3.11.3",{system = false})
add_requires("fmt 11.1.4",{system = false})
add_requires("cpptrace v0.8.2",{system = false})
-- 宏定义
add_defines("USE_AI_DETECT")


if is_mode "debug" then
    add_defines("DEBUG")
    set_symbols "debug"
    set_optimize "none"
    set_runtimes("MT")
end

if is_mode "release" then
    --set_symbols "hidden"
    --set_optimize "fastest"
	set_runtimes("MT")
	--调试时打开下面两个
	set_optimize "none"
    set_symbols("debug")
end














--自动更新vs解决方案结构
add_rules("plugin.vsxmake.autoupdate")

-- 算法dll
target("tv_algorithm")
	set_kind("shared")
	add_packages("opencv")
	add_packages("fmt")
	add_packages("nlohmann_json")
	add_packages("cpptrace")

	add_includedirs("3rdparty/ai_inference/include")
	add_linkdirs("3rdparty/ai_inference/lib")
	add_links("AIFramework")

    add_headerfiles("src/framework/*h")
	add_files("src/framework/*cpp")
	add_headerfiles("src/example/*h")
	add_files("src/example/*cpp")
    add_headerfiles("src/utils/*h")
	add_files("src/utils/*cpp")
    add_headerfiles("src/*.h")
	add_files("src/*.cpp")
    add_defines("EXPORT_API")

-- 调用算法dll测试程序
target("test_dll")
    set_kind("binary")
	add_packages("opencv")
	add_packages("fmt")
	add_packages("nlohmann_json")
	add_packages("cpptrace")
	add_deps("tv_algorithm")
    add_defines("EXPORT_API")
	add_files("test/main.cpp")



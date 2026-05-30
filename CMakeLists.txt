cmake_minimum_required(VERSION 3.14)
project(EUI-NEO VERSION 1.0)

if(APPLE)
    enable_language(OBJC)
    set_source_files_properties(core/tray_bridge.c PROPERTIES LANGUAGE OBJC)
    set_source_files_properties(core/ime_bridge.c PROPERTIES LANGUAGE OBJC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include("${CMAKE_CURRENT_SOURCE_DIR}/3rd/dependencies.cmake")

set(CORE_SOURCES
    core/async.cpp
    core/image.cpp
    core/ime_bridge.c
    core/network.cpp
    core/platform.cpp
    core/tray_bridge.c
    core/text.cpp
)

set(APP_ASSET_FILES)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(GLOB APP_ASSET_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
endif()

set(APP_ICON_RESOURCE)
if(WIN32)
    set(APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico")
    set(APP_ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/eui_app_icon.rc")
    file(TO_NATIVE_PATH "${APP_ICON_ICO}" APP_ICON_ICO_NATIVE)
    string(REPLACE "\\" "\\\\" APP_ICON_ICO_ESCAPED "${APP_ICON_ICO_NATIVE}")
    file(WRITE "${APP_ICON_RC}" "IDI_APP_ICON ICON \"${APP_ICON_ICO_ESCAPED}\"\n")
    set(APP_ICON_RESOURCE "${APP_ICON_RC}")
endif()

function(configure_opengl_app target_name)
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${TRAY_INCLUDE_DIR})
    target_link_libraries(${target_name} PRIVATE glfw glad OpenGL::GL Threads::Threads)

    if(TARGET harfbuzz::harfbuzz)
        target_link_libraries(${target_name} PRIVATE harfbuzz::harfbuzz)
        target_compile_definitions(${target_name} PRIVATE EUI_HAS_HARFBUZZ=1)
    elseif(TARGET PkgConfig::EUI_HARFBUZZ)
        target_link_libraries(${target_name} PRIVATE Freetype::Freetype)
        target_link_libraries(${target_name} PRIVATE PkgConfig::EUI_HARFBUZZ)
        target_compile_definitions(${target_name} PRIVATE EUI_HAS_HARFBUZZ=1)
    else()
        target_link_libraries(${target_name} PRIVATE Freetype::Freetype)
    endif()

    if(MSVC)
        set_target_properties(${target_name} PROPERTIES VS_GLOBAL_VcpkgXUseBuiltInApplocalDeps true)
        target_compile_options(${target_name} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> /GS- /sdl- /utf-8 /wd4819)
        target_link_options(${target_name} PRIVATE /OPT:REF /OPT:ICF /ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS)
    elseif(APPLE)
        target_compile_options(${target_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Os -fno-exceptions -fno-rtti>)
        target_link_options(${target_name} PRIVATE -Wl,-dead_strip)
    else()
        target_compile_options(${target_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Os -fno-exceptions -fno-rtti>)
        target_link_options(${target_name} PRIVATE -Wl,--gc-sections -s)
    endif()

    if(WIN32)
        target_compile_definitions(${target_name} PRIVATE EUI_TRAY_WINAPI=1)
        target_link_libraries(${target_name} PRIVATE winmm urlmon shell32 user32 imm32)

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:glfw>
            $<TARGET_FILE_DIR:${target_name}>
        )
    elseif(APPLE)
        target_compile_definitions(${target_name} PRIVATE EUI_TRAY_APPKIT=1)
        target_link_libraries(${target_name} PRIVATE "-framework Cocoa" objc)
    elseif(UNIX AND EUI_APPINDICATOR_FOUND AND EUI_GTK3_FOUND)
        target_compile_definitions(${target_name} PRIVATE EUI_TRAY_APPINDICATOR=1)
        target_include_directories(${target_name} PRIVATE ${EUI_APPINDICATOR_INCLUDE_DIRS} ${EUI_GTK3_INCLUDE_DIRS})
        target_compile_options(${target_name} PRIVATE ${EUI_APPINDICATOR_CFLAGS_OTHER} ${EUI_GTK3_CFLAGS_OTHER})
        target_link_directories(${target_name} PRIVATE ${EUI_APPINDICATOR_LIBRARY_DIRS} ${EUI_GTK3_LIBRARY_DIRS})
        target_link_libraries(${target_name} PRIVATE ${EUI_APPINDICATOR_LIBRARIES} ${EUI_GTK3_LIBRARIES})
    endif()

    if(TARGET CURL::libcurl)
        target_link_libraries(${target_name} PRIVATE CURL::libcurl)
        target_compile_definitions(${target_name} PRIVATE EUI_HAS_CURL=1)
    endif()

    if(APP_ASSET_FILES)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${target_name}>/assets
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${APP_ASSET_FILES} $<TARGET_FILE_DIR:${target_name}>/assets
        )
    endif()
endfunction()

file(GLOB APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp")
foreach(app_source ${APP_SOURCES})
    get_filename_component(app_name "${app_source}" NAME_WE)
    add_executable(${app_name} main.cpp "${app_source}" ${CORE_SOURCES} ${APP_ICON_RESOURCE})
    configure_opengl_app(${app_name})
endforeach()

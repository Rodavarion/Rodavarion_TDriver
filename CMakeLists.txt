cmake_minimum_required(VERSION 3.22)

project(RodavarionTDriver
    VERSION 1.0.1
    DESCRIPTION "Universal peripheral-management platform by Rodavarion Technologies"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" RODAVARION_VERSION_STRING LIMIT_COUNT 1)
if(NOT RODAVARION_VERSION_STRING)
    set(RODAVARION_VERSION_STRING "${PROJECT_VERSION}")
endif()


option(RODAVARION_BUILD_GUI "Build the full Qt Widgets application" ON)
option(RODAVARION_BUILD_DAEMON "Build the headless background service" ON)
option(RODAVARION_BUILD_CLI "Build the service-control CLI" ON)
option(RODAVARION_BUILD_TESTS "Build tests" ON)
option(RODAVARION_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(RODAVARION_ENABLE_MOCK_DEVICES "Show mock devices together with real devices" OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core DBus)

if(RODAVARION_BUILD_GUI)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
endif()

find_path(HIDAPI_INCLUDE_DIR NAMES hidapi/hidapi.h)
find_library(HIDAPI_LIBRARY NAMES hidapi-hidraw hidapi-libusb hidapi)

find_path(LIBEVDEV_INCLUDE_DIR
    NAMES libevdev/libevdev.h
    PATH_SUFFIXES libevdev-1.0
)
find_library(LIBEVDEV_LIBRARY NAMES evdev libevdev)

if(NOT HIDAPI_INCLUDE_DIR OR NOT HIDAPI_LIBRARY)
    message(FATAL_ERROR
        "HIDAPI was not found. On Arch Linux install it with: sudo pacman -S hidapi"
    )
endif()

if(NOT LIBEVDEV_INCLUDE_DIR OR NOT LIBEVDEV_LIBRARY)
    message(FATAL_ERROR
        "libevdev was not found. On Arch Linux install it with: sudo pacman -S libevdev"
    )
endif()

qt_standard_project_setup()

add_library(rodavarion_core
    include/rodavarion/context/ContextBridge.hpp
    src/core/Logger.cpp
    src/core/Application.cpp
    src/config/ConfigService.cpp
    src/i18n/LocalizationService.cpp
    src/catalog/VendorDatabase.cpp
    src/capability/DeviceCapabilityClassifier.cpp
    src/device/DeviceManager.cpp
    src/device/PhysicalDeviceGrouper.cpp
    src/device/PeripheralDescriptor.cpp
    src/event/EventBus.cpp
    src/plugin/PluginRegistry.cpp
    src/profile/ProfileManager.cpp
    src/profile/ProfileStore.cpp
    src/action/MouseAction.cpp
    src/action/MouseMappingStore.cpp
    src/context/ApplicationContext.cpp
    src/context/ContextBridge.cpp
    src/safety/ActionSafetyPolicy.cpp
    src/runtime/DesktopActionExecutor.cpp
    src/runtime/DesktopOutputEngine.cpp
    src/runtime/ActionEngine.cpp
    src/input/MouseEventNormalizer.cpp
    src/setup/DependencySetupManager.cpp
    src/setup/DependencyMaintenanceManager.cpp
    src/mock/MockDeviceProvider.cpp
    src/hid/HidDeviceProvider.cpp
    src/system/LinuxDiagnostics.cpp
    src/transport/TransportClassifier.cpp
)

set_target_properties(rodavarion_core PROPERTIES
    AUTOMOC ON
)

target_include_directories(rodavarion_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${HIDAPI_INCLUDE_DIR}
)

target_link_libraries(rodavarion_core
    PUBLIC Qt6::Core Qt6::DBus
    PRIVATE ${HIDAPI_LIBRARY}
)

target_compile_definitions(rodavarion_core
    PUBLIC
        RODAVARION_ENABLE_MOCK_DEVICES=$<BOOL:${RODAVARION_ENABLE_MOCK_DEVICES}>
        RODAVARION_VERSION_STRING="${RODAVARION_VERSION_STRING}"
)

add_library(rodavarion_runtime
    include/rodavarion/input/EvdevMouseMonitor.hpp
    src/input/EvdevMouseMonitor.cpp
    src/runtime/MouseRuntimeController.cpp
)

set_target_properties(rodavarion_runtime PROPERTIES
    AUTOMOC ON
)

target_include_directories(rodavarion_runtime
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${LIBEVDEV_INCLUDE_DIR}
)

target_link_libraries(rodavarion_runtime
    PUBLIC rodavarion_core Qt6::Core
    PRIVATE ${LIBEVDEV_LIBRARY}
)

function(rodavarion_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(RODAVARION_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(RODAVARION_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

rodavarion_enable_warnings(rodavarion_core)
rodavarion_enable_warnings(rodavarion_runtime)

if(RODAVARION_BUILD_GUI)
    add_library(rodavarion_gui
        src/gui/MainWindow.cpp
        src/gui/MouseActionDialog.cpp
        src/gui/DependencySetupDialog.cpp
    )

    target_include_directories(rodavarion_gui
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(rodavarion_gui
        PUBLIC rodavarion_runtime Qt6::Widgets
    )

    rodavarion_enable_warnings(rodavarion_gui)

    qt_add_executable(rodavarion-tdriver
        src/app/main.cpp
    )

    target_link_libraries(rodavarion-tdriver
        PRIVATE rodavarion_gui Qt6::Widgets
    )

    set_target_properties(rodavarion-tdriver PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
    )
endif()

if(RODAVARION_BUILD_DAEMON)
    qt_add_executable(rodavarion-tdriverd
        src/daemon/main.cpp
    )

    target_link_libraries(rodavarion-tdriverd
        PRIVATE rodavarion_runtime Qt6::Core Qt6::DBus
    )

    rodavarion_enable_warnings(rodavarion-tdriverd)
endif()

if(RODAVARION_BUILD_CLI)
    qt_add_executable(rodavarion-tdriverctl
        src/cli/main.cpp
    )

    target_link_libraries(rodavarion-tdriverctl
        PRIVATE Qt6::Core
    )

    rodavarion_enable_warnings(rodavarion-tdriverctl)
endif()

include(GNUInstallDirs)

if(TARGET rodavarion-tdriver)
    install(TARGETS rodavarion-tdriver
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT gui
    )
endif()

if(TARGET rodavarion-tdriverd)
    install(TARGETS rodavarion-tdriverd
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT daemon
    )
endif()

if(TARGET rodavarion-tdriverctl)
    install(TARGETS rodavarion-tdriverctl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT daemon
    )
endif()

install(DIRECTORY config/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/rodavarion-tdriver/config
    COMPONENT daemon
)

install(FILES
    packaging/systemd/user/rodavarion-tdriverd.service
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user
    COMPONENT daemon
)

if(RODAVARION_BUILD_TESTS)
    enable_testing()

    add_executable(rodavarion_tests
        tests/test_main.cpp
    )

    target_link_libraries(rodavarion_tests
        PRIVATE rodavarion_core Qt6::Core
    )

    rodavarion_enable_warnings(rodavarion_tests)
    add_test(NAME rodavarion_tests COMMAND rodavarion_tests)
endif()

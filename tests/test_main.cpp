#include "rodavarion/action/MouseAction.hpp"
#include "rodavarion/action/MouseMappingStore.hpp"
#include "rodavarion/safety/ActionSafetyPolicy.hpp"
#include "rodavarion/runtime/DesktopActionExecutor.hpp"
#include "rodavarion/input/MouseEventNormalizer.hpp"
#include "rodavarion/context/ApplicationContext.hpp"

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);

    rodavarion::action::MouseButtonMapping safeMapping{
        .button = rodavarion::action::MouseButton::Back,
        .action = rodavarion::action::ActionType::Copy,
        .parameter = {}
    };

    const auto dryRun =
        rodavarion::safety::ActionSafetyPolicy::validateForDryRun(
            safeMapping
        );

    if (!dryRun.allowed) {
        std::cerr << "Dry-run safety validation failed.\n";
        return EXIT_FAILURE;
    }

    rodavarion::action::MouseButtonMapping invalidScript{
        .button = rodavarion::action::MouseButton::Forward,
        .action = rodavarion::action::ActionType::RunScript,
        .parameter = {}
    };

    const auto scriptDryRun =
        rodavarion::safety::ActionSafetyPolicy::validateForDryRun(
            invalidScript
        );

    if (scriptDryRun.allowed) {
        std::cerr << "Empty script path was accepted.\n";
        return EXIT_FAILURE;
    }

    const auto runtime =
        rodavarion::safety::ActionSafetyPolicy::validateForRuntime(
            safeMapping
        );

    if (!runtime.allowed) {
        std::cerr << "Standard runtime action was rejected.\n";
        return EXIT_FAILURE;
    }

    const auto invalidScriptRuntime =
        rodavarion::safety::ActionSafetyPolicy::validateForRuntime(
            invalidScript
        );

    if (invalidScriptRuntime.allowed) {
        std::cerr << "Invalid runtime script was accepted.\n";
        return EXIT_FAILURE;
    }

    rodavarion::action::MouseMappingProfile profile;
    profile.deviceKey = "046D:B034:TEST";
    profile.mappings = {
        safeMapping,
        {
            .button = rodavarion::action::MouseButton::Forward,
            .action = rodavarion::action::ActionType::RunScript,
            .parameter = "/tmp/example.sh"
        }
    };

    QTemporaryDir directory;
    if (!directory.isValid()) {
        std::cerr << "Cannot create temporary directory.\n";
        return EXIT_FAILURE;
    }

    const auto filePath = directory.filePath("mouse-mappings.json");
    rodavarion::action::MouseMappingStore store;

    if (!store.save(filePath, profile)) {
        std::cerr << "Mouse mapping save failed.\n";
        return EXIT_FAILURE;
    }

    const auto loaded = store.load(
        filePath,
        profile.deviceKey
    );

    if (!loaded
        || loaded.value().mappings.size() != 2
        || loaded.value().mappings[0].action
            != rodavarion::action::ActionType::Copy) {
        std::cerr << "Mouse mapping load failed.\n";
        return EXIT_FAILURE;
    }

    const auto buttons =
        rodavarion::action::defaultMouseButtons();

    if (buttons.size() < 12
        || buttons[buttons.size() - 2]
            != rodavarion::action::MouseButton::WheelUp
        || buttons.back()
            != rodavarion::action::MouseButton::WheelDown) {
        std::cerr << "Wheel button model failed.\n";
        return EXIT_FAILURE;
    }

    if (rodavarion::runtime::DesktopActionExecutor::
            backendDescription().isEmpty()) {
        std::cerr << "Executor backend description failed.\n";
        return EXIT_FAILURE;
    }


    qputenv("RODAVARION_CONFIG_DIR", directory.path().toUtf8());

    // Simulate an upgrade from an older release whose persisted settings only
    // contained the terminal profile. load() must preserve it and append newly
    // introduced browser/default profiles.
    const QVector<rodavarion::context::ApplicationProfile> legacyProfiles = {
        {"terminal", "Термінали", {"konsole", "org.kde.konsole"},
         "CTRL+SHIFT+C", "CTRL+SHIFT+V", "CTRL+SHIFT+X",
         "CTRL+SHIFT+Z", "CTRL+SHIFT+Z", "CTRL+SHIFT+A", true}
    };
    const auto legacySave = rodavarion::context::ApplicationContext::save(legacyProfiles);
    if (!legacySave) {
        std::cerr << "Legacy context fixture save failed.\n";
        return EXIT_FAILURE;
    }

    qputenv("RODAVARION_ACTIVE_APP", "firefox | org.mozilla.firefox");
    if (rodavarion::context::ApplicationContext::shortcutFor(
            rodavarion::action::ActionType::Paste) != "CTRL+V") {
        std::cerr << "Browser paste fallback failed.\n";
        return EXIT_FAILURE;
    }

    qputenv("RODAVARION_ACTIVE_APP", "org.kde.konsole | konsole");
    if (rodavarion::context::ApplicationContext::shortcutFor(
            rodavarion::action::ActionType::Paste) != "CTRL+SHIFT+V") {
        std::cerr << "Terminal paste profile failed.\n";
        return EXIT_FAILURE;
    }

    qputenv("RODAVARION_ACTIVE_APP", "unknown-graphical-application");
    if (rodavarion::context::ApplicationContext::shortcutFor(
            rodavarion::action::ActionType::Copy) != "CTRL+C") {
        std::cerr << "Default application fallback failed.\n";
        return EXIT_FAILURE;
    }

    qunsetenv("RODAVARION_ACTIVE_APP");

    const auto normalized =
        rodavarion::input::MouseEventNormalizer::normalize(
            rodavarion::action::MouseButton::Gesture,
            true,
            0x116
        );

    if (normalized.control
        != rodavarion::input::DeviceControl::HapticMenuButton) {
        std::cerr << "Input normalization failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

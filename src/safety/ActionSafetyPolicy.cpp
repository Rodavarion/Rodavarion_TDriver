#include "rodavarion/safety/ActionSafetyPolicy.hpp"

#include <filesystem>

namespace rodavarion::safety {

SafetyDecision ActionSafetyPolicy::validateForDryRun(const action::MouseButtonMapping& mapping) {
    if ((mapping.action == action::ActionType::RunScript || mapping.action == action::ActionType::CustomShortcut) && mapping.parameter.empty())
        return {false, mapping.action == action::ActionType::RunScript ? "Не вказано шлях до скрипту." : "Не вказано комбінацію клавіш."};
    return {true, "Перевірка безпечна: обладнання не змінюється."};
}

SafetyDecision ActionSafetyPolicy::validateForRuntime(const action::MouseButtonMapping& mapping) {
    if (mapping.action != action::ActionType::RunScript) return {true, "Стандартна дія дозволена."};
    if (mapping.parameter.empty()) return {false, "Не вказано шлях до скрипту."};
    const std::filesystem::path p(mapping.parameter);
    if (!p.is_absolute()) return {false, "Шлях до скрипту має бути абсолютним."};
    if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p)) return {false, "Скрипт не існує або не є звичайним файлом."};
    if (std::filesystem::is_symlink(p)) return {false, "Символічні посилання для скриптів заборонені."};
    return {true, "Скрипт пройшов перевірку."};
}

} // namespace rodavarion::safety

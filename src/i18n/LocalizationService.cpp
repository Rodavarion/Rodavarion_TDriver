#include "rodavarion/i18n/LocalizationService.hpp"

#include <QSettings>
#include <QHash>

namespace rodavarion::i18n {

namespace {

const QHash<QString, QString>& uk() {
    static const QHash<QString, QString> values{
        {"app.title", "Rodavarion TDriver"},
        {"nav.peripherals", "Центр периферії"},
        {"nav.profiles", "Профілі"},
        {"nav.diagnostics", "Діагностика"},

        {"nav.maintenance", "Обслуговування"},
        {"nav.about", "Інформація"},
        {"maintenance.title", "Залежності та безпечне очищення"},
        {"maintenance.description", "Rodavarion TDriver відстежує лише ті пакети, які встановив сам. Після налаштування пристроїв тут можна прибрати невикористані компоненти, не торкаючись пакетів, що існували до встановлення програми. Якщо пізніше буде підключено новий пристрій, програма визначить відсутній профіль і запропонує довстановлення."},
        {"about.title", "Про програму"},
        {"about.company", "Rodavarion Technologies"},
        {"about.motto", "Rooted in Heritage. Engineered for the Future."},
        {"about.description", "Rodavarion TDriver — відкрите програмне забезпечення для Linux, яке наразі створюється ветераном-ентузіастом. Rodavarion Technologies перебуває на етапі створення. Проєкт розвивається з акцентом на безпеку, модульність, контроль користувача та довготривалу архітектуру."},
        {"about.version", "Версія: %1\nЛіцензія: MIT Open Source\nСтатус: громадська розробка"},
        {"page.peripherals", "Універсальний центр периферії"},
        {"page.profiles", "Постійні профілі"},
        {"page.diagnostics", "Діагностика"},
        {"button.scan", "Сканувати знову"},
        {"button.save", "Зберегти"},
        {"button.cancel", "Скасувати"},
        {"button.clear", "Очистити"},
        {"button.refresh_input", "Оновити пристрої вводу"},
        {"button.start_monitor", "Запустити живий монітор"},
        {"button.stop_monitor", "Зупинити живий монітор"},
        {"button.test", "Тест"},
        {"menu.settings", "Налаштування"},
        {"menu.language", "Мова"},
        {"language.auto", "Автоматично — за ОС"},
        {"language.uk", "Українська"},
        {"language.en", "English"},
        {"table.device", "Пристрій"},
        {"table.manufacturer", "Виробник"},
        {"table.class", "Клас"},
        {"table.transport", "Транспорт"},
        {"table.capabilities", "Можливості"},
        {"table.interfaces", "Інтерфейси"},
        {"table.backend", "Бекенд"},
        {"table.status", "Стан"},
        {"table.connection", "З'єднання"},
        {"table.vendor_product", "Vendor / Product"},
        {"table.serial", "Серійний номер"},
        {"table.system_path", "Системний шлях"},
        {"mouse.title", "Кнопки миші: призначення і тест лише для читання"},
        {"mouse.protected", "Захищений режим лише для читання: натискання реальної кнопки тільки вибирає та коротко підсвічує відповідний рядок. Програма не блокує пристрій, не створює віртуальний ввід, не записує HID-звіти й не виконує призначені дії або скрипти."},
        {"mouse.input_device", "Linux-пристрій вводу:"},
        {"mouse.monitor_stopped", "Монітор зупинено"},
        {"mouse.monitor_active", "Монітор лише для читання активний"},
        {"mouse.no_devices", "Пристрої /dev/input/event* не знайдено"},
        {"mouse.no_readable", "Знайдено пристрої, але немає доступних для читання"},
        {"mouse.select_device", "Оберіть пристрій вводу"},
        {"mouse.log", "Журнал тестування лише для читання"},
        {"mouse.permission_hint", "Немає доступу. Запустіть tools/install_input_monitor_udev_rule.sh, перепідключіть мишу та увійдіть у сеанс знову."},
        {"mouse.refresh_done", "Сканування завершено: знайдено %1 пристроїв, доступних для читання — %2."},
        {"mouse.started", "Безпечний монітор запущено: %1"},
        {"mouse.row_button", "# / Кнопка"},
        {"mouse.state", "Стан"},
        {"mouse.action", "Дія"},
        {"mouse.parameter", "Комбінація / скрипт"},
        {"mouse.dry_test", "Сухий тест"},
        {"state.inferred", "Передбачено"},
        {"state.unavailable", "Недоступна"},
        {"state.confirmed", "Підтверджено наживо"},
        {"state.readable", "доступний"},
        {"state.permission_denied", "немає доступу"},
        {"state.not_mouse", "інший пристрій вводу"},
        {"profile.name", "Назва профілю:"},
        {"profile.pointer_speed", "Швидкість вказівника:"},
        {"profile.scroll_mode", "Режим прокручування:"},
        {"profile.save", "Зберегти профіль"},
        {"status.ready", "Готово."},
        {"status.peripherals_found", "Знайдено периферійних пристроїв: %1"},
        {"status.mouse_only", "Призначення дій поки доступне лише для мишей."},
        {"diagnostics.summary", "Ядро: ініціалізовано\nGUI: Qt 6 Widgets\nМодель периферії: універсальна\nПоточний бекенд: HIDAPI + тестовий провайдер\nПлановані бекенди: CUPS, SANE, evdev, serial, network\nФізичне групування: увімкнено\nКласифікатор можливостей: увімкнено\nКласифікатор транспорту: увімкнено\nПостійні профілі: увімкнено\nPlugin API: v1\n\n"}
    };
    return values;
}

const QHash<QString, QString>& en() {
    static const QHash<QString, QString> values{
        {"app.title", "Rodavarion TDriver"},
        {"nav.peripherals", "Peripheral Center"},
        {"nav.profiles", "Profiles"},
        {"nav.diagnostics", "Diagnostics"},

        {"nav.maintenance", "Maintenance"},
        {"nav.about", "Information"},
        {"maintenance.title", "Dependencies and safe cleanup"},
        {"maintenance.description", "Rodavarion TDriver tracks only packages installed by the application. After devices are configured, unused components can be removed without touching packages that existed beforehand. When a new device is connected later, the application detects missing profiles and offers to install them."},
        {"about.title", "About"},
        {"about.company", "Rodavarion Technologies"},
        {"about.motto", "Rooted in Heritage. Engineered for the Future."},
        {"about.description", "Rodavarion TDriver is open-source Linux software currently developed by a veteran enthusiast. Rodavarion Technologies is in the process of being established. The project focuses on safety, modularity, user control, and a durable architecture."},
        {"about.version", "Version: %1\nLicense: MIT Open Source\nStatus: community development"},
        {"page.peripherals", "Universal Peripheral Center"},
        {"page.profiles", "Persistent profiles"},
        {"page.diagnostics", "Diagnostics"},
        {"button.scan", "Scan again"},
        {"button.save", "Save"},
        {"button.cancel", "Cancel"},
        {"button.clear", "Clear"},
        {"button.refresh_input", "Refresh input devices"},
        {"button.start_monitor", "Start live monitor"},
        {"button.stop_monitor", "Stop live monitor"},
        {"button.test", "Test"},
        {"menu.settings", "Settings"},
        {"menu.language", "Language"},
        {"language.auto", "Automatic — system locale"},
        {"language.uk", "Українська"},
        {"language.en", "English"},
        {"table.device", "Device"},
        {"table.manufacturer", "Manufacturer"},
        {"table.class", "Class"},
        {"table.transport", "Transport"},
        {"table.capabilities", "Capabilities"},
        {"table.interfaces", "Interfaces"},
        {"table.backend", "Backend"},
        {"table.status", "Status"},
        {"table.connection", "Connection"},
        {"table.vendor_product", "Vendor / Product"},
        {"table.serial", "Serial number"},
        {"table.system_path", "System path"},
        {"mouse.title", "Mouse buttons: mapping and read-only test"},
        {"mouse.protected", "Protected read-only mode: pressing a real mouse button only selects and briefly highlights the matching row. The program does not grab the device, create virtual input, write HID reports, or execute mapped actions or scripts."},
        {"mouse.input_device", "Linux input device:"},
        {"mouse.monitor_stopped", "Monitor stopped"},
        {"mouse.monitor_active", "Read-only monitor active"},
        {"mouse.no_devices", "No /dev/input/event* devices found"},
        {"mouse.no_readable", "Devices found, but none are readable"},
        {"mouse.select_device", "Select an input device"},
        {"mouse.log", "Read-only test log"},
        {"mouse.permission_hint", "Permission denied. Run tools/install_input_monitor_udev_rule.sh, reconnect the mouse, and sign in again."},
        {"mouse.refresh_done", "Scan completed: %1 devices found, %2 readable."},
        {"mouse.started", "Safe monitor started: %1"},
        {"mouse.row_button", "# / Button"},
        {"mouse.state", "State"},
        {"mouse.action", "Action"},
        {"mouse.parameter", "Shortcut / script"},
        {"mouse.dry_test", "Dry test"},
        {"state.inferred", "Inferred"},
        {"state.unavailable", "Unavailable"},
        {"state.confirmed", "Confirmed live"},
        {"state.readable", "readable"},
        {"state.permission_denied", "permission denied"},
        {"state.not_mouse", "other input device"},
        {"profile.name", "Profile name:"},
        {"profile.pointer_speed", "Pointer speed:"},
        {"profile.scroll_mode", "Scroll mode:"},
        {"profile.save", "Save profile"},
        {"status.ready", "Ready."},
        {"status.peripherals_found", "Peripherals found: %1"},
        {"status.mouse_only", "Action mapping is currently available for mice only."},
        {"diagnostics.summary", "Core: initialized\nGUI: Qt 6 Widgets\nPeripheral model: universal\nCurrent backend: HIDAPI + mock provider\nPlanned backends: CUPS, SANE, evdev, serial, network\nPhysical grouping: enabled\nCapability classifier: enabled\nTransport classifier: enabled\nPersistent profiles: enabled\nPlugin API: v1\n\n"}
    };
    return values;
}

} // namespace

LocalizationService& LocalizationService::instance() {
    static LocalizationService service;
    return service;
}

void LocalizationService::load() {
    QSettings settings;
    configured_ = static_cast<Language>(
        settings.value("ui/language", static_cast<int>(Language::Automatic)).toInt()
    );
}

void LocalizationService::setLanguage(const Language language) {
    configured_ = language;
    QSettings settings;
    settings.setValue("ui/language", static_cast<int>(language));
}

Language LocalizationService::configuredLanguage() const noexcept {
    return configured_;
}

Language LocalizationService::effectiveLanguage() const noexcept {
    if (configured_ != Language::Automatic) {
        return configured_;
    }
    return QLocale::system().language() == QLocale::Ukrainian
        ? Language::Ukrainian
        : Language::English;
}

QString LocalizationService::text(const QString& key) const {
    const auto& map = effectiveLanguage() == Language::Ukrainian ? uk() : en();
    return map.value(key, key);
}

QString LocalizationService::languageName(const Language language) const {
    switch (language) {
        case Language::Automatic: return text("language.auto");
        case Language::Ukrainian: return text("language.uk");
        case Language::English: return text("language.en");
    }
    return {};
}

} // namespace rodavarion::i18n

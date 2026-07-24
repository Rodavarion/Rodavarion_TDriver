# Changelog

## 1.1.6 Stable

- Restored live input monitor lifecycle.
- Added tray recovery after suspend/resume and Plasma shell restart.
- Added explicit device button-mapping actions while retaining double-click navigation.
- Expanded HID classification and ASUS ROG Gladius III Wireless AimPoint support.
- Added per-monitor scaling profile protection for KDE Plasma/KScreen.
- Improved clean installation, dependency cleanup and full uninstall.
- Removed automatic desktop shortcut creation.

## 1.1.5 Stable

- Corrected release numbering after the mistakenly published 1.0.1 package.
- Fixed disabled side-button suppression and unknown-control handling.
- Cached mouse mappings in the runtime hot path.
- Removed one proven obsolete duplicate udev rule.
- Added a repository linkage and cleanup audit.

## 1.0.1 — Runtime Correctness Hotfix

- Fixed `DeviceControl::Unknown` so unsupported events are rejected instead of being interpreted as the left mouse button.
- Fixed `Disabled` mappings so side-button events are suppressed rather than passed through to the desktop.
- Preserved the native system behavior for mappings set to `Default`.
- Added an in-memory mappings cache that reloads only when the mappings file, device key, modification time, or file size changes.
- Added clearer runtime diagnostics for `Default`, `Disabled`, and rejected unknown events.
- Added a testable `ActionEngine` constructor with an explicit mappings-file path.

## 1.0.0 — Stable

- Promoted the verified RC1.2 architecture to the first stable release.
- Preserved the shared GUI/daemon configuration path and automatic profile migration.
- Preserved the single-owner mouse capture model that prevents duplicate grabs and browser Back/Forward leakage.
- Preserved terminal shortcuts (`Ctrl+Shift+C` / `Ctrl+Shift+V`), browser shortcuts (`Ctrl+C` / `Ctrl+V`) and the default GUI fallback profile.
- Preserved transactional upgrades, safe uninstall, autostart-to-tray, Open Source information and donation controls.
- Added stable-release validation notes without changing user settings, profile identifiers or persistence paths.

## 1.0.0-rc1.2 — KWin Context Bridge

- Вбудований KWin 6 script передає активне вікно через D-Bus.
- KDE Plasma Wayland більше не залежить від kdotool.
- Контекстні дії не відправляють небезпечний Ctrl+V, якщо активне вікно не визначено.

# Changelog

## 1.0.0-rc1.2 — Shared Context Hotfix

- GUI and background daemon now read and write one canonical application-context profile file.
- Existing profiles are migrated from the legacy service-specific configuration directory.
- The daemon now uses the same Qt application identity as the GUI.
- Terminal mappings saved in the GUI become visible to the daemon without a separate copy of settings.
- Preflight now guards against reintroducing a separate daemon configuration identity.
- Retains all Open Source Edition features from Beta 8.

## 1.0.0-beta7

- Автоматичний запуск GUI після входу в систему через XDG Autostart.
- Запуск за замовчуванням лише у системному треї з останніми налаштуваннями.
- Нова вкладка «Загальні» для керування автозапуском і відновленням стану.
- Збереження геометрії вікна та останньої відкритої вкладки.
- Безпечне додавання й видалення файла автозапуску.
- Деінсталятор видаляє зареєстрований компонент автозапуску.

## 1.0.0-beta6.2 — Mouse Action Dialog Compile Fix

- Added visible operation status, activity indicator, and last-run timestamp on the Maintenance page.
- Maintenance buttons now always explain whether an action completed, was cancelled, failed, or required no changes.
- Buttons are temporarily locked during an operation and show action-specific progress text.
- Preserved safe dependency installation and cleanup rules.

## 1.0.0-beta5 — Terminal Context & Script Picker Fix

- Виправлено кнопку вибору виконуваного скрипту: вона активна лише для дії «Запустити скрипт», відкриває файловий діалог, перевіряє файл і попереджає про відсутність права виконання.
- Розпізнавання активної програми тепер враховує PID, клас і заголовок вікна через kdotool/xdotool.
- Додано сумісність зі старими власними комбінаціями Ctrl+C/Ctrl+V: у терміналах вони автоматично стають Ctrl+Shift+C/Ctrl+Shift+V.
- Діагностика показує активне вікно та фактично застосований профіль.

# Журнал змін

## 1.0.0-beta3 — Clean namespace and migration boundary

- виправлено розсинхронізацію `MaintenanceAudit` і `DependencyMaintenanceManager`;
- остаточно перейменовано C++ namespace, include-шляхи, CMake-цілі, макроси, журнали пакетів і системні ресурси на Rodavarion;
- поточний інсталятор працює лише з `rodavarion-tdriver` і не містить перевірок VeteranUA;
- одноразове очищення попереднього бренду винесено в окремий захищений скрипт;
- версія GUI, daemon, конфігурації та інсталятора береться з єдиного файла `VERSION`.

# 1.0.0 Alpha 5 — Safe Relay, Desktop Shortcut and Tray

- Added exclusive grab-and-relay mode for mouse interfaces where side buttons share the same evdev node as pointer movement and primary buttons.
- Suppresses original Back/Forward browser events while relaying normal mouse movement, wheel and primary buttons through a virtual uinput device.
- Added a generated Rodavarion shield icon.
- Added application-menu and desktop shortcuts during full-edition installation.
- Added system tray behavior: closing the window hides it to tray.
- Added tray menu actions to open, hide, restart mouse processing, show status and exit.
- Added udev access rule for Logitech evdev nodes and /dev/uinput.
- Fixed install_udev_rules.sh referencing a missing rules file.

# Changelog

## 1.0.0 Alpha 4

- Expanded Full edition with useful mainstream peripheral packages.
- Added package ownership ledger.
- Added enabled-feature state.
- Added device-to-dependency-profile detection.
- Added the Maintenance GUI page.
- Added missing dependency installation from the GUI.
- Added conservative removal of unused Rodavarion-owned packages.
- Protected all pre-existing packages from cleanup.
- Protected core/system/build packages.
- Refused cleanup when pacman reports reverse dependencies.
- Changed cleanup to non-recursive `pacman -R`.
- Added automatic ledger updates after feature installation.
- Added dependency lifecycle documentation.

## 1.0.0 Alpha 6 — Safe Upgrade & Uninstall

- Added explicit replacement prompt when an installed version is detected.
- Added per-user installation state and an allow-listed managed-file manifest.
- Updates preserve settings, profiles, logs, and package ledger.
- Added autonomous installed uninstaller and application-menu entry.
- Default uninstall removes only program components and preserves user data.
- Optional `--purge-data` removes only Rodavarion TDriver-owned data directories.
- Optional system cleanup validates SHA-256 hashes before removing udev/modules-load files.
- Arch packages are never removed automatically because other applications may depend on them.

## 1.0.0-alpha7 — One-step installer

- Додано кореневий `install.sh`: повний цикл встановлення однією командою.
- `preflight_arch.sh` залишається окремою розробницькою перевіркою та виконує чисту збірку і тести.
- `install_user.sh` більше не запускає CMake, компіляцію або тести повторно.
- Після успішної збірки створюється маркер версії, редакції та проходження тестів.
- Для бінарних артефактів записуються SHA-256 контрольні суми.
- Інсталяція зупиняється, якщо збірка застаріла, має неправильну редакцію або була змінена після тестування.

## 1.0.0 Alpha 8 — Transactional Upgrade & Menu Polish

- Надійне завершення попереднього GUI через перевірку `/proc/<pid>/exe`; усунено подвійні значки в треї після оновлення.
- Додано блокування другого екземпляра GUI через `QLockFile`.
- Оновлення порівнює старий і новий реєстри файлів та видаляє лише застарілі керовані компоненти.
- Версія у вікні «Інформація», заголовку та журналі формується з єдиного файла `VERSION`.
- Додано окремий значок деінсталяції, покращений ярлик програми та XDG-підменю Rodavarion TDriver.

## 1.0.0-alpha9 — Polished desktop integration

- Додано PNG-іконки 16, 24, 32, 48, 64, 128 і 256 px поряд із SVG.
- Ярлик на робочому столі отримує абсолютний шлях до PNG-іконки, що усуває білий стандартний значок у KDE Plasma.
- Інсталятор позначає desktop-ярлик як довірений через `gio`, коли це підтримується.
- Оновлення кешу іконок і бази desktop-файлів виконується автоматично.
- Деінсталятор видаляє тільки зареєстровані іконки Rodavarion TDriver усіх установлених розмірів.

## 1.0.0-beta1 — Verified desktop integration

- Ярлики тепер генеруються з абсолютними шляхами `Exec` та `Icon`.
- Додано канонічний каталог ресурсів `~/.local/share/rodavarion-tdriver/resources/icons`.
- При оновленні видаляється старий ярлик `Rodavarion TDriver.desktop`, який Plasma могла показувати як білий документ.
- Новий ярлик має безпечне ім’я `Rodavarion-TDriver.desktop` без пробілів у назві файла.
- Додано обов’язкову післяінсталяційну перевірку виконуваних файлів, іконок та Desktop Entry.
- Додано оновлення кешу KDE через `kbuildsycoca6`/`kbuildsycoca5`, якщо команда доступна.
- Встановлення більше не повідомляє про успіх, якщо ярлик або його іконка не пройшли перевірку.

## 1.0.0-beta4 — Smart Context & IDE Profiles
- Automatic terminal-aware Copy/Paste: Ctrl+Shift+C / Ctrl+Shift+V.
- Added configurable application-context profiles.
- Added built-in profiles for Konsole and common terminals, VS Code/VSCodium, JetBrains IDEs, Qt Creator, Visual Studio, Eclipse, and Neovim GUIs.
- Added “Програми та IDE” settings page with editable shortcuts and process matching.
- Added best-effort active-window detection through kdotool or xdotool, with safe fallback to standard shortcuts.

## 1.0.0-beta8.2 — Direct UInput Core

- Added an in-process virtual keyboard backend based directly on `/dev/uinput`.
- Removed graphical-session and ydotool socket state from the primary key-injection path.
- Kept ydotool, wtype and xdotool only as fallback backends.
- Extended diagnostics to report the built-in uinput keyboard state.
- Terminal profiles continue to map Copy/Paste to Ctrl+Shift+C / Ctrl+Shift+V.

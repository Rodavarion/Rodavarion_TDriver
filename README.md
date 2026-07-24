<div align="center">

<img src="packaging/icons/rodavarion-tdriver.svg" alt="Rodavarion TDriver logo" width="128" height="128">

# Rodavarion TDriver

**Context-aware peripheral and mouse-button management for Linux**
**Контекстне керування периферією та кнопками миші для Linux**

[![Linux CI](https://github.com/Rodavarion/Rodavarion_TDriver/actions/workflows/linux-build.yml/badge.svg?branch=main)](https://github.com/Rodavarion/Rodavarion_TDriver/actions/workflows/linux-build.yml)
[![Release](https://img.shields.io/github/v/release/Rodavarion/Rodavarion_TDriver?display_name=tag&sort=semver)](https://github.com/Rodavarion/Rodavarion_TDriver/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](CMakeLists.txt)
[![Qt 6](https://img.shields.io/badge/Qt-6-41CD52.svg)](CMakeLists.txt)
[![Platform: Linux](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](#system-requirements--системні-вимоги)

[Installation](#installation--встановлення) · [Architecture](docs/ARCHITECTURE.md) · [Roadmap](ROADMAP.md) · [Contributing](CONTRIBUTING.md) · [Security](docs/SECURITY.md) · [Українська](#український-опис)

</div>

> **Stable release:** 1.1.6
> **License:** MIT
> **Primary platform:** Arch Linux; the core is designed for broader Linux support.

Rodavarion TDriver is an open-source C++20/Qt 6 platform for detecting Linux peripherals, classifying their capabilities, managing context-aware mouse mappings, and safely maintaining optional device dependencies.

## Highlights · Основні можливості

- Physical-device discovery and grouping across HID, USB, Bluetooth and serial transports.
- Context-aware mouse actions for terminals, IDEs and desktop applications.
- Native keyboard-output backend through `/dev/uinput`, with Wayland/X11 fallbacks.
- Conservative dependency maintenance backed by a package ledger.
- Qt Widgets GUI, background daemon, CLI, tests and Linux packaging assets.
- Arch Linux installation scripts with explicit preflight, build, test and uninstall stages.

## Installation · Встановлення

### Arch Linux — stable installation

```bash
chmod +x install.sh tools/*.sh
./install.sh --edition full
```

Minimal edition:

```bash
./install.sh --edition minimal
```

### Developer build

```bash
chmod +x tools/*.sh
./tools/preflight_arch.sh --edition full
./tools/build_arch.sh
./tools/run_arch.sh
```

## System requirements · Системні вимоги

- Linux with a graphical session;
- CMake 3.22 or newer;
- C++20 compiler;
- Qt 6.5 or newer (`Core`, `DBus`, `Widgets`);
- HIDAPI and libevdev;
- systemd/udev and Polkit for full desktop integration.

## Repository structure · Структура репозиторію

| Path | Purpose |
|---|---|
| `include/rodavarion/` | Public C++ interfaces |
| `src/` | Core, runtime, GUI, daemon and CLI implementations |
| `tests/` | Automated tests |
| `tools/` | Arch Linux development and installation scripts |
| `packaging/` | Desktop, systemd, udev, icons and KWin assets |
| `docs/` | Architecture, security and dependency documentation |
| `.github/` | CI, release automation and contribution templates |

## Project status · Стан проєкту

Version **1.0.1 Stable** is the current public release; it contains critical runtime correctness and latency fixes over 1.0.0. Development is currently led by **DevSwiftPro**, a Ukrainian veteran enthusiast. Rodavarion Technologies is being established. Contributions and responsible issue reports are welcome.

## Український опис

Rodavarion TDriver — відкрита платформа на C++20 і Qt 6 для виявлення периферійних пристроїв Linux, визначення їхніх можливостей, контекстного перепризначення кнопок миші та безпечного керування додатковими залежностями.

---

## Detailed documentation · Детальна документація

## Full edition тепер справді «повна»

Повна редакція заздалегідь готує основні класи периферії:

- HID та USB;
- Wayland/X11 action runtime;
- CUPS, фільтри друку, Ghostscript, PDF-інструменти;
- SANE, AirScan та старі паралельні сканери;
- Bluetooth;
- мережеве виявлення через Avahi;
- smart card / RFID через PC/SC;
- камера й аудіо через V4L2 та PipeWire;
- базову системну діагностику.

Рідкісні proprietary/vendor-пакети не встановлюються наперед.

## Головний захист від видалення чужих пакетів

Під час bootstrap Rodavarion TDriver записує **лише пакети, яких не було
до початку встановлення і які встановила саме наша процедура**:

```text
~/.local/share/rodavarion-tdriver/package-ledger.arch.json
```

Кнопка **«Видалити безпечний баласт»** розглядає лише цей журнал.

Пакет не можна видалити, якщо він:

- існував до Rodavarion TDriver;
- потрібний активному профілю пристрою;
- входить до системного захищеного списку;
- має інші залежні пакети;
- не був зафіксований у журналі нашого інсталятора.

Для очищення використовується `pacman -R`, а не агресивний `-Rns`.

## Нова вкладка «Обслуговування»

У Full GUI додано сторінку:

- перевірка залежностей;
- список відсутніх потрібних пакетів;
- список безпечно доступного баласту;
- кнопка `Довстановити потрібне`;
- кнопка `Видалити безпечний баласт`.

Усі адміністративні операції проходять через стандартний Polkit.

## Автовиявлення нового обладнання

Після сканування пристроїв програма активує профілі:

- принтер / термопринтер / етикетки → `printing`;
- документний або штрихкодовий сканер → `scanning`;
- Bluetooth transport → `bluetooth`;
- Serial transport → `serial`;
- базові HID-пристрої → `runtime-core`.

Якщо потрібних пакетів немає, вкладка обслуговування показує їх і
пропонує довстановлення. Це залишає архітектуру готовою до майбутнього
автоматичного onboarding-майстра.

## Встановлення Full — одна команда

```bash
chmod +x install.sh tools/*.sh
./install.sh
```

Або з явним зазначенням редакції:

```bash
./install.sh --edition full
```

Інсталятор сам готує систему, виконує чисту збірку, запускає тести й
встановлює саме перевірені файли. Повторної збірки на етапі встановлення немає.

## Встановлення Minimal

```bash
chmod +x install.sh tools/*.sh
./install.sh --edition minimal
```

## Окремі команди для розробника

Лише перевірити, зібрати й протестувати без встановлення:

```bash
./tools/preflight_arch.sh --edition full
```

Встановити вже готову перевірену збірку з `build/` без повторної компіляції:

```bash
./tools/install_user.sh --edition full
```

`install_user.sh` відмовиться працювати, якщо збірка відсутня, має іншу
версію чи редакцію, не пройшла тести або її контрольні суми змінилися.

## Важлива межа

Неможливо гарантувати, що пакет абсолютно не використовується вручну
користувачем. Тому очищення навмисно надзвичайно консервативне:
видаляються лише наші пакети без системних reverse dependencies.
Користувач завжди бачить точний список і окремо підтверджує операцію.

## Встановлення

Для повної редакції на Arch Linux достатньо однієї команди:

```bash
./install.sh
```

Alpha 9 встановлює SVG та PNG-іконки різних розмірів, оновлює кеш іконок і створює довірений ярлик на робочому столі з абсолютним шляхом до іконки.


## Beta 1: перевірена інтеграція з робочим столом

Інсталятор створює ярлик `Rodavarion-TDriver.desktop` з абсолютними шляхами до програми та PNG-іконки. Після копіювання він перевіряє права, поля `Exec`/`Icon`, існування ресурсів та, за наявності, запускає `desktop-file-validate`.


## Контекстні профілі програм (Beta 5)

Вкладка **«Програми та IDE»** дозволяє змінювати комбінації для терміналів та відомих IDE.
Типовий профіль термінала використовує `Ctrl+Shift+C` і `Ctrl+Shift+V`, щоб не надсилати `Ctrl+C` активному процесу.
Вбудовані профілі охоплюють Konsole та інші термінали, Visual Studio Code/VSCodium, JetBrains IDE, Qt Creator, Visual Studio, Eclipse і графічні клієнти Vim/Neovim.
Визначення активної програми виконується через `kdotool` (за наявності) або `xdotool`; якщо активну програму визначити неможливо, застосовуються звичайні глобальні комбінації.


## Автозапуск

Після встановлення Rodavarion TDriver автоматично запускається після входу в графічний сеанс і залишається у системному треї. Поведінку можна змінити на вкладці «Загальні».

## Direct keyboard backend (Beta 8.2)

Keyboard shortcuts are now emitted by Rodavarion TDriver itself through a dedicated virtual keyboard on `/dev/uinput`. This is the primary backend on both Wayland and X11. `ydotool`, `wtype`, and `xdotool` remain fallback options only. The terminal context profile maps Copy/Paste to `Ctrl+Shift+C` and `Ctrl+Shift+V`.


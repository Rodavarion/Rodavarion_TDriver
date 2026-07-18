# Rodavarion TDriver 1.0.0 Stable


Context-aware peripheral and mouse-button management for Linux.

**Open Source · MIT License · Made in Ukraine**

Rodavarion TDriver is currently developed by a **veteran enthusiast**. Rodavarion Technologies is in the process of being established. See [SUPPORT.md](SUPPORT.md) for voluntary support and other ways to contribute.

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

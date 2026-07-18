# Актуальна архітектура

```text
Device Provider
    ↓
Input Backend (HIDAPI / evdev / future CUPS, SANE, serial)
    ↓
Normalizer
    ↓
InputEvent + DeviceControl
    ↓
Action Engine
    ↓
Output Engine (Wayland / X11 / safe script runner)
```

## Dependency architecture

```text
Clean Linux
    ↓
Universal bootstrap manifest
    ├── base
    ├── printing
    ├── scanning
    └── bluetooth

Connected device
    ↓
Capability / backend check
    ↓
Optional allow-listed profile
    └── serial / rare vendor driver
```

Ні GUI, ні скрипт не можуть передавати довільну назву пакета привілейованому
інсталятору. Усі профілі визначені в контрольованому маніфесті.

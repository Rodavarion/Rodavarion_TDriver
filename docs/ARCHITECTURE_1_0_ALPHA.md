# Rodavarion TDriver 1.0 Alpha — Runtime architecture

```text
Physical device
  ↓
Input backend (evdev / future HID++ / future libinput)
  ↓
MouseEventNormalizer
  ↓
InputEvent + DeviceControl
  ↓
ActionEngine
  ├── mapping lookup
  ├── safety policy
  └── action dispatch
  ↓
IOutputEngine
  ↓
DesktopOutputEngine
  ├── ydotool / Wayland
  ├── xdotool / X11
  └── controlled script executor
```

The GUI no longer needs Linux event codes. Future keyboards, gamepads,
barcode scanners and other devices can provide normalized `InputEvent`
objects and reuse the same Action Engine and output adapters.

## Safety boundaries

- Input monitoring is read-only unless a narrowly verified auxiliary button
  interface is exclusively remapped.
- Cursor and primary-button interfaces are never exclusively grabbed.
- Hardware firmware and vendor configuration are never written.
- Action execution passes through `ActionSafetyPolicy`.
- Privileged dependency setup remains allow-listed and Polkit-confirmed.

# Rodavarion TDriver 1.0.1 Stable

Rodavarion TDriver 1.0.1 is a focused stability release for the mouse runtime pipeline. It keeps the 1.0 public interface and persistence format while correcting event handling and removing unnecessary file I/O from the hot input path.

## Fixed

- Unknown device controls are now rejected safely and can no longer fall back to the left mouse button.
- Side buttons configured as `Disabled` are now actually suppressed.
- Side buttons configured as `Default` continue to use their native desktop behavior.
- Mouse mappings are cached in memory and reloaded only when the configuration changes.
- Runtime status messages now distinguish system-default behavior, disabled buttons, and rejected events.

## Compatibility

- Existing settings and `mouse-mappings.json` files remain compatible.
- No migration is required from 1.0.0.
- Linux and KDE Plasma Wayland support remain unchanged.

## Verification

Before publishing, run the standard Arch Linux preflight and verify `Disabled`, `Default`, and live mapping reload behavior on a real device.

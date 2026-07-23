# Rodavarion TDriver 1.0.1 Stable release checks

## Build gate

1. Parse all JSON configuration files.
2. Validate all shell scripts with `bash -n`.
3. Configure a clean CMake build.
4. Build every target.
5. Run all CTest tests; installation must stop on any failure.

## Regression gate

- Konsole paste resolves to `Ctrl+Shift+V`.
- Firefox/Chromium paste resolves to `Ctrl+V`.
- Unknown graphical applications resolve to the default GUI profile.
- Old profile files gain missing `browser` and `default` profiles without losing user-defined profiles.
- Only one normal runtime owner may request `EVIOCGRAB`.
- Native mouse Back/Forward events do not leak while remapped.

## Installer gate

- Existing GUI and daemon are stopped before replacement.
- Obsolete files are removed only from the previous install manifest.
- User settings, profiles and logs are preserved during upgrade.
- Desktop entry, icon, autostart entry, service and uninstall entry are validated before success is reported.

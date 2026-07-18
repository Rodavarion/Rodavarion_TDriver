# 1.0.0-rc1 — superseded context bridge notes

- Added a bundled KWin 6 active-window bridge over the user D-Bus.
- Removed the mandatory KDE dependency on `kdotool`.
- Added a daemon-side D-Bus context endpoint.
- Made Copy/Paste/Cut/Undo/Redo/Select All fail safely when the active
  application cannot be determined instead of sending a generic shortcut.
- Added installer and uninstaller handling for the KWin script.

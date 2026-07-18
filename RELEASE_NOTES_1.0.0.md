# Rodavarion TDriver 1.0.0 Stable

This release freezes the working RC1.2 architecture as the first stable version.

## Stable guarantees

- One daemon owns the physical mouse during normal operation.
- The live monitor temporarily and explicitly takes ownership only while the mapping dialog is open.
- Browser profiles use `Ctrl+C` and `Ctrl+V` and suppress native Back/Forward events.
- Terminal profiles use `Ctrl+Shift+C` and `Ctrl+Shift+V`.
- Older context configuration files are migrated by adding missing system profiles while preserving user changes.
- GUI, daemon and CLI use the same persistent configuration identity and path.
- Upgrades stop the previous process, remove registered obsolete files and preserve user data.
- Uninstall removes only registered Rodavarion TDriver components unless the user explicitly requests data purge.

## Compatibility

The stable release intentionally keeps the same organization name, application name, settings keys and profile IDs as RC1.2 so existing preferences remain available after upgrade.

# Repository cleanup audit — TDriver 1.1.5

## Method

The repository was checked through CMake source lists, include references, shell-script call chains, packaging installers, systemd/desktop integration, and uninstall manifests.

## Confirmed removal

`packaging/udev/70-rodavarion-logitech-input-monitor.rules`

Reason: no build, installer, documentation, or packaging path referenced it. Its only rule was duplicated exactly by the actively installed `packaging/udev/70-rodavarion-input.rules`, which additionally grants the required uinput access.

## Retained intentionally

- `tools/clean.sh`: standalone safe build cleanup command.
- `tools/install_feature_arch.sh`: optional dependency-profile installer.
- `tools/remove_gui_keep_service.sh`: supported transition from full GUI installation to service-only operation.
- `tools/install_udev_rules.sh`: focused manual recovery/install command for udev access.
- historical beta/RC changelogs: release history, not runtime code.
- desktop, menu, icon, autostart, KWin, systemd, and uninstall assets: all consumed by installation or removal workflows.

## Code linkage result

- No orphaned `.cpp` implementation files were found.
- No public `.hpp` header without a consumer was found.
- GUI, daemon, CLI, runtime, core, and test entry points are all represented in CMake.

No other file was removed because its lack of an internal caller alone did not prove it unnecessary; several scripts are deliberate user-facing entry points.

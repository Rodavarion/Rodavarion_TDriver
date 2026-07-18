# KWin Context Bridge

Rodavarion TDriver installs a small KWin 6 script named `rodavarion-context`.
It observes `workspace.activeWindow` and `workspace.windowActivated`, then sends
`resourceClass`, `resourceName`, `desktopFileName`, and `caption` to the
Rodavarion daemon through the user session D-Bus.

This removes the mandatory dependency on `kdotool` in KDE Plasma Wayland.
If no trusted context is available, context-sensitive actions such as Copy and
Paste are not emitted; the daemon does not silently fall back to Ctrl+V.

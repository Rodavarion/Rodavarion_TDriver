# Rodavarion TDriver 1.0.0 RC1 — Core Rebuild

- Fixed GUI live monitor ownership: the daemon is stopped while the monitor dialog is open and restarted afterwards.
- Added safe single-profile fallback when GUI and daemon expose different interface keys for the same physical mouse.
- Removed delayed action dispatch that caused application-context races.
- Reworked direct uinput keyboard output into atomic modifier/key frames.
- Preserved exclusive side-button suppression while relaying pointer movement, wheel and primary buttons.
- Kept settings, Open Source information, autostart, tray and installer behavior from Beta 8.3.1.

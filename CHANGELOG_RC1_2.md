# Rodavarion TDriver 1.0.0 RC1.2

- Fixed application-context configuration migration during upgrades.
- Existing user-edited profiles are preserved.
- Newly introduced built-in browser and default profiles are appended when absent.
- Prevented test/portable config overrides from importing live user configuration.
- Added a regression test using a legacy settings file without browser/default profiles.

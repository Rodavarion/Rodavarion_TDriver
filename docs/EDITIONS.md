# Product editions

## Minimal / Headless
Build flags:

```text
RODAVARION_BUILD_GUI=OFF
RODAVARION_BUILD_DAEMON=ON
RODAVARION_BUILD_CLI=ON
```

Purpose:
- servers;
- embedded-like Linux hosts;
- low-memory systems;
- background-only execution.

## Full GUI
Build flags:

```text
RODAVARION_BUILD_GUI=ON
RODAVARION_BUILD_DAEMON=ON
RODAVARION_BUILD_CLI=ON
```

Purpose:
- desktop configuration;
- diagnostics;
- device onboarding;
- profile editing.

## Packaging model
The daemon is the stable runtime package.
The GUI is an optional frontend package.

Removing the frontend must not remove the daemon or user profiles.

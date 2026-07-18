# Persistence and removal behavior

## Software persistence
Mappings persist while `rodavarion-tdriverd` is installed and enabled.

The daemon is independent of the GUI, so GUI removal does not stop
configured actions.

## Complete removal
After complete removal:
- the native Linux driver remains;
- basic device behavior remains;
- Rodavarion software mappings stop.

This is intentional fail-safe behavior.

## Onboard device memory
True persistence without any installed software is possible only when
hardware provides onboard writable memory.

Future vendor plugins may support this under strict conditions:
- read-only capability detection first;
- explicit user confirmation;
- configuration backup;
- verified write transaction;
- rollback;
- never write firmware blindly.

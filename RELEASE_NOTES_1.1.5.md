# Rodavarion TDriver 1.1.5 Stable

Rodavarion TDriver 1.1.5 is the corrected stable release following the previously published 1.1.2 package and supersedes the mistakenly numbered 1.0.1 release.

## Runtime correctness

- `Disabled` mappings now suppress the physical side-button action correctly.
- Unknown controls are ignored instead of being interpreted as the left mouse button.
- Mapping configuration is cached and refreshed when its file changes, removing repeated JSON disk I/O from the input hot path.
- Default and disabled actions retain distinct runtime semantics.

## Repository cleanup

- Removed the obsolete `70-rodavarion-logitech-input-monitor.rules` file.
- The removed rule was not referenced by any installer or build path and duplicated the Logitech access rule already present in `70-rodavarion-input.rules`.
- Verified that every C++ implementation is included in CMake and every public header has a source consumer.
- Retained manual maintenance scripts because each implements a distinct supported operation.

## Validation

- Clean Arch Linux build completed successfully.
- All automated tests passed.
- Public archives include SHA-256 checksums.

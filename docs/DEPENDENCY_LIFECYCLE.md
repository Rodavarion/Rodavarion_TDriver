# Dependency lifecycle

## 1. Install
The bootstrap records only packages absent before installation.

## 2. Claim
Device discovery claims feature profiles such as printing, scanning,
Bluetooth, serial, or runtime-core.

## 3. Audit
The maintenance manager compares:
- active profiles;
- required packages;
- installed packages;
- the Rodavarion package ledger;
- protected packages;
- pacman reverse dependencies.

## 4. Safe cleanup
A package is removable only when every condition is true:
- installed by Rodavarion;
- no longer required;
- not protected;
- installed now;
- no reverse dependencies.

## 5. Re-onboarding
When a new device claims a removed profile, missing packages are offered
for installation and recorded again in the ledger.

## Future improvement
Stable native packages should use split packages and package-manager
metadata. The ledger remains an additional ownership guard, not a
replacement for Arch packaging.

# Beta 8 release checks

Completed in the packaging environment:

- shell syntax validation for all `.sh` files;
- JSON parsing for every configuration file;
- version consistency checks for `VERSION` and default configuration;
- source-level checks for the donation URL, card-copy action, welcome-dialog persistence, and method declaration/definition links;
- archive integrity test.

A full Qt 6 compile could not be performed in the packaging environment because Qt 6 development files are not installed there. The included `./install.sh` performs a clean CMake/Ninja build and executes the project tests on the target Arch Linux system before installing anything.

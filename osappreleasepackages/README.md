# OS App Release Packages

This directory is the future distribution layer for CarsonCC.

The goal is to let CarsonCC ship native compiler packages, runtime tools, terminal integrations, and helper binaries for different operating systems without mixing them into the compiler source tree.

## Layout

- `windows/` — Windows builds, installers, terminal tools, and future MSVC/MinGW integrations.
- `ubuntu/` — Ubuntu/Debian-family packages such as `.deb` releases and terminal helpers.
- `linux/` — Generic Linux releases, portable archives, and distro-neutral binaries.
- `terminals/` — CarsonCC terminal/front-end packages and shell integrations shared across operating systems.

Each platform folder can eventually contain versioned release metadata and generated packages from GitHub Actions.

## Planned package flow

`CarsonCC source → build → platform package → OS release → terminal integration`

Generated binaries should be treated as release artifacts; source code stays under `src/` and examples stay under `examples/`.

# CarsonCC 🦅

**CarsonCC** is a from-scratch compiler for the **Carson programming language**.

CarsonCC has a working frontend, function parameters/calls, semantic validation, x86-64 code generation, and a native Windows x64 MASM backend. GitHub Actions automatically builds and tests the compiler on every branch push, pull request, and scheduled run.

> 🚧 **Status: early but functional.** CarsonCC can parse Carson source, validate names and function calls, generate native assembly, and build executables on supported toolchains.

## ⚡ Quick Start

```bash
make
make test
./carsoncc examples/hello.car -o hello
./hello
```

Generate host assembly only:

```bash
./carsoncc examples/hello.car -S -o hello.s
```

Generate native Windows x64 MASM assembly from any host:

```bash
./carsoncc examples/hello.car -target windows -S -o hello.asm
```

On a Windows machine with MSVC and MASM installed, CarsonCC can also build the Windows executable directly:

```text
carsoncc examples\hello.car -target windows -o hello.exe
```

## 📱 CarsonCC Mobile

CarsonCC now includes a native Android companion app under `mobile/`.

**CarsonCC Mobile** is designed as a lightweight background bridge for terminal applications and future Android/system integrations. It deliberately avoids constant polling and wake locks. When the user starts the service, it keeps a small localhost listener on `127.0.0.1:3939` and responds to simple commands:

```text
ping
status
version
help
```

A terminal app can connect locally, for example with a TCP client such as `nc`, without requiring internet access. The service is user-controlled and uses a low-importance ongoing notification while active.

The Android app does **not** grant itself privileged Android/system access. Future CarsonCC Mobile work can add narrowly scoped integrations as Android APIs and permissions allow.

GitHub releases automatically build an unsigned release APK alongside the Linux, Windows, and source packages. The APK is intended for sideloading/testing while the mobile layer is developed.

## 🧠 Compiler Pipeline

```text
Carson source (.car)
        ↓
      Lexer
        ↓
     Tokens
        ↓
     Parser
        ↓
       AST
        ↓
 Semantic Analysis
        ↓
   Target backend
      ↙       ↘
 System V     Windows x64
 x86-64       MASM
      ↓          ↓
 Host toolchain / MSVC
      ↓
 Native executable
```

The IR layer is the next major compiler architecture step.

## 🟢 Current Features

- 🔤 Lexer with keywords, identifiers, integers, strings, operators, punctuation, and `//` comments
- 🌳 AST construction
- 🧩 Recursive-descent parser
- ➕ `+`, `-`, `*`, `/` precedence
- 📦 `let` declarations
- 🔎 Undefined-variable and duplicate-name diagnostics
- 🧬 Function declarations with parameters
- 📞 Function calls with argument validation
- 🧠 Function symbol table and call-arity checking
- ↩️ `return` statements
- 🏗️ System V x86-64 Intel-syntax backend
- 🪟 Native Windows x64 MASM backend using the Windows register calling convention
- 🔗 Native executable generation through the host toolchain
- 🧪 Automated end-to-end tests
- 🤖 Always-on GitHub Actions automation
- 📱 Android companion/background bridge
- 📦 Automatic release packages from `v*` tags and main-branch builds

## 📝 Carson Syntax

```carson
fn add(a, b) {
    return a + b;
}

fn main() {
    return add(20, 22);
}
```

The function-call test produces an exit status of `42`.

## 📦 OS App Release Packages

CarsonCC has a dedicated `osappreleasepackages/` tree for distributable compiler packages and terminal integrations. Generated release files stay separate from the compiler source.

```text
osappreleasepackages/
├── windows/     # Windows compiler packages and future terminal tools
├── ubuntu/      # Ubuntu/Debian .deb packages and APT metadata
├── linux/       # Distro-neutral Linux archives
└── terminals/   # Cross-platform shell/terminal integrations
```

The release workflow currently produces:

- 🐧 Linux x86-64 `.tar.gz`
- 🟠 Ubuntu/Debian x86-64 `.deb`
- 🪟 Windows x86-64 `.zip`
- 🤖 Android release `.apk`
- 📦 Source `.tar.gz`

## 🤖 Automatic GitHub Actions

Workflows live under `.github/workflows/`.

### Always-On Auto Build

`auto.yml` runs automatically on:

- every push to any branch
- every pull request
- every `v*` version tag
- a daily scheduled build
- manual `workflow_dispatch`

It builds CarsonCC on both Ubuntu and Windows. The Windows job actually invokes the native Windows backend, builds `hello.exe`, and verifies its exit status. The Linux job also verifies that Windows MASM assembly can be emitted.

### CI

`ci.yml` provides the focused main-branch compiler tests, including multi-function calls and assembly generation.

### Releases

Push a version tag such as:

```bash
git tag v0.1.0
git push origin v0.1.0
```

The release workflow automatically:

1. Builds CarsonCC on Ubuntu.
2. Runs the compiler tests.
3. Creates an Ubuntu/Debian `.deb` package.
4. Creates a Linux x86-64 archive.
5. Builds the Windows compiler with MSVC.
6. Creates a Windows x86-64 ZIP package.
7. Builds the Android release APK.
8. Creates a source archive.
9. Publishes a GitHub Release containing the generated files.

Main-branch pushes also use an automatic `0.1.<run>` version so package builds can be published without manually creating a tag.

No manual package assembly is required.

## 📁 Project Layout

```text
Carsoncc/
├── .github/
│   └── workflows/
│       ├── auto.yml
│       ├── ci.yml
│       └── release.yml
├── mobile/
│   ├── app/
│   │   └── src/main/
│   │       ├── AndroidManifest.xml
│   │       ├── java/com/carsoncc/mobile/
│   │       │   ├── MainActivity.java
│   │       │   └── CarsonService.java
│   │       └── res/values/styles.xml
│   ├── app/build.gradle
│   ├── build.gradle
│   └── settings.gradle
├── src/
│   ├── main.c
│   ├── lexer.c/.h
│   ├── parser.c/.h
│   ├── ast.c/.h
│   ├── semantic.c/.h
│   ├── semantic_full.c  # complete function/call validation
│   ├── ir.c/.h
│   ├── codegen.c/.h
│   └── codegen_windows.c # native Windows x64 MASM backend
├── examples/
│   └── hello.car
├── osappreleasepackages/
│   ├── windows/
│   ├── ubuntu/
│   ├── linux/
│   └── terminals/
├── Makefile
└── README.md
```

## 🛠️ Command Line

```text
carsoncc <input.car> [-o output] [-S] [-target TARGET]
```

- `-S` — generate assembly and stop before linking.
- `-o FILE` — choose the output executable or assembly filename.
- `-target host` — use the System V x86-64 backend (default).
- `-target windows` or `-target win64` — use the native Windows x64 MASM backend.

The Windows backend currently supports up to four integer parameters/arguments, matching the four register argument positions of the Windows x64 ABI. Additional Windows ABI stack-argument support is planned.

## 🔧 Building

CarsonCC is written in C11 and currently needs a C compiler plus the appropriate host assembler/linker toolchain.

```bash
make
make test
make clean
```

The Android companion is built with Gradle 8.9 and Android Gradle Plugin 8.7.3:

```bash
gradle -p mobile assembleRelease
```

## 🗺️ Roadmap

### Phase 1 — Working Frontend ✅
- [x] Compiler driver
- [x] Lexer
- [x] AST
- [x] Recursive-descent parser
- [x] Arithmetic
- [x] Variables
- [x] Semantic analysis

### Phase 2 — Functions & Language 🚧
- [x] Function parameter syntax
- [x] Function-call syntax
- [x] Function/call semantic checking
- [x] Multi-function x86-64 code generation
- [ ] Assignment expressions
- [ ] Booleans and comparisons
- [ ] `if` / `else`
- [ ] `while` / `for`
- [ ] Strings and runtime support
- [ ] Better diagnostics

### Phase 3 — Real IR ⚙️
- [ ] Three-address Carson IR
- [ ] Basic blocks
- [ ] Control-flow graph
- [ ] AST → IR lowering
- [ ] IR verifier
- [ ] Constant folding
- [ ] Dead-code elimination

### Phase 4 — Native Toolchain 🔥
- [x] x86-64 assembly backend foundation
- [x] Native executable generation
- [x] Native Windows x86-64 codegen foundation
- [ ] Direct ELF object generation
- [ ] Carson runtime
- [ ] Register allocation
- [ ] Optimization levels
- [ ] Debug information
- [ ] Windows ABI stack arguments

### Phase 5 — OS Distribution 📦
- [x] Automated Linux CI
- [x] Automated Windows compiler build
- [x] Always-on cross-platform auto build
- [x] Ubuntu/Debian `.deb` packaging
- [x] Generic Linux portable packaging
- [x] Windows ZIP packaging
- [x] Android companion APK build
- [x] Automatic GitHub Releases from version tags
- [ ] APT repository
- [x] Windows installer
- [ ] Terminal integrations
- [ ] Android terminal command API
- [ ] Android system integrations
- [ ] Cross-compilation
- [ ] Beta OS target
- [ ] Self-hosting CarsonCC

## 🎯 Long-Term Goal

The end goal is a compiler that can eventually compile itself and target multiple operating systems:

```text
Carson source
      ↓
   CarsonCC
      ↓
 Carson IR
      ↓
 Target backend
      ↓
 Windows / Ubuntu / Linux / Android / Beta OS
      ↓
 Native Carson application 🚀
```

## 📜 License

License information will be added as the project develops.

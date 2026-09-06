# CarsonCC 🦅

**CarsonCC** is a from-scratch compiler for the **Carson programming language**.

The compiler now has a working frontend, function parameters/calls, semantic validation, and an x86-64 backend. GitHub Actions automatically builds and tests the compiler on Linux and Windows, while version tags automatically produce Linux/Ubuntu and Windows release packages.

> 🚧 **Status: early but functional.** CarsonCC can parse Carson source, validate names and function calls, generate x86-64 assembly, and build native executables on its supported host toolchain.

## ⚡ Quick Start

```bash
make
make test
./carsoncc examples/hello.car -o hello
./hello
```

Generate assembly only:

```bash
./carsoncc examples/hello.car -S -o hello.s
```

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
   x86-64 Codegen
        ↓
 Host assembler/linker
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
- 🏗️ x86-64 Intel-syntax backend
- 🔗 Native executable generation through the host toolchain
- 🧪 Automated end-to-end tests
- 🤖 GitHub Actions CI on pushes, pull requests, and manual dispatch
- 📦 Automatic release packages from `v*` tags

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
- 📦 Source `.tar.gz`

## 🤖 Automatic GitHub Actions

Workflows live under `.github/workflows/`.

### CI

Every push to `main`, pull request targeting `main`, or manual CI dispatch runs the compiler build and tests. Linux also tests assembly generation and a multi-function call.

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
7. Creates a source archive.
8. Publishes a GitHub Release containing the generated files.

No manual package assembly is required.

## 📁 Project Layout

```text
Carsoncc/
├── .github/
│   └── workflows/
│       ├── ci.yml
│       └── release.yml
├── src/
│   ├── main.c
│   ├── lexer.c/.h
│   ├── parser.c/.h
│   ├── ast.c/.h
│   ├── semantic.c/.h
│   ├── semantic_full.c  # complete function/call validation
│   ├── ir.c/.h
│   └── codegen.c/.h
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
carsoncc <input.car> [-o output] [-S]
```

- `-S` — generate x86-64 assembly and stop before linking.
- `-o FILE` — choose the output executable or assembly filename.

## 🔧 Building

CarsonCC is written in C11 and currently needs a C compiler plus the host assembler/linker toolchain.

```bash
make
make test
make clean
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
- [ ] Direct ELF object generation
- [ ] Carson runtime
- [ ] Register allocation
- [ ] Optimization levels
- [ ] Debug information
- [ ] Native Windows x86-64 codegen

### Phase 5 — OS Distribution 📦
- [x] Automated Linux CI
- [x] Automated Windows compiler build
- [x] Ubuntu/Debian `.deb` packaging
- [x] Generic Linux portable packaging
- [x] Windows ZIP packaging
- [x] Automatic GitHub Releases from version tags
- [ ] APT repository
- [ ] Windows installer
- [ ] Terminal integrations
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
 Windows / Ubuntu / Linux / Beta OS
      ↓
 Native Carson application 🚀
```

## 📜 License

License information will be added as the project develops.

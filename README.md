# CarsonCC 🦅

**CarsonCC** is a from-scratch compiler for the **Carson programming language**.

The project is being built from the ground up: CarsonCC has its own lexer, parser, AST, semantic checker, and x86-64 assembly backend. The host assembler/linker is used for native executables for now, while a real IR and more native toolchain work are on the roadmap.

> 🚧 **Status: early but functional.** The compiler can parse Carson source, check names, generate x86-64 assembly, and build a native executable.

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
      Carson IR
        ↓
   Target Backend
        ↓
 Native OS package
```

The IR stage is being introduced as the next major compiler architecture step.

## 🟢 Current Features

- 🔤 Lexer with keywords, identifiers, integers, strings, operators, punctuation, and `//` comments
- 🌳 AST construction
- 🧩 Recursive-descent parser
- ➕ `+`, `-`, `*`, `/` precedence
- 📦 `let` declarations
- 🔎 Undefined-variable and duplicate-name diagnostics
- ↩️ `return` statements
- 🏗️ x86-64 Intel-syntax backend
- 🔗 Native executable generation through the host toolchain
- 🧪 End-to-end Makefile test
- 🧱 Function-parameter and function-call syntax is being added to the compiler frontend

## 📝 Carson Syntax

```carson
fn main() {
    let answer = 10 + 20 * 2;
    return answer;
}
```

The expression evaluates to `50`.

The planned function-call syntax is:

```carson
fn add(a, b) {
    return a + b;
}

fn main() {
    return add(20, 22);
}
```

## 📦 OS App Release Packages

CarsonCC now has a dedicated `osappreleasepackages/` tree for future distributable compiler packages and terminal integrations. This keeps generated OS packages separate from the compiler source.

```text
osappreleasepackages/
├── windows/     # Windows executables, ZIP/installer releases, terminal tools
├── ubuntu/      # .deb packages, APT metadata, Ubuntu terminal integration
├── linux/       # Distro-neutral Linux archives and portable builds
└── terminals/   # Cross-platform shell/terminal integration
```

Future GitHub Actions can build these directories from tagged CarsonCC releases. Additional distro-specific directories can be added later without changing the compiler's `src/` layout.

## 📁 Project Layout

```text
Carsoncc/
├── src/
│   ├── main.c
│   ├── lexer.c/.h
│   ├── parser.c/.h
│   ├── ast.c/.h
│   ├── semantic.c/.h
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

- `-S` — generate assembly and stop before linking.
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
- [x] Basic semantic analysis

### Phase 2 — Functions & Language 🚧
- [x] Function parameter syntax
- [x] Function-call syntax
- [ ] Full semantic checking for calls and parameters
- [ ] Reliable multi-function code generation tests
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

### Phase 5 — OS Distribution 📦
- [ ] Windows x86-64 release packages
- [ ] Ubuntu/Debian `.deb` packages
- [ ] Generic Linux portable packages
- [ ] Terminal integrations
- [ ] Automated GitHub Actions release builds
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

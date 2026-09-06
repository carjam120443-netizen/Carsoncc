# CarsonCC 🦅

**CarsonCC** is a from-scratch compiler project for the **Carson programming language**.

The goal is to build a real, understandable compiler instead of wrapping an existing compiler. CarsonCC is being developed in stages, starting with a small C implementation and growing toward native **x86-64** code generation and eventually self-hosting.

> 🚧 **Early development:** the compiler pipeline is being built piece by piece. APIs and language syntax may change.

## 🧠 Compiler Pipeline

```text
Carson source (.car)
        │
        ▼
     Lexer
        │
        ▼
     Tokens
        │
        ▼
     Parser
        │
        ▼
      AST
        │
        ▼
Semantic Analysis
        │
        ▼
   Carson IR
        │
        ▼
   Optimizations
        │
        ▼
 x86-64 Codegen
        │
        ▼
 Assembly / Object
        │
        ▼
 Native Executable
```

## ✨ Current Foundation

The repository now contains the initial pieces of the compiler architecture:

- 🖥️ C compiler driver entry point
- 🔤 Lexer/token definitions
- 🌳 Abstract Syntax Tree foundation
- 🧩 Parser interface and implementation stub
- 🔎 Semantic-analysis interface and stub
- ⚙️ Intermediate Representation interface and stub
- 🏗️ x86-64 code-generation interface and stub
- 📦 Makefile-based build system
- 🧪 First `.car` language example

These components are intentionally small right now. Each stage will become functional as development continues.

## 📁 Project Layout

```text
Carsoncc/
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── lexer.h
│   ├── parser.c
│   ├── parser.h
│   ├── ast.c
│   ├── ast.h
│   ├── semantic.c
│   ├── semantic.h
│   ├── ir.c
│   ├── ir.h
│   ├── codegen.c
│   └── codegen.h
├── examples/
│   └── hello.car
├── Makefile
└── README.md
```

## 📝 Carson Language

The language is designed to stay readable while giving the compiler room to grow.

A very early example looks like:

```carson
fn main() {
    let message = "Hello from CarsonCC!";
    return 0;
}
```

Planned language features include:

- Variables and constants
- Integer and string values
- Arithmetic expressions
- Functions
- Return values
- Conditionals
- Loops
- User-defined types
- Modules/imports
- A standard library
- Memory-management features
- A stable compiler command-line interface

## 🔨 Building

You need a C11-compatible compiler such as GCC, Clang, or another compatible implementation.

```bash
make
```

Run the current compiler driver against the example:

```bash
make test
```

Or directly:

```bash
./carsoncc examples/hello.car
```

At this stage the executable is the compiler driver and scaffolding; native Carson compilation is still being implemented.

## 🎯 Roadmap

### Phase 1 — Foundation

- [x] Repository structure
- [x] Compiler driver
- [x] Token definitions
- [x] AST foundation
- [x] Build system
- [ ] Functional lexer

### Phase 2 — Frontend

- [ ] Complete lexer
- [ ] Expression parser
- [ ] Statement parser
- [ ] Function parser
- [ ] AST construction
- [ ] Diagnostics and source locations

### Phase 3 — Semantic Analysis

- [ ] Symbol tables
- [ ] Name resolution
- [ ] Type checking
- [ ] Function checking
- [ ] Useful compiler errors

### Phase 4 — Backend

- [ ] Carson IR
- [ ] IR lowering
- [ ] Basic optimizations
- [ ] x86-64 instruction generation
- [ ] Assembly output
- [ ] Object/executable generation

### Phase 5 — Beyond

- [ ] Standard library
- [ ] Better optimization
- [ ] Debug information
- [ ] Cross-compilation support
- [ ] Beta OS target support
- [ ] Self-hosted CarsonCC

## 🧪 Development Philosophy

CarsonCC is intended to be **small, hackable, and understandable**. Compiler stages should remain separated so the project can eventually support additional architectures and tools without rewriting the entire compiler.

The long-term idea is simple:

```text
Write Carson
    ↓
Compile with CarsonCC
    ↓
Get native machine code
    ↓
Run it anywhere CarsonCC supports
```

## 🛠️ Project Status

**Experimental / pre-alpha** 🚧

The architecture is being established now. The first major milestone is turning the lexer into a real tokenizer, followed by a working parser and AST pipeline.

## 📜 License

License information will be added as the project develops.

# CarsonCC 🦅

**CarsonCC** is a from-scratch compiler for the **Carson programming language**.

The project is intentionally being built from the ground up: CarsonCC has its own lexer, parser, AST, semantic checker, and x86-64 assembly backend. The system linker/assembler is used for the final native executable for now; replacing more of the toolchain is part of the roadmap.

> 🚧 **Status: early but actually functional.** CarsonCC can parse a small Carson program, perform basic semantic checks, generate x86-64 assembly, and build a native executable.

## ⚡ Quick Start

```bash
make
make test
```

Compile a Carson program:

```bash
./carsoncc examples/hello.car -o hello
./hello
```

Generate assembly instead:

```bash
./carsoncc examples/hello.car -S -o hello.s
```

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
 x86-64 Code Generator
        │
        ▼
   GNU/LLVM toolchain
        │
        ▼
 Native Executable
```

The IR layer is reserved for the next major compiler stage.

## 🟢 What Works Now

- 🔤 Real tokenizer with keywords, identifiers, integers, operators, punctuation, comments, and strings
- 🌳 AST construction
- 🧩 Recursive-descent parser
- ➕ Operator precedence for `+`, `-`, `*`, and `/`
- 📦 `let` declarations
- 🔎 Basic semantic checking and undefined-variable diagnostics
- ↩️ `return` statements
- 🏗️ x86-64 Intel-syntax assembly generation
- 🔗 Native executable generation through the host toolchain
- 🧪 Executable build test in the Makefile
- 📄 Source line diagnostics

## 📝 Carson Syntax

A currently supported Carson program:

```carson
fn main() {
    let answer = 10 + 20 * 2;
    return answer;
}
```

The expression evaluates to `50`, so the resulting program exits with status `50`.

Variables can be used in later expressions:

```carson
fn main() {
    let a = 8;
    let b = 4;
    return a * b + 2;
}
```

Comments use `//`:

```carson
// Carson comment
fn main() {
    return 42;
}
```

## 📁 Project Layout

```text
Carsoncc/
├── src/
│   ├── main.c       # Compiler CLI and driver
│   ├── lexer.c/.h   # Source → tokens
│   ├── parser.c/.h  # Tokens → AST
│   ├── ast.c/.h     # AST data structures
│   ├── semantic.c/.h# Name checking
│   ├── ir.c/.h      # Planned intermediate representation
│   └── codegen.c/.h # AST → x86-64 assembly
├── examples/
│   └── hello.car
├── Makefile
└── README.md
```

## 🛠️ Command Line

```text
carsoncc <input.car> [-o output] [-S]
```

### `-S`

Generate x86-64 assembly and stop before linking.

### `-o FILE`

Choose the output filename. Without `-o`, the compiler creates `a.out`.

## 🔧 Building From Source

CarsonCC is written in portable C11 and currently needs a C compiler plus the host assembler/linker toolchain.

```bash
make
```

Run the end-to-end test:

```bash
make test
```

Clean build artifacts:

```bash
make clean
```

## 🧪 Compiler Development

The compiler is intentionally developed in layers:

```text
Lexer → Parser → AST → Semantic Analysis → IR → Backend
```

Each stage should have a clear interface and be testable independently. This makes it easier to add language features without turning the compiler into one giant source file.

## 🗺️ Roadmap

### Phase 1 — Working Frontend ✅

- [x] Compiler driver
- [x] Token definitions
- [x] Functional lexer
- [x] AST
- [x] Recursive-descent parser
- [x] Arithmetic expressions
- [x] Variables
- [x] Basic semantic analysis

### Phase 2 — Better Language Support 🚧

- [ ] Assignment expressions
- [ ] Boolean values
- [ ] Comparisons
- [ ] `if` / `else`
- [ ] `while` / `for`
- [ ] Function parameters
- [ ] Function calls
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
- [ ] Multiple optimization levels
- [ ] Debug information

### Phase 5 — CarsonCC Gets Serious 🦅

- [ ] Standard library
- [ ] Modules/imports
- [ ] User-defined types
- [ ] Arrays and pointers
- [ ] Cross-compilation
- [ ] Windows x86-64 backend
- [ ] Beta OS target
- [ ] Self-hosting CarsonCC

## 🎯 Long-Term Goal

The end goal is a compiler that can eventually compile itself:

```text
        CarsonCC
           │
           ▼
     Carson language
           │
           ▼
      CarsonCC again
           │
           ▼
     Native compiler
```

And eventually:

```text
Carson source
      ↓
   CarsonCC
      ↓
 Carson IR
      ↓
 x86-64 / other targets
      ↓
 Native executable
      ↓
     Beta OS 🚀
```

## 📜 License

License information will be added as the project develops.

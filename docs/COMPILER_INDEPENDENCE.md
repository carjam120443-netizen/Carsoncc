# CarsonCC Compiler Independence Roadmap

This roadmap deliberately preserves the existing compiler, command-line interface, package formats, Windows MASM backend, Linux assembly backend, GitHub Actions workflows, and Android companion. Work lands in small, testable steps; existing behavior remains the default until a replacement passes the same tests.

## Current architecture (verified against the source)

The current pipeline is:

```text
.car source -> lexer -> parser -> AST -> semantic checks
                                      |
                                      +-> System V x86-64 assembly -> host `cc`
                                      |
                                      +-> Windows x64 MASM -> `ml64` + `link.exe`
```

The frontend is written in C11. `src/ir.c` currently contains only an empty `ir_init()` placeholder; it is not yet a functioning intermediate representation. `src/main.c` invokes external assembler/linker tools through `system()`. The compiler is therefore a functioning compiler frontend with assembly backends, but it does not yet own the complete native toolchain.

## Compatibility rules

1. Keep `carsoncc <input.car> [-o output] [-S] [-target TARGET]` working.
2. Keep `-target host` and `-target windows` behavior available while new backends mature.
3. Do not require LLVM, GCC, Clang, NASM, or another compiler framework as a runtime dependency of the future native backend. A C compiler may still be used to bootstrap building CarsonCC's C implementation.
4. Do not remove existing release artifacts, package layouts, Android files, or CI jobs as part of compiler work.
5. Every backend change must have a test that checks emitted output and, where the runner supports it, executes a generated program and verifies its exit status.
6. Add features to the language only when the frontend, semantic analysis, IR, and backends agree on their behavior.

## Milestone 1 — Real, verified Carson IR

**Goal:** replace the placeholder IR with a real representation between the AST and target backends.

- Define explicit IR program, function, basic-block, instruction, operand, and value structures.
- Lower existing integer literals, variables, arithmetic, local declarations, function calls, parameters, and returns from the AST.
- Assign stable virtual temporary IDs and local-slot IDs.
- Add an IR verifier that rejects undefined temporaries, invalid operands, malformed calls, and functions without valid termination.
- Add a readable IR dump mode for debugging; it must be additive and must not change existing CLI defaults.
- Add tests for the existing `examples/hello.car`, function calls, arithmetic precedence, bad source, and invalid IR.
- Keep current code generators consuming the AST until IR lowering and verification have comprehensive coverage.

**Exit criteria:** deterministic IR output, verifier tests, and all existing tests passing on Linux and Windows.

## Milestone 2 — Make existing backends consume IR

- Add IR-to-System-V-x86-64 and IR-to-Windows-x64 lowering.
- Preserve the current AST backends behind compatibility options until parity tests pass.
- Centralize target ABI rules, stack alignment, parameter passing, call clobbers, and return values.
- Add regression tests for nested calls, recursion, more arguments, locals, arithmetic, and stack alignment.
- Make the IR verifier run before any backend emits output.

**Exit criteria:** the IR path passes the same behavioral tests as both legacy backends; the legacy paths remain available for rollback.

## Milestone 3 — Own object-file generation

- Start with one well-defined target: x86-64 ELF on Linux.
- Implement a small machine-code emitter with bounds-checked byte buffers, labels, fixups, relocations, and explicit error reporting.
- Write valid ELF64 relocatable object files directly, including sections, symbols, string tables, and relocation entries.
- Initially rely on the platform linker only to produce the final executable; do not claim a fully independent toolchain yet.
- Add object inspection tests and execute linked output in CI.
- Add Windows COFF/PE support only after ELF output is stable.

**Exit criteria:** CarsonCC emits a valid object without invoking an external assembler; tests prove relocations and symbol resolution work.

## Milestone 4 — Native executable and runtime support

- Implement the minimum runtime interfaces Carson needs, keeping the runtime small and documented.
- Build a native linker/loader path incrementally: section layout, symbol resolution, relocation application, entry point, and platform executable format.
- For Linux, target ELF first. For Windows, treat COFF object files and PE executable output as separate milestones.
- Keep system libraries and operating-system APIs as explicit platform interfaces; “independent compiler” does not mean reimplementing the operating system.
- Avoid invoking shell commands for the new native path. Never silently fall back to the host toolchain if native output fails.

**Exit criteria:** a documented native path creates runnable executables without an external assembler or linker for its supported subset. Unsupported features and targets fail with clear diagnostics.

## Milestone 5 — Language and optimization foundation

- Add booleans, comparisons, conditional branches, loops, and assignment through coordinated frontend/IR/backend changes.
- Add basic blocks and a control-flow graph before control-flow language features land.
- Implement conservative constant folding and dead-code elimination with unit tests.
- Add debug/source locations and optimization levels only after unoptimized code is correct.
- Document integer widths, overflow behavior, division behavior, calling conventions, and the language's runtime contract.

## Milestone 6 — Bootstrap and self-hosting

Self-hosting is a later goal, not a prerequisite for independent code generation.

1. Stabilize Carson's syntax, types, memory model, standard library, and module/import behavior.
2. Add the language features needed to express the compiler safely and maintainably.
3. Port small compiler components to Carson while keeping the C implementation as the trusted bootstrap compiler.
4. Use a three-stage bootstrap check: C compiler builds compiler A; A compiles Carson source into compiler B; B rebuilds the same compiler source into compiler C. Compare outputs or run equivalent compiler test suites.
5. Keep a reproducible bootstrap recipe and a known-good release binary/source archive.
6. Remove the C implementation only if the project explicitly chooses to; self-hosting does not require deleting the bootstrap implementation.

**Exit criteria:** a clean bootstrap process can rebuild CarsonCC from Carson source and passes the same frontend, IR, backend, and end-to-end tests.

## CI gates for every milestone

- Existing `make clean all` and `make test` remain green.
- Linux and Windows compiler builds remain green.
- Existing MASM output and Windows toolchain tests remain green.
- New IR and backend unit tests run without requiring privileged access.
- Native executable tests run only on supported runners and report unsupported targets explicitly.
- Android APK build/lint jobs and existing release artifact names remain unchanged unless a separate, intentional packaging change is approved.

## Non-goals for the first implementation steps

- Rewriting the lexer or parser from scratch.
- Removing the C bootstrap implementation.
- Replacing package workflows or the Android companion.
- Requiring a new third-party compiler framework.
- Claiming self-hosting or a fully independent linker before those capabilities are implemented and tested.

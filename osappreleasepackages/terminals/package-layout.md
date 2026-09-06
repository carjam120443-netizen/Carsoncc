# Terminal Package Layout

Future generated terminal integrations should follow this shape:

```text
terminals/
├── windows/
│   ├── powershell/
│   └── cmd/
└── linux/
    ├── bash/
    └── posix/
```

The compiler itself remains in `src/`; this area is for distributable integration files such as PATH setup, shell completion, aliases, launchers, and terminal helpers.

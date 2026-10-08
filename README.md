# XYO Coff to Def

Extract symbols from COFF object and generate a DEF file for linker
- Reads COFF objects (`.obj` from `cl.exe` / `clang-cl`; i386, x64, ARM64,
also `/bigobj`) and writes a module definition file (`.def`) that exports
every symbol they define, to link a DLL with `link /DLL /DEF:file.def`.
- Variables (also C common variables) are exported with `DATA`; compiler
generated symbols (constants, vftables, RTTI, throw info) are left out.
- `--mode AUTO` (default) applies the i386 naming rules (`_name` becomes
`name`) to i386 objects and keeps x64 / ARM64 names as they are.
- `@list.txt` response files; any bad input is an error (exit code `1`)
and no `.def` is written.

Used by `lib-to-dll` to turn static libraries into DLLs.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, first DEF file, link a DLL
- [Command line](docs/command-line.md) - options, modes, response files, exit codes, examples
- [DEF file](docs/def-file.md) - which symbols are exported, naming rules, `DATA`, filters, supported objects
- [API reference](docs/reference.md)

A Claude Code skill for this tool is in
[.claude/skills/xyo-coff-to-def](.claude/skills/xyo-coff-to-def/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.

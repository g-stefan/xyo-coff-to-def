---
name: xyo-coff-to-def
description: >-
  How to use xyo-coff-to-def, the XYO command line tool (namespace
  XYO::CoffToDef, on top of xyo-system) that reads COFF object files (.obj
  from cl.exe / clang-cl, i386, x64, ARM64, /bigobj) and writes a module
  definition file (.def: LIBRARY, EXPORTS, one name per line, "name DATA"
  for variables) to link a DLL with link /DLL /DEF:; the options --out
  (default out.def), --mode AUTO | WIN32 | WIN64 (default AUTO: i386
  objects lose the leading "_" of cdecl names, stdcall _f@8 / fastcall
  @f@8 / C++ ?names stay; x64 / ARM64 names unchanged), --show, --license,
  --version, @list.txt response files; which symbols are exported
  (defined external symbols and C common symbols; undefined, static,
  absolute skipped; compiler constants, vftables, RTTI, throw info
  filtered); errors and exit code 1 with no .def written (/GL objects,
  .lib archives, .exe, truncated objects); its use by lib-to-dll. Use when
  generating a .def file from objects, turning a static library into a DLL,
  running xyo-coff-to-def or lib-to-dll in a build or fabricare script,
  debugging missing or extra DLL exports, writing code that includes
  <XYO/CoffToDef/Coff.hpp>, or when working inside the xyo-coff-to-def
  repository.
---

# xyo-coff-to-def

Reads COFF objects and writes a `.def` that exports every symbol they
define, so objects without `__declspec(dllexport)` can be linked into a
DLL:

```
xyo-coff-to-def --out my.def a.obj b.obj @more.txt
link /DLL /DEF:my.def /OUT:my.dll /IMPLIB:my.lib a.obj b.obj ...
```

Full documentation: `docs/` in the xyo-coff-to-def repository
(`X:\Storage\XYO\Gitea\CPP\xyo-coff-to-def\docs` on this machine): README,
getting-started, command-line, **def-file** (export rules), reference.
The reader and writer are `source/XYO/CoffToDef/Coff.cpp`; option parsing
is `Application.cpp`.

## Output

```
LIBRARY
EXPORTS
?cppFn@NS@@YAHH@Z
?cppVar@NS@@3HA DATA
@fastFn@8
_stdFn@8
cdeclFn
commonVar DATA
```

`LIBRARY` without a name (link takes `/OUT`), sorted by byte value, each
name once, `\r\n`. No ordinals / `NONAME` / `PRIVATE` / aliases.

## What is exported

- **External and defined** symbols: in a section, or **common** (section
  0 with a size: C `int x;` at file scope). Undefined (`__imp_*`, what the
  object calls), static, absolute, debug symbols and aux records are not.
- **`DATA`** for every non function symbol (`(Type & 0x30) != 0x20`) and
  every common symbol. Consumers must use `__declspec(dllimport)` for them
  (the import library has only `__imp_name`).
- **Filtered** (checked on the name and, for i386, the name without its
  first `_`): `__real@`, `__xmm@`, `__ymm@`, `__zmm@`, `__mask@@`, `??_`
  (vftables, RTTI, string literals, `??_G` / `??_E`, but also
  `operator new[]` / `delete[]`), `_CT??`, `_CTA<digit>`,
  `_TI[CUV]*<digit>` (throw info), `__@@_PchSym_`. `_TIMER`, `_CTAB` stay.
- Inline / template COMDAT functions from headers are exported too
  (e.g. `std::exception` members, `_Avx2WmemEnabledWeakValue`): harmless.

## Modes (`--mode`, any case)

| Mode | Rule |
|------|------|
| `AUTO` (default) | per object: i386 → `WIN32`, x64 / ARM64 → `WIN64` |
| `WIN32` | `_name` with no `@` → `name`; `_f@8`, `@f@8`, `?f...` unchanged |
| `WIN64` | unchanged |

`link.exe` re-adds the i386 `_` for undecorated DEF names and keeps
decorated ones (import lib gets `__imp__f@8`, `__imp_@f@8`). Wrong mode
for the machine = names that do not match = link errors: keep `AUTO`.
Older versions defaulted to `WIN32` for everything (x64 C names were
dropped without `--mode`).

## Hard rules

1. **Exit code is the error report.** Any bad input → `Error: <file>:
   <reason>` on stdout, exit `1`, and **no `.def` is written** (an old one
   stays). All inputs are checked before failing. In fabricare:
   `exitIf(Shell.system("xyo-coff-to-def ..."))`.
2. **Objects only.** Rejected: `/GL` (LTCG) objects and import objects
   ("anonymous object ... not supported": rebuild without `/GL`), `.lib`
   archives ("extract the objects first (lib /extract)", or use
   `lib-to-dll`), `.exe` / `.dll`, other machines (ARM32, ARM64EC),
   truncated / damaged files ("invalid COFF object, ..."; never a crash).
3. **Options take two dashes.** Unknown `--x` → error; `-out` is a file
   name. Missing or invalid `--out` / `--mode` value → error.
4. **No file names**: no arguments → usage, exit `0`; `--license` /
   `--version` alone → exit `0`, nothing written; other options alone
   (`--show`) → `Error: no input files`, exit `1`.
5. **Response files** `@list.txt`: one name per line, trimmed, quotes
   stripped, `//` comments and empty lines skipped, UTF-8 BOM ok, nested
   `@file` up to 16 levels; names relative to the **current directory**;
   missing file → error.
6. `--show` prints `Coff Symbols: <file>` and the raw external defined
   names (before filtering / renaming); combine with `--out nul` to only
   look.
7. The output is overwritten; its folder must exist; default `out.def` in
   the current directory.

## lib-to-dll

`lib-to-dll [--mode WIN32|WIN64] [--static-crt] [--use-coff-def]
name.lib [extra.lib ...]` renames `name.lib` to `name.static.lib`,
extracts its objects into `name.obj\`, and — when there is no hand written
`name.def` or with `--use-coff-def` — runs
`xyo-coff-to-def --out name.dll.def --mode ... <objects>`, then
`link /DLL /DEF:...` to `name.dll` + import `name.lib`. Missing exports in
such a DLL: run `xyo-coff-to-def --show --out nul` on the extracted
objects and check the rules above.

## From C++

```cpp
#include <XYO/CoffToDef/Coff.hpp>
using namespace XYO::CoffToDef;

Coff::SymbolList symbols;   // TRedBlackTree<String, bool>: export name -> is data
String error;
if (!Coff::getSymbolsFromFile("a.obj", Coff::ModeAuto, symbols, false, error)) { /* error */ };
Coff::getSymbols(data, size, Coff::ModeAuto, symbols, false, error); // object in memory
Coff::generateDefFile("a.def", symbols);                             // or generateDef(symbols) -> String
```

Also `Coff::isFiltered(name)`, `Coff::exportName(name, Coff::ModeWin32)`.
Nothing is installed as a library: these are the tool's own sources (the
test compiles them in with `XYO_COFFTODEF_LIBRARY`, which removes
`main`). `Application::main(cmdN, cmdS)` runs the whole command and
returns the exit code.

## Debugging an object

```bash
dumpbin /nologo /headers a.obj | findstr machine    # machine, or "anonymous object" for /GL
dumpbin /nologo /symbols a.obj | findstr External   # what the tool reads (SECTn = defined, UNDEF = not)
dumpbin /nologo /exports my.dll                     # what the DLL ended up exporting
```

## Working inside this repository

- `fabricare.json`: `xyo-coff-to-def` (exe, `source/XYO/CoffToDef`) and
  `test.01` (`test/test.01.cpp` + the tool sources, `sourcePrefix: ""`,
  define `XYO_COFFTODEF_LIBRARY`). One key in `version.json`.
- Cycle: `fabricare make`, `fabricare test`, `fabricare install`,
  `fabricare clean` (see the fabricare skill for the Windows shell setup).
  The reader is portable: it also builds and passes the tests on Linux
  (WSL), useful to check it, though the tool targets Windows objects.
- `test/test.01.cpp` builds COFF objects in memory (`CoffBuilder`:
  regular / bigobj, i386 / x64 / ARM64, short and long names, aux
  records, common, filtered names) and checks the exact DEF text, the
  malformed object errors (every truncation of a valid object must not
  crash) and `Application::main` (exit codes, response files, no output
  on error). Add a case there for every rule change.
- To check against real compiler output: compile a sample with
  `cl /c` for x86 and x64 (also `/bigobj`, `/GL`), run the tool, and link
  `link /DLL /NOENTRY /DEF:...` with the objects; the link must succeed
  and `dumpbin /exports` must list the expected names.
- Keep `docs/` and this skill in step with `Coff.cpp` / `Application.cpp`.
- Licensing follows REUSE: `source/`, `docs/` and `README.md` are MIT
  (source files also carry SPDX headers); `test/`, config,
  `fabricare.json`, `version.json` and `.claude/` are Unlicense. Every new
  top level file or folder needs a `Files:` entry in `.reuse/dep5` (check
  with `python -m reuse lint`).

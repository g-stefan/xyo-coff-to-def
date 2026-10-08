# DEF file

## Layout

```
LIBRARY
EXPORTS
?cppFn@NS@@YAHH@Z
?cppVar@NS@@3HA DATA
@fastFn@8
_stdFn@8
cdeclFn
commonVar DATA
dataVar DATA
```

- `LIBRARY` has no name: `link.exe` takes the DLL name from `/OUT`.
- One export per line, `\r\n` line ends, sorted by byte value (`?` < `@` <
  upper case < `_` < lower case), each name once even when several objects
  (or several machines in `AUTO` mode) define it.
- No ordinals, `NONAME`, `PRIVATE` or aliases (`name=internal`): edit the
  file afterwards, or write it by hand, if you need them.
- An object set that defines nothing still gives a valid file with an empty
  `EXPORTS` section.

## Which symbols are exported

Every symbol of the COFF symbol table that is

- **external** (`IMAGE_SYM_CLASS_EXTERNAL`): static functions and
  variables are not, weak externals are not;
- **defined** in the object: in one of its sections, or a **common
  symbol** (section 0 with a size, what the C compiler makes of a
  tentative definition such as `int counter;` at file scope);

and that is not one of the [filtered symbols](#filtered-symbols).

Not exported: undefined symbols (what the object uses from elsewhere,
including `__imp_*`), absolute and debug symbols, auxiliary records.

Inline functions and templates instantiated from headers are external
COMDAT symbols in the object, so they are exported too, for example the
`std::exception` constructors when the code throws, or
`_Avx2WmemEnabledWeakValue` from the STL headers. That is harmless; the
DLL simply carries one more copy.

## DATA

A symbol whose type is not "function" (`(Type & 0x30) != 0x20`) and every
common symbol is a variable and gets `DATA`:

```
dataVar DATA
```

For a `DATA` export the import library holds only `__imp_dataVar`, the
address of the variable in the DLL. Code that uses it must declare it
`__declspec(dllimport)`; without that the link fails with an unresolved
`dataVar`, instead of silently reading the code of an import thunk (which
is what happened before `DATA` was written).

## Naming rules

| Object machine | Mode | Symbol | Export |
|----------------|------|--------|--------|
| i386 | `AUTO` / `WIN32` | `_adler32` (cdecl) | `adler32` |
| i386 | `AUTO` / `WIN32` | `_stdFn@8` (stdcall) | `_stdFn@8` |
| i386 | `AUTO` / `WIN32` | `@fastFn@8` (fastcall) | `@fastFn@8` |
| i386 | `AUTO` / `WIN32` | `?cppFn@@YAHH@Z` (C++) | `?cppFn@@YAHH@Z` |
| x64, ARM64 | `AUTO` / `WIN64` | `adler32`, `_name`, `?cppFn@@YAHH@Z` | unchanged |

What `link.exe` does with them (checked with MSVC 2026): `adler32` is
exported as `adler32`, and the import library has `_adler32` /
`__imp__adler32`, so i386 callers link as usual. `_stdFn@8` and
`@fastFn@8` are exported with their decoration, and the import library has
`__imp__stdFn@8` / `__imp_@fastFn@8`, which is what callers compiled with
the same declaration reference.

`--mode WIN32` on x64 objects strips the `_` of x64 names that really
start with `_`, and `--mode WIN64` on i386 objects keeps the `_` of C
names, so the DEF names do not match and the link fails. Use `AUTO`
unless you have a reason not to.

## Filtered symbols

Compiler generated symbols that must not, or need not, be exported. The
rules apply to the name as it is, and for i386 names also to the name
without its first `_`:

| Name starts with | What it is |
|------------------|------------|
| `__real@` | floating point constant |
| `__xmm@`, `__ymm@`, `__zmm@`, `__mask@@` | SSE / AVX / AVX-512 constants and masks |
| `??_` | vftables (`??_7`), RTTI (`??_R`), string literals (`??_C@`), compiler generated special members (`??_G`, `??_E` deleting destructors, ...) |
| `_CT??` | exception catchable type |
| `_CTA` + digit | exception catchable type array |
| `_TI` + digit, `_TIC`, `_TIU`, `_TIV`, ... + digit | exception throw info |
| `__@@_PchSym_` | precompiled header marker |

Names that only look similar are kept: `_TIMER`, `_CTAB`.

Because `??_` covers every special member name, a library that defines its
own `operator new[]` (`??_U@YAPAXI@Z`) or `operator delete[]` (`??_V...`)
does not export them. Add them to the DEF file by hand if needed.

## Supported objects

| Object | Supported |
|--------|-----------|
| COFF object, i386 (`0x014C`) | yes |
| COFF object, x64 (`0x8664`) | yes |
| COFF object, ARM64 (`0xAA64`) | yes, names as they are |
| `/bigobj` object (`ANON_OBJECT_HEADER_BIGOBJ`, 32 bit section numbers) | yes, any of the machines above |
| `/GL` (LTCG) object, import object, .NET object | no, error |
| ARM32, ARM64EC, ARM64X, other machines | no, error |
| static library (`.lib`, `!<arch>`) | no, error: extract the objects first |
| `.exe` / `.dll` | no, error |

The reader checks every offset against the file size: a truncated or
damaged object is reported as `invalid COFF object, ...`, it never makes
the tool crash, and nothing is written.

## Pitfalls

- A function whose C name is a DEF keyword (`DATA`, `PRIVATE`, `EXPORTS`,
  `LIBRARY`, ...) confuses `link.exe`; rename it or edit the file.
- Symbols that differ only in decoration may collapse: on i386 `_name` and
  a symbol really named `name` both become `name`.
- The DEF file exports everything external. Internal helper functions
  that happen to be `extern` are part of the DLL interface too; make them
  `static`, or edit the list.

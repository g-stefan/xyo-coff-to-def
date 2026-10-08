# Command line

```
xyo-coff-to-def [--out file] [--mode AUTO|WIN32|WIN64] [--show] file.obj ... @list.txt ...
xyo-coff-to-def --license
xyo-coff-to-def --version
```

## Options

| Option | Effect |
|--------|--------|
| `--out file` | the DEF file to write; default `out.def` in the current directory; overwritten |
| `--mode type` | naming rules: `AUTO` (default), `WIN32` or `WIN64`, any letter case (see [Modes](#modes)) |
| `--show` | print `Coff Symbols: <file>` and every external symbol each object defines, before filtering and renaming |
| `--license` | print the license (MIT) |
| `--version` | print `version X build Y [date time]` |
| `@list.txt` | read file names from `list.txt` (see [Response files](#response-files)) |
| any other argument | an object file |

Options can come in any order, before, between or after the file names;
all of them apply to all files. An argument that starts with `--` and is
not one of the options above is an error (`Error: unknown option --x`);
one dash (`-out`) is not an option, it is taken as a file name.

`--license` and `--version` print and go on: with no file names the tool
then exits `0` without writing a DEF file; with file names it also
processes them.

Running with **no arguments** prints the usage and exits `0`. Options but
no file names (`--show` alone, `--out x.def` alone) is an error,
`Error: no input files`, exit `1`.

## Modes

The mode decides how a symbol name becomes a DEF export name. Filtering
(see [DEF file](def-file.md#filtered-symbols)) is the same in every mode.

| Mode | Rule | Use it for |
|------|------|------------|
| `AUTO` (default) | `WIN32` for i386 objects, `WIN64` for x64 and ARM64 objects, chosen per object | everything; mixed x86 / x64 lists work |
| `WIN32` | a name that starts with `_` and has no `@` loses the `_` (`_adler32` → `adler32`); every other name stays (`_stdFn@8`, `@fastFn@8`, `?cppFn@@YAHH@Z`) | i386 objects |
| `WIN64` | names stay as they are | x64 and ARM64 objects |

i386 C compilers put `_` in front of every C name (`adler32` is
`_adler32` in the object) and `link.exe` adds it back when it reads a DEF
file, so the DEF file must list the C name without it. x64 and ARM64 have
no such prefix: `_name` there is a real name that starts with `_`, which is
why `WIN32` rules on x64 objects are wrong. `AUTO` avoids that.

Older versions used `WIN32` rules for every object when `--mode` was not
given, so x64 objects run without `--mode` lost their C functions; scripts
that pass `--mode` (like `lib-to-dll`) are not affected.

## Response files

`@list.txt` reads `list.txt` and adds the file names in it, one per line:

```
// zlib objects
adler32.obj
compress.obj

"objects with spaces\deflate.obj"
@more-objects.txt
```

- spaces and tabs around a name are removed, `\r\n` and `\n` both work, a
  UTF-8 byte order mark is skipped;
- empty lines and lines that start with `//` are ignored;
- a name between double quotes loses the quotes;
- a line that starts with `@` reads another response file, up to 16
  levels deep (a response file that includes itself is an error);
- names are relative to the **current directory**, not to the response
  file;
- a missing response file is an error.

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | DEF file written; also no arguments (usage), `--license` / `--version` without files |
| `1` | bad option or option value, no input files, a response file can not be read, any input can not be read or is not a supported COFF object, the DEF file can not be written |

Every error prints one `Error: ...` line on standard output. All inputs are
checked and all their errors printed; if any failed, **the DEF file is not
written** (an existing one is left as it was), so a build never goes on
with a partial export list.

| Message | Cause |
|---------|-------|
| `Error: file.obj: unable to read file` | missing or locked file |
| `Error: file.obj: is a library (.lib), extract the objects first (lib /extract)` | a `.lib` archive was given |
| `Error: file.obj: is an executable image (.exe / .dll), not a COFF object` | an `.exe` or `.dll` was given |
| `Error: file.obj: anonymous object (compiled with /GL, an import object or a .NET object) is not supported` | object compiled with `/GL`, or a member of an import library |
| `Error: file.obj: not a COFF object or unsupported machine 0x....` | not an object, or an architecture other than i386, x64, ARM64 |
| `Error: file.obj: not a COFF object, file too small` | empty or truncated file |
| `Error: file.obj: invalid COFF object, ...` | symbol table, string table or a name points outside of the file |
| `Error: unable to generate def file x.def` | the output can not be written (missing folder, read only) |

## Examples

Objects of one x64 library:

```
xyo-coff-to-def --out my-library.def a.obj b.obj c.obj
```

A long list from a response file:

```
dir /b /s temp\*.obj > objects.txt
xyo-coff-to-def --out my-library.def @objects.txt
```

What do these objects define (raw names, before filtering)?

```
xyo-coff-to-def --show --out nul a.obj
```

i386 objects, explicit mode (what `lib-to-dll` runs):

```
xyo-coff-to-def --out zlib.dll.def --mode WIN32 zlib.obj\adler32.obj zlib.obj\compress.obj
```

From a fabricare script:

```js
exitIf(Shell.system("xyo-coff-to-def --out temp/my-library.def @temp/my-library.objects.txt"));
```
